
#include "SAT.h"
#include "Node.h"
#include <vector>
using namespace std;

#define MAXLENGTH 500
#define STOREBLOCK 2000000

SAT::SAT(Configuration* conf){
    config=conf;
	string inputFileName;       config->getParameter("param.inputfilename", &inputFileName);
    int MAXVAR = 1000;         config->getParameter("const.maxVar", &MAXVAR);
    int MAXCLAUSE = 1000;       config->getParameter("const.maxClause", &MAXCLAUSE);
    db = new Database(config);
    
    int lastc;
	int nextc;
    
    numvariable=0;
    numclause=0;

    cout << "reading input file " << inputFileName << endl ;
	while ((lastc = getc(config->getCNFStream())) == 'c'){
		while ((nextc = getc(config->getCNFStream())) != EOF && nextc != '\n');
	}

    ungetc(lastc,config->getCNFStream());
	if (fscanf(config->getCNFStream(),"p cnf %i %i",&numvariable,&numclause) != 2){
		fprintf(stderr,"Bad input file\n");
		exit(-1);
	}

    cout << "#variables=" << numvariable << ", #clauses=" << numclause << endl ;
	if(numvariable > MAXVAR){
		fprintf(stderr,"ERROR - too many atoms\n");
		exit(-1);
	}
    
    if(numclause > MAXCLAUSE){
		fprintf(stderr,"ERROR - too many clauses\n");
		exit(-1);
	}
    
    numOccurence = new int[numvariable]; // number of times each literal occurs
    numOccurencePos = new int[numvariable];
    numOccurenceNeg = new int[numvariable];
    for(int i = 0;i < numvariable;i++){
        numOccurence[i] = 0;
        numOccurencePos[i] = 0;
        numOccurenceNeg[i] = 0;
    }
    
	size = new int[numclause];
	clause = new pair<int,bool>*[numclause];
    occurrence = new int*[numvariable];
	int lit;
    
    //problem with cnf is that you don't know the size of the clause until you read in all the literals,
    //so we hold literals in a temporary vector & when we read the line, we allocate the memory.
    for(int i = 0;i < numclause;i++){
        vector<int> tmp;
		size[i] = -1;
		do{
			size[i]++;
			if(size[i] > 100){
				printf("ERROR - clause too long\n");
				exit(-1);
			}
			if (fscanf(config->getCNFStream(),"%i ",&lit) != 1){
				fprintf(stderr, "Bad input file\n");
				exit(-1);
			}
            tmp.push_back(lit);

			if(lit != 0){
		        //We keep the count of how many times a literal (whether in original form or negated form) has occured.
                //obviously numoccurance = numoccpos + numoccneg
                numOccurence[(lit>0?lit:-lit)-1]++;
                if(lit>0){
                    numOccurencePos[lit-1]++;
                }else{
                    numOccurenceNeg[-lit-1]++;
                }
			}
		}while(lit != 0);
        
        clause[i] = new pair<int, bool>[size[i]];
        for(int j=0;j<size[i];j++){ //bugfix from tmp.size() to size[i], in cases that the clause size are different (like intel)
            //in cnf format, the vars start from 1, in c++ we index them at 0, so -1 for the var and +1 for the negation.
            //bugfix: the 0 does not differentiate between +-0, so I added a sign to the clause.
            //wasn't it easier just to reindex everything as 1?
            int lit = abs(tmp[j])-1;
            bool sign = tmp[j]>0;
            clause[i][j] = make_pair(lit, sign);
        }
	}
    
    // Populating the occurance matrix
    // The occurance matrix keeps track of which clauses a variables appear in
    // Index as occurance[variable][clause]
    for(int i=0;i<numvariable;i++){
        occurrence[i] = new int[numOccurence[i]];
    }
    
    int* occtmp = new int[numvariable];
    for(int i=0;i<numvariable;i++)
        occtmp[i]=0;
    
    for(int i = 0;i < numclause;i++){
    	for(int j = 0;j < size[i];j++){
            int var = getClause(i, j);
            occurrence[var][ occtmp[var]] = i;
            occtmp[var]++;
		}
	}
    delete occtmp;
    
    //Constant propagation optimization
    //This works really good on circuit benchmarks, but fails miserebly on random sat benchmarks!
    config->getParameter("param.constantpropagation", &enablePreProcess);
    if(enablePreProcess==1){
        cout << "[init] Constant propagation is enabeld. Proceed with reducing the search space." << endl ;
        variableMask=new int[numvariable]();
        clauseMask=new int[numclause]();
        preValue = new int[numvariable];
        for(int i=0;i<numvariable;i++){
            preValue[i]=-1;
        }
        
        bool moreUnmaskedVariable;
        do{
            moreUnmaskedVariable=false;
            for(int i=0;i<numclause;i++){
                if(clauseMask[i]==0){   //if we have not yet determined the value of this clause
                    int undeterminedCount=0;
                    int undeterminedVar=-1;
                    for(int j=0;j<size[i];j++){
                        int var=getClause(i, j);
                        if(variableMask[var]==0){
                            undeterminedCount++;
                            undeterminedVar=j;
                        }
                    }
                    
                    //if there is only 1 undetermined variable in this clause,
                    //we can determine that variable and clause directly
                    if(undeterminedCount==1){
                        int var=getClause(i, undeterminedVar);
                        bool sign = getSign(i, undeterminedVar);
                        int data=0;
                        for(int j=0;j<size[i];j++){
                            if(j!=undeterminedVar){
                                if( getSign(i, j)==preValue[getClause(i, j)] ){
                                    data=1;
                                }
                            }
                        }
                        
                        if(data==0){//Yaaay! we can remove another variable from the list!
                            moreUnmaskedVariable=true;
                            variableMask[var] = 1;
                            preValue[var]=sign?1:0;
                            clauseMask[i]=1;
                            //cout << "Variable " << var << " has been determined throught clause " << i << "    [" << preValue[var] << "]" <<endl ;
                        }
                    }
                }
            }
        }while(moreUnmaskedVariable);
        
        for(int i=0; i<numclause;i++){
            clauseMask[i]=0;
            for(int j=0;j<size[i];j++){
                int var=getClause(i, j);
                //when can I mask the clause?
                //If the clause doesn't have any input or I can determine it's output from any of it's current determined inputs.
                if( (variableMask[var]==1) && (getSign(i, j)==preValue[var]) ){
                    clauseMask[i]=1;
                }
            }
        }
        
        
        int c=0,d=0;
        for(int i=0;i<numvariable;i++){
            if(variableMask[i]==1){
                //cout << "variable " << i << " is determined" << endl;
                c++;
            }
        }
        
        for(int i=0;i<numclause;i++){
            if(clauseMask[i]==1){
                //cout << "clause " << i << " is determined" << endl ;
                d++;
            }
        }
        preDeterminedClauses = d;
        preDeterminedVariables = c;
        cout << "Total determined variable = " << c << "/" << numvariable << endl ;
        cout << "Total determined clauses = " << d << "/" << numclause<< endl ;
        
        int nullvar=0;
        for(int i=0;i<numvariable;i++){
            if(numOccurence[i]==0){
                nullvar++;
            }
        }
        //reduction in the original problem ...
        variableMap1 = new int[numvariable]();
        variableMap2 = new int[numvariable-preDeterminedVariables-nullvar];
        clauseMap1 = new int[numclause]();
        clauseMap2 = new int[numclause-preDeterminedClauses]();
        
        int k=0;
        for(int i=0;i<numclause;i++){
            if(clauseMask[i]==0){
                clauseMap1[i]=k;
                clauseMap2[k]=i;
                k++;
            }else{
                clauseMap1[i]=-1;
            }
        }
        
        k=0;
        for(int i=0;i<numvariable;i++){
            if(variableMask[i]==0 && numOccurence[i]>0){
                variableMap1[i]=k;
                variableMap2[k]=i;
                k++;
            }else{
                variableMap1[i]=-1;
            }
        }
        
         cout << "Dumping clauses " << endl ;
         for(int i=0;i<numclause;i++){
             if(clauseMask[i]==0){
                 cout << i << " :" ;
                 for(int j=0;j<size[i];j++){
                     int var = getClause(i, j);
                     int sign= getSign(i, j);
                     if (variableMask[var]==0 )
                         cout << var << " [" << sign << "]["<< variableMask[var] <<"/"<< preValue[var]  <<"], " ;
                 }
                 cout << endl ;
             }
         }
         
         cout << "Mapping clauses" << endl ;
         for(int i=0;i<numclause-preDeterminedClauses; i++){
             cout << i << "   <---" << clauseMap2[i] << endl;
         }
         
         cout << "Mapping variables" << endl ;
         for(int i=0;i<numvariable-preDeterminedVariables-nullvar;i++){
             cout << i << "  <=---" << variableMap2[i] << " /" << numOccurence[variableMap2[i]] << endl;
         }
        
        size_original = size;
        numOccurence_original = numOccurence;
        numOccurencePos_original = numOccurencePos;
        numOccurenceNeg_original = numOccurenceNeg;
        clause_original = clause;
        occurrence_original = occurrence;
        numvariable_original = numvariable;
        numclause_original = numclause;
        numvariable -= preDeterminedVariables+nullvar;
        numclause -= preDeterminedClauses;
        
        //update size
        k=0;
        size = new int[numclause]();
        for(int i=0;i<numclause_original;i++){
            if(clauseMask[i]==0){
                size[k]=0;
                for(int j=0;j<size_original[i];j++){
                    if(variableMask[ getClause(i, j)]==0){
                        size[k]++;
                    }
                }
                k++;
            }
        }
        
        cout << "size array" << endl ;
        for(int i=0;i<numclause;i++){
            cout << i << " " << size[i] << endl ;
        }
        
        //populating clause array
        clause = new pair<int,bool>*[numclause];
        for(int i=0, c=0;i < numclause; i++,c=0){
            clause[i] = new pair<int, bool>[size[i]];
            for(int j=0;j<size_original[ clauseMap2[i] ]; j++){
                int var = clause_original[ clauseMap2[i] ][ j ].first;
                bool sign = clause_original[ clauseMap2[i] ][ j ].second;
                if (variableMask[var]==0 ){
                    clause[i][c] = make_pair( variableMap1[var], sign);
                    c++;
                }
            }
        }
        
         for(int i=0;i<numclause;i++){
             cout << i << " : " ;
             for(int j=0;j<size[i];j++){
                 cout << getClause(i, j) << " [" << getSign(i, j)<< "],  ";
             }
             cout << endl ;
         }
        
        occurrence = new int*[numvariable];
        numOccurence = new int[numvariable]();
        numOccurencePos = new int[numvariable]();
        numOccurenceNeg = new int[numvariable]();
        
        for(int i=0;i<numclause;i++){
            for(int j=0;j<size[i];j++){
                numOccurence[getClause(i, j)]++;
                if(getSign(i, j)){
                    numOccurencePos[getClause(i, j)]++;
                }else{
                    numOccurenceNeg[getClause(i, j)]++;
                }
            }
        }
        
        // Populating the occurance matrix
        for(int i=0;i<numvariable;i++){
            occurrence[i] = new int[numOccurence[i]];
        }
        
        occtmp = new int[numvariable]();
        for(int i = 0;i < numclause;i++){
            for(int j = 0;j < size[i];j++){
                int var = getClause(i, j);
                occurrence[var][occtmp[var]] = i;
                occtmp[var]++;
            }
        }
        delete occtmp;
        
        for(int i=0;i<numvariable;i++){
            cout << "var " << i << " " ;
            
            for(int j=0;j<numOccurence[i];j++){
                cout << occurrence[i][j] << " " ;
            }
            cout << endl ;
        }
        
        bool conflict=false;
        for(int i=0; i<numclause;i++){
            if(size[i]==0){
                int r=0;
                int oc = clauseMap2[i];
                for(int j=0;j<size_original[oc];j++){
                    int var = clause_original[oc][j].first;
                    bool sign = clause_original[oc][j].second;
                    if(sign==preValue[var]){
                        r=1;
                    }
                }
                if(r==0){
                    cout << "Found conflict, Unsatisfiable Clause!  Clause:" << oc << " ";
                    conflict=true;
                    for(int j=0;j<size_original[oc];j++){
                        int var = clause_original[oc][j].first;
                        bool sign = clause_original[oc][j].second;
                        if(sign)
                            cout << var << "(" << preValue[var] << ") " ;
                        else
                            cout << "~" << var << "(" << preValue[var] << ") " ;

                    }
                    cout << endl ;
                }
            }
        }
        
        if(conflict){
            exit(1);
        }
    }   //End of constant propagation

    
    
    
    
    result = new Result();
}

SAT::SAT(SAT* copy){
    config=copy->config;
    db = new Database(config);
    numvariable=copy->numvariable;
    numclause=copy->numclause;
    
    numOccurence = copy->numOccurence; // number of times each literal occurs
    numOccurencePos = copy->numOccurencePos;
    numOccurenceNeg = copy->numOccurenceNeg;
	size = copy->size;
	clause = copy->clause;
    occurrence = copy->occurrence;
    
    signalProbabilityStat = copy->signalProbabilityStat;
    posCorrelationStatVariableClause = copy->posCorrelationStatVariableClause;
    posCorrelationStatClauseVariable = copy->posCorrelationStatClauseVariable;
    correlationStatVariableClause = copy->correlationStatVariableClause;
    correlationStatClauseVariable = copy->correlationStatClauseVariable;
    
    result = new Result();
}

SAT::~SAT(){
}

Result* SAT::getResult(){
    return result;
}

int SAT::getClause(int i, int j){
    return clause[i][j].first;
}

bool SAT::getSign(int i, int j){
    return clause[i][j].second;
}

int SAT::GetNumberOfVariables(){
    return numvariable;
}

int SAT::GetNumberOfClauses(){
    return numclause;
}

//This function recieves the state as input & computes the outpus and some statistics
Output* SAT::adjust(State* state){
    int* data = new int[numclause]();
    for(int i=0;i<numclause;i++){
        data[i]=0;
        for(int j=0;j<size[i];j++){
            if( getSign(i, j)==state->get(getClause(i, j)) ){
                data[i]=1;
            }
        }
    }
    
    Output* output = db->getOutput(data);
    if(output->null()){
        delete output;
        return new Output(numclause, data);
    }else{
        return output;
    }
}

int distance(Output* source, Output* target){
    return source->distance(target);
}

//todo: bug: two state may have the same output
Node* SAT::flip(Node* node, int flipbit){
    update(node, flipbit);
    //First, construct the flipped state
    int* data = new int[node->getState()->getSize()]();
    for(int i=0;i<node->getState()->getSize();i++){
        data[i] = node->getState()->get(i);
    }
    data[flipbit] = (data[flipbit]==0)?1:0;
    //Then, search for the given state, have we reached this state before?
    State* s = db->getState(data);
    if(s->null()){
        delete s;
        //we explored a new state
        State* state = new State(node->getState()->getSize());
        state->setData(data);
        Output* output = adjust(state);
        
        //check if these output has been explored before (incase two state yields the same output)
        //Output* o = db->getOutput();
        Node* flippedNode = new Node(state, output);
        update(flippedNode, flipbit);
        updateNodeStat(flippedNode);
        return flippedNode;
    }else{
        //this state is already explored
        update(s->getNode(), flipbit);
        return s->getNode();
    }
}

//update node statistics such as makecount/ break count, etc.
//This code is borrowd from WalkSAT v50.
//This can also be done incrementally, given a previous node and the flipped literal.
//TODO: implement the incremental update
void SAT::updateNodeStat(Node* node){
    int thetruelit=0;
	// Initialize breakcount and makecount in the following:
	for(int i = 0;i < numclause;i++){
        for(int j = 0;j < size[i];j++){
            if( getSign(i, j)==node->getState()->get(getClause(i, j)) ){
                node->numtruelit[i]++;
                thetruelit = getClause(i, j);
            }
		}
        if(node->numtruelit[i] == 0){   //There is no true literal in i^th clause, so its unsatisfied.
            node->wherefalse[i] = node->numfalse;
            node->falseClause[node->numfalse] = i;
            node->numfalse++;
            for(int j = 0;j < size[i];j++){
                node->makecount[getClause(i, j)]++;
            }
		}else if (node->numtruelit[i] == 1){
            node->breakcount[abs(thetruelit)]++;
		}
    }
}

//The initial training part of the algorithm, we later update the algorithm using the statistics
//genereated from the algorithm itself
void SAT::init(){
    config->getParameter("param.learning", &enableLearning);
    if(enableLearning==1){
        int trainingSamples=100; config->getParameter("param.training", &trainingSamples);
        //probability = new double*[numclause];
        
        signalProbabilityStat = new double[numclause];
        correlationStatVariableClause = new double*[numvariable];
        correlationStatClauseVariable = new double*[numclause];
        posCorrelationStatVariableClause = new double*[numvariable];
        posCorrelationStatClauseVariable = new double*[numclause];
        
        for(int i=0;i<numvariable;i++) correlationStatVariableClause[i]=new double[numclause]();
        for(int i=0;i<numclause;i++) correlationStatClauseVariable[i]=new double[numvariable]();
        for(int i=0;i<numvariable;i++) posCorrelationStatVariableClause[i]=new double[numclause]();
        for(int i=0;i<numclause;i++) posCorrelationStatClauseVariable[i]=new double[numvariable]();
        //for(int i=0;i<numclause;i++) probability[i] = new double[1 << size[i]]();
        
        for(int i=0;i<trainingSamples;i++){
            State* s = new State(numvariable);
            s->randomize();
            Output* o = adjust(s);
            for(int j=0;j<numclause;j++){
                if(o->get(j)==1)
                    signalProbabilityStat[j]++;
            }
            for(int j=0;j<numvariable;j++){
                for(int k=0;k<numOccurence[j];k++){
                    int clause = abs(occurrence[j][k]);
                    if( s->get(j) == o->get(clause)){ //corrolation of 0-0 and 1-1
                        correlationStatVariableClause[j][clause]++;
                        correlationStatClauseVariable[clause][j]++;
                        if(s->get(j)==1){
                            posCorrelationStatVariableClause[j][clause]++;
                            posCorrelationStatClauseVariable[clause][j]++;
                        }
                    }
                }
            }
            delete s;
            delete o;
        }
        
        for(int i=0;i<numclause;i++){
            signalProbabilityStat[i] /= trainingSamples;
        }
        for(int i=0;i<numvariable;i++){
            for(int j=0;j<numclause;j++){
                correlationStatVariableClause[i][j]/= trainingSamples;
                correlationStatClauseVariable[j][i]/= trainingSamples;
                posCorrelationStatVariableClause[i][j]/= trainingSamples;
                posCorrelationStatClauseVariable[j][i]/= trainingSamples;
            }
        }
    }
}

//Updates the mask bits, call this function when we flipped the bit^th in the node->state
void SAT::update(Node* node, int bit){
    //There are four mask that needs to be updated in order
    //The first mask is the stateMask. stateMask[i] indicates whether or not the bit i has already been flipped.
    //Therefore we never flip a variable i where stateMask[i]==1
    node->setStateMask(bit, true);
    
    //2. output mask in the node class
    //The second mask is the outputMask. Every output is connected to a limited number of variables (defined in int** clause).
    //For outputMask[i]==1 iff forall j in clause[i]. stateMask[j]==1
    //When the outputMask[i] is one, there is no reason to pick that node anymore.
    //Think: when outputMask[i] is one + output[i]=0: is this node unsatisfiable??
    //2.1 find which output clauses the variable i affects
    
    for(int i=0;i<numOccurence[bit];i++){
        int cl=occurrence[bit][i];
        // Now for clause cl, list all the variables that effect it, if all of them where masked, then there is no use in picking this clause.
        // so let's mask clause cl.
        bool mask=true;
        for(int j=0;j<size[cl];j++){
            int var= getClause(cl, j);
            if(!node->getStateMask(var)){
                mask=false;
            }
        }
        node->setOutputMask(cl, mask);
    }
    
    //3. The third mask is the node mask which indicates whether or not this node is unsatisfiable.
    // If the nodeMask is one, we cannot reach a satisfiable solution
    // within one bit-flip of this node. Therefore we exclude this node from searching for nearest node.
    // nodemask is 1 iff forall j where output[j]=0. outputmask[j]=1
    bool nodemask=true;
    for(int i=0;i<numclause;i++){
        if(!node->getOutputMask(i) && (node->getOutput()->get(i)==0))
            nodemask=false; //if there exists a clause where both output and mask is false
    }
    node->setNodeMask(nodemask);
    
    //4. output mask in output class
    //A very nasty pattern is happening here.
    //Update mask should be used in the output class, however, I cannot access the Node's mask data in the output
    //class because of forward declared type. Hence this loophole happend. todo: fix this
    node->getOutput()->setMask(true);
    for(int i=0;i<node->getOutput()->getStateSize();i++){
        //get node from the state
        if(!node->getOutput()->getState(i)->getNode()->getNodeMask()){ //wait, whaaat?
            node->getOutput()->setMask(false);
            return;
        }
    }
}

int SAT::pick_naive(Node* node){
    //step 1: pick which clause to flip
    int flipClause=-1;
    for(int i=0;i<numclause;i++){
        if( node->getOutputMask(i)==false && node->getOutput()->get(i)==0   ){
            flipClause=i;
        }
    }
    if(flipClause==-1) cout << "This shouldn't happen!, the node should already have been masked" << endl ;
    
    //Step 2: pick which variable to flip
    for(int i=0;i<size[flipClause]; i++){
        int flipBit = getClause(flipClause, i);
        if( node->getStateMask(flipBit)==false ){
            return flipBit;
        }
    }
    cout << "This shouldn't happend! all variables are masked, but the clause itself isn't masked" << endl ;
    return -1;
}

int SAT::pick_walksat2(Node* node){
    
}

//This is the same strategy as walksat/best
//(C) WalkSAT
pair<int, int> SAT::pick_best(Node* node){
    int numbreak;
	int best[MAXLENGTH];
	register int numbest;
	register int bestvalue;
	register int var;
    
	int tofix = node->falseClause[random()%node->numfalse];
    //for(int i=0;i<numclause; i++){
    //    if(!node->getOutputMask(i) && node->getOutput()->get(i)==0){ //false clause that is unmasked
            //This is not random anymore
    //    }
    //}
    
	int clausesize = size[tofix];
	numbest = 0;
	bestvalue = 999999999;
    
	for (int i=0; i< clausesize; i++){
        var = getClause(tofix, i);
        numbreak = node->breakcount[var];
        if (numbreak<=bestvalue){
            if(!node->getStateMask(var)){
                if (numbreak<bestvalue) numbest=0;
                bestvalue = numbreak;
                best[numbest++] = var;
            }
        }
	}
    
    //if(numbest==0) return make_pair(-1, bestvalue);
    cout << "numbest=" << numbest << endl ;
	if (numbest == 1) return make_pair(best[0], bestvalue);
	return make_pair(best[random()%numbest], bestvalue);
}

int SAT::pick_frequencist(Node* node){
    //We have to answer two qeustions:
    //1. Which clause C we want to turn satisfy at this iteration? Which is the easiest?
    //2. Which bit X to flip in order to satisfy C (and probably unsatisfy some other clauses).
    int flipClause=-1;
    for(int i=0;i<numclause;i++){
        if(node->getOutputMask(i)==false && node->getOutput()->get(i)==0){
            if(flipClause==-1){
                flipClause=i;
            }else{
                //Select a clause with highest signal probability
                if(signalProbabilityStat[i] > signalProbabilityStat[flipClause]){
                    flipClause=i;
                }
            }
        }
    }
    
    int max=-1;
    double maxCorrolation=-1;
    for(int i=0;i<size[flipClause]; i++){
        if(node->getStateMask(getClause(flipClause, i))==false){
            if(max==-1){
                max=i;
                maxCorrolation=posCorrelationStatClauseVariable[flipClause][i];
            }else{
                if(maxCorrolation<posCorrelationStatClauseVariable[flipClause][i]){
                    maxCorrolation=posCorrelationStatClauseVariable[flipClause][i];
                    max=i;
                }
            }
        }
    }
    int flipBit = getClause(flipClause,max);
    return flipBit;
}

//We randomly select an unsatisfied claused and flip one of it's inputs
int SAT::pick_random(Node* node){
    int tofix = node->falseClause[random()%node->numfalse];
    int flipBit= getClause(tofix, random()%size[tofix]);
    //int flipBit = rand()%node->getState()->getSize();   //randomly selects a bit to flip.
    return flipBit;
}


//Selects should return which bit to flip in the state
int SAT::select(Node* node){
    bool randomFlag;
    double randomBias = 0.5;                    config->getParameter("param.randomBias", &randomBias);
    if( (double)rand()/(double)RAND_MAX < randomBias ){
        randomFlag=true;
    }
    
    
    int numbreak;
    int tofix;
    int clausesize;
    int i;
    int best[MAXLENGTH];
    register int numbest;
    register int bestvalue;
    register int var;
    
    tofix = node->falseClause[random()%node->numfalse];
    clausesize = size[tofix];
    numbest = 0;
    bestvalue = 9999999;
    
    for (i=0; i< clausesize; i++){
        var = getClause(tofix, i);
        numbreak = node->breakcount[var];
        if (numbreak<=bestvalue){
            if (numbreak<bestvalue) numbest=0;
            bestvalue = numbreak;
            best[numbest++] = var;
        }
    }
    
    if (bestvalue>0 && (randomFlag))
        return getClause(tofix, random()%clausesize );
    
    if (numbest == 1) return best[0];
    return best[random()%numbest];
    
    
    
    
    pair<int, int> pick1 = pick_best(node);
    int defaultCandidate = pick1.first;
//    cout << pick1.first << endl ;
  //  if(pick1.first==-1) {
    //    return pick_random(node);
    //}
    return pick1.first;
    
    /*
    
    
    
    
    bool randomFlag;
    double randomBias = 0.5;                    config->getParameter("param.randomBias", &randomBias);
        cout << "Pick:" << pick1.first << " " << pick1.second << " \t\t\t"<< randomBias << endl ;
    if( (double)rand()/(double)RAND_MAX < randomBias ){
        cout << "randomIsSet!" << endl;
        randomFlag=true;
    }
    
    if(pick1.first==-1){ //no viable candidate
        cout << "Picking random, no viable option!" << endl ;
        return pick_random(node);
    }else if(pick1.second>0 && randomFlag){ //best strategy is going to break something,
        cout << "Pick random again, no good strategy" << endl ;
        return pick_random(node);
    }else{ //pick best
        cout << "Picking the best" << endl ;
        return defaultCandidate;
    }
        
    /*string strategy;       config->getParameter("param.strategy", &strategy);
    if( strategy.compare("naive")==0){
        return pick_naive(node);
    }else if(strategy.compare("frequentist")==0){
        return pick_frequencist(node);
    }else if(strategy.compare("bayesian")==0){
        return pick_bayesian(node);
    }else if(strategy.compare("random")==0){
        return pick_random(node);
    }else if(strategy.compare("best")==0){
        return pick_best(node);
    }else{
        cout << "Undefined selection strategy" << endl;
        return -1;
    }*/
}

int* SAT::goal(){
    
    string strategy;       config->getParameter("param.bias", &strategy);
    double goalBias=0.5;   config->getParameter("param.randomBias", &goalBias);
    if( strategy.compare("uniform")==0){
        goalBias=0.5;
    }else if(strategy.compare("fixed")==0){
        goalBias= 0.5+0.5*(double)iter/double(maxIterations);
    }else if(strategy.compare("adaptive")==0){
        int period;       config->getParameter("param.biasPeriod", &period);
        double rise=0.5; double fall=1-0.5;
        int i=iter%period;   //1<i<200
        if(i<= rise*period){
            // we are in a rising period of bias, iteratively increase the bias toward the goal
            goalBias = 0.5 + 0.5*(double)i/(rise*period);
        }else{ //100<i<200
            goalBias = 1 - 0.5*(double)(i-rise*period)/(fall*period);
        }
    }else if(strategy.compare("variable")==0){
        double r = (double)rand()/(double)RAND_MAX;
        if( r>=0.5 ){
            goalBias = 1 ;
        //}else if (r>0.3){
 //           goalBias = 0.75;
        }else{
            goalBias=0.5;
        }
    }else{
        cout << "Uknown biasing strategy" << endl ;
    }

    int* data = new int[numclause];
    for(int i=0;i<numclause;i++){
        double x = (double)rand()/INT_MAX;
        if(x<goalBias)
            data[i]=1;
        else
            data[i]=0;
    }
    return data;
}

void SAT::solve(){
    bool satisfiableAssignmentFound = false;
    Node* solution;
    iter=0;
    config->getParameter("const.maxIterations", &maxIterations);
    g = new GraphFacade();
    //Add a random root state to the graph
    State* initialState = new State(numvariable);
    initialState->randomize();
    Output* initialOutput = adjust(initialState);
    Node* root = new Node(initialState, initialOutput);
    updateNodeStat(root);
    db->insert(root);
    g->add(root);
    if(root->isSatisfiable()){
        satisfiableAssignmentFound=true;
        solution=root;
    }

    while( !satisfiableAssignmentFound && iter<maxIterations ){
        if(iter%10==0){
            cout << iter << endl;
        }
        //1. generate a new goal output
        Output* goalClause = new Output(numclause);

        goalClause->randomize( goal() );
        // todo: check if we already reached the goal.

        //2. find the nearest output to the goal
        Output* nearestOutput = db->search(goalClause);
        
        //3. Find the state corresponding to the nearest output
        State* nearestState;
        for(int i=0;i<nearestOutput->getStateSize();i++){
            if (!nearestOutput->getState(i)->getNode()->getNodeMask()){
                nearestState=nearestOutput->getState(i);
                break;
            }
        }
        //State* nearestState = nearestOutput->getState( rand()%nearestOutput->getStateSize() );

        Node* nearestNode = nearestState->getNode();
        //4. Pick which bit to flip in the corresponding state
        int flipBit = select(nearestNode);
        Node* newNode = flip(nearestNode, flipBit);
        
        //int unsatCount = newNode->getOutput()->getunsat();
        //cout << "unsatCount " << iter << " " << unsatCount << endl ;
        if(!newNode->getDBFlag())
            db->insert(newNode);
        
        //5. Add the new generated state and the output to the graph
        g->add(newNode, nearestNode);
    
        //6. Check for satisfiability
        if(newNode->isSatisfiable()){
            solution=newNode;
            satisfiableAssignmentFound=true;
        }
        //7. update statistics
        
        iter++;
    }
    
    int* data = new int[numvariable];
    db->getState(data);
    if(satisfiableAssignmentFound){
        result->success=true;
        result->iterations=iter;
        result->states=db->stateSize();
        cout << "found a satisfiable assigment" << endl ;
        cout << solution->getState()->toString() << endl ;
    }else{
        result->success=false;
        cout << "No satisfiable assigment found." << endl ;
    }
    
    cout << "[info] Urbana execution finished." << endl ;
    cout << "[info] Total iterations =" << iter << endl ;
    cout << "[info] State space coverage = " << db->stateSize() << " / 2^" << numvariable << endl ;
    cout << "[info] Output clause space coverage = " << db->outputSize() << " / 2^" << numclause << endl ;
}