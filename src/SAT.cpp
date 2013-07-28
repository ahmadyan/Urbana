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
            int var = abs(clause[i][j])-1;
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
        goal->randomize();
        cout << goal->toString() << endl ;

        //2. find the nearest outout to the goal
        Output* nearestOutput = search(goal);
        cout << nearestOutput->toString() << endl ;
        
        //3. Find the state corresponding to the nearest output
        State* nearestState = nearestOutput->getState( rand()%nearestOutput->getStateSize() );
        Node* nearestNode = nearestState->getNode();
        
        //4. Pick which bit to flip in the corresponding state
        int flipBit = rand()%nearestState->getSize();
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