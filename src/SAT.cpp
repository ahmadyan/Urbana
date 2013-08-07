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
	clause = new int*[numclause];
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
        
        clause[i] = new int[size[i]];
        for(int j=0;j<tmp.size();j++){
            //in cnf format, the vars start from 1, in c++ we index them at 0, so -1 for the var and +1 for the negation. 
            clause[i][j] = tmp[j]>0? tmp[j]-1 : tmp[j]+1;
        }
	}
    
    cout << "size of each clause:" << endl ;
    for(int i=0;i<numclause;i++){
     //           cout << clause[i] << endl ;
        cout << size[i] << endl ;
        for(int j=0;j<size[i];j++){
            cout << clause[i][j] << " " ;
        }
        cout << endl ;
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
            int var = abs(clause[i][j]);
            occurrence[var][ occtmp[var]] = i;
            occtmp[var]++;
		}
	}
    delete occtmp;
}

SAT::~SAT(){
}

int SAT::GetNumberOfVariables(){
    return numvariable;
}

int SAT::GetNumberOfClauses(){
    return numclause;
}

// This function recieves the state as input & computes the outpus and some statistics
Output* SAT::update(State* state){
    // Initialize breakcount and makecount in the following:
    int* numtruelit = new int[numclause];		// number of true literals in each clause
    int* wherefalse = new int[numclause];		// where each clause is listed in false
    int* falseClause= new int[numclause];		// clauses which are false
    int* breakcount = new int[numvariable];     // number of clauses that become unsat if var if flipped
    int* makecount = new int[numvariable];      // number of clauses that become sat if var if flipped

    int numfalse=0;                               // number of false clauses
    
    for(int i = 0;i < numclause;i++){
        numtruelit[i] = 0;
        wherefalse[i] = 0;
    }
    
    int thetruelit;
    
    // check these codes, probably buggy
    for(int i=0;i<numclause;i++){
        for(int j = 0;j<size[i];j++){
            if((clause[i][j] > 0) ==  state->get(abs(clause[i][j]))){
                numtruelit[i]++;
                thetruelit = clause[i][j];
            }
        }
        if(numtruelit[i] == 0){
            wherefalse[i] = numfalse;
            falseClause[numfalse] = i;
            numfalse++;
            for(int j = 0;j < size[i];j++){
                makecount[abs(clause[i][j])]++;
            }
        }else if (numtruelit[i] == 1){
            breakcount[abs(thetruelit)]++;
        }
    }
    
    Output* output = new Output(numclause);
    for(int i=0;i<numclause;i++){
        output->set(i, 0);
        for(int j=0;j<size[i];j++){
            if(((clause[i][j]>0) && state->get(abs(clause[i][j]))==1 )   ||
               ((clause[i][j]<0) && state->get(abs(clause[i][j]))==0 )   ){
                output->set(i, 1);
            }
        }
    }
    return output;
}

int distance(Output* source, Output* target){
    int distance=0;
    for(int i=0;i<source->getSize(); i++){
        if(source->get(i)!=target->get(i)){
            distance++;
        }
    }
    return distance;
}

//This needs some serious work, this is the most naive way of search.
Output* SAT::search(Output* goal){
    int minimumDistance = 9999;
    Output* result = exploredOutputs[0];
    for(int i=0;i<exploredOutputs.size();i++){
        if(distance(goal, exploredOutputs[i]) < minimumDistance ){
            result = exploredOutputs[i];
        }
    }
    return result;
}

//needs optimization
Node* SAT::flip(Node* node, int flipbit){
    State* state = new State(node->getState());
    state->set(flipbit, (state->get(flipbit)==0)? 1 : 0 );
    Output* output = update(state);
    Node* flippedNode = new Node(state, output);
    
    exploredStates.push_back(state);
    exploredOutputs.push_back(output);
    
    return flippedNode;
}

//The initial training part of the algorithm, we later update the algorithm using the statistics
//genereated from the algorithm itself
void SAT::init(){
    int trainingSamples=100;
    signalProbabilityStat = new double[numclause];
    correlationStatVariableClause = new double*[numvariable];
    correlationStatClauseVariable = new double*[numclause];
    posCorrelationStatVariableClause = new double*[numvariable];
    posCorrelationStatClauseVariable = new double*[numclause];
    
    
    // posCorrelationStatVariableClause
    
    for(int i=0;i<numvariable;i++) correlationStatVariableClause[i]=new double[numclause];
    for(int i=0;i<numclause;i++) correlationStatClauseVariable[i]=new double[numvariable];
    for(int i=0;i<numvariable;i++) posCorrelationStatVariableClause[i]=new double[numclause];
    for(int i=0;i<numclause;i++) posCorrelationStatClauseVariable[i]=new double[numvariable];
    
    for(int i=0;i<numclause;i++){
        signalProbabilityStat[i]=0;
    }
    for(int i=0;i<numvariable;i++){
        for(int j=0;j<numclause;j++){
            correlationStatVariableClause[i][j]=0;
            correlationStatClauseVariable[j][i]=0;
            posCorrelationStatVariableClause[i][j]=0;
            posCorrelationStatClauseVariable[j][i]=0;
        }
    }
    
    for(int i=0;i<trainingSamples;i++){
        State* s = new State(numvariable);
        s->randomize();
        Output* o = update(s);
        cout << "Output tostring " << o->toString() << endl ;
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
    
    for(int i=0;i<numclause;i++){
        cout << "clause:" << i << " " << signalProbabilityStat[i] << endl;
    }
    for(int i=0;i<numvariable;i++){
        cout << "var " << i << " " ;
        for(int j=0;j<numOccurence[i];j++){
            int clause = abs(occurrence[i][j]);
            cout << "(" << clause << ") " ;
            cout << correlationStatVariableClause[i][clause] << "(" << posCorrelationStatVariableClause[i][clause]  << ") ";
        }
        cout << endl ;
    }

    for(int i=0;i<numclause;i++){
        cout << "clause " << i << " " ;
        for(int j=0;j<size[i];j++){
            
            int var = abs(clause[i][j]);
            cout << "("<< var << ") " ;
            cout << correlationStatClauseVariable[i][var] << "(" << posCorrelationStatClauseVariable[i][var] << ")    ";
        }
        cout << endl ;
    }
    
    
    
}

//Selects should return which bit to flip in the state
int SAT::select(Node * node){
    string strategy;       config->getParameter("param.strategy", &strategy);
    if( strategy.compare("walksat")==0){
        return -1;
    }else if(strategy.compare("frequentist")==0){
        //We have to answer two qeustions:
        //1. Which clause C we want to turn satisfy at this iteration? Which is the easiest?
        //2. Which bit X to flip in order to satisfy C (and probably unsatisfy some other clauses).
        cout << "frequentist strategy 1" << endl ;
        int flipClause=-1;
        for(int i=0;i<numclause;i++){
            if(node->getOutput()->get(i)==0){
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
        
        cout << "flip clause=" << flipClause << endl;
        int max=0;
        double maxCorrolation=0;
        
        for(int i=0;i<size[flipClause]; i++){
            if(maxCorrolation<posCorrelationStatClauseVariable[flipClause][i]){
                maxCorrolation=posCorrelationStatClauseVariable[flipClause][i];
                max=i;
            }
        }
        int flipBit = abs(clause[flipClause][max]);
        cout << "flip bit=" << flipBit << endl ;
        return flipBit;
    }else if(strategy.compare("bayesian")==0){
        int flipClause=-1;
        int flipBit=0;
        
        for(int i=0;i<numclause;i++){
            if(node->getOutput()->get(i)==0){
                for(int j=0;j<size[i];j++){
                    //compute some sort of bayesian
                    
                }
            }
            
        }
        
        return flipBit;
    }else{
        int flipBit = rand()%node->getState()->getSize();   //randomly selects a bit to flip.
        return flipBit;
    }
}

void SAT::solve(){
    bool satisfiableAssignmentFound = false;
    int iter = 0;
    int maxIterations; config->getParameter("const.maxIterations", &maxIterations);

    g = new Graph();
    
    //Add a random root state to the graph
    State* initialState = new State(numvariable);
    initialState->randomize();
    Output* initialOutput = update(initialState);
    exploredStates.push_back(initialState);
    exploredOutputs.push_back(initialOutput);
    Node* root = new Node(initialState, initialOutput);
    g->addRoot(root);
    
    while( !satisfiableAssignmentFound && iter<maxIterations ){
        //1. generate a new goal output
        Output* goal = new Output(numclause);
        double goalBias= 0.5+0.5*(double)iter/double(maxIterations);
        goal->randomize( goalBias );
        cout << "goal=" << goal->toString() << "    (bias)=" << goalBias << endl ;

        //2. find the nearest outout to the goal
        Output* nearestOutput = search(goal);
        cout << "near=" << nearestOutput->toString() << endl ;
        
        //3. Find the state corresponding to the nearest output
        State* nearestState = nearestOutput->getState( rand()%nearestOutput->getStateSize() );
        Node* nearestNode = nearestState->getNode();
        
        //4. Pick which bit to flip in the corresponding state
        int flipBit = select(nearestNode);
        Node* newNode = flip(nearestNode, flipBit);
        
        g->addNode(newNode, nearestNode);
        cout << "current state=" << newNode->getState()->toString() << "  --> " << newNode->getOutput()->toString() << endl ;
        //5. Add the new generated state and the output to the graph
        
        
        //6. Check for satisfiability
        if(newNode->isSatisfiable()){
            cout << "found a satisfiable assigment" << endl ;
            cout << newNode->getState()->toString() << " --> " << newNode->getOutput()->toString() << endl ;
            satisfiableAssignmentFound=true;
        }
        //7. update statistics
        
        iter++;
    }
    
}
/*

void init(char initfile[], int initoptions){
	printf("[adel-dbg] Init: numclauses:%d numatoms%d\n", numclause, numatom);
	int i;
	int j;
	int thetruelit;
	FILE * infile;
	int lit;
    
	for(i = 0;i < numclause;i++)
		numtruelit[i] = 0;
	numfalse = 0;
    
	for(i = 1;i < numatom+1;i++){
		changed[i] = -BIG;
		breakcount[i] = 0;
		makecount[i] = 0;
	}
    
	if (initfile[0] && initoptions!=INIT_PARTIAL){
		for(i = 1;i < numatom+1;i++)
			atom[i] = 0;
	}else{
		for(i = 1;i < numatom+1;i++)
			atom[i] = random()%2;
	}
    
	if (initfile[0]){
		if ((infile = fopen(initfile, "r")) == NULL){
			fprintf(stderr, "Cannot open %s\n", initfile);
			exit(1);
		}
		i=0;
		while (fscanf(infile, " %i", &lit)==1){
			i++;
			if (ABS(lit)>numatom){
				fprintf(stderr, "Bad init file %s\n", initfile);
				exit(1);
			}
			if (lit<0) atom[-lit]=0;
			else atom[lit]=1;
		}
		if (i==0){
			fprintf(stderr, "Bad init file %s\n", initfile);
			exit(1);
		}
		fclose(infile);
	}
    
	// Initialize breakcount and makecount in the following:
	for(i = 0;i < numclause;i++)
	{
		for(j = 0;j < size[i];j++)
		{
			if((clause[i][j] > 0) == atom[ABS(clause[i][j])])
			{
				numtruelit[i]++;
				thetruelit = clause[i][j];
			}
		}
		if(numtruelit[i] == 0)
		{
			wherefalse[i] = numfalse;
			false[numfalse] = i;
			numfalse++;
			for(j = 0;j < size[i];j++){
				makecount[ABS(clause[i][j])]++;
			}
		}
		else if (numtruelit[i] == 1)
		{
			breakcount[ABS(thetruelit)]++;
		}
	}
    
	if (hamming_flag){
		hamming_distance = calc_hamming_dist(atom, hamming_target, numatom);
		fprintf(hamming_fp, "0 %i\n", hamming_distance);
	}
	
	if(rrtflag) rrt_init();
}

*/