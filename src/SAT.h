#pragma once
#include <iostream>
#include <vector>

#include "Output.h"
#include "State.h"
#include "Graph.h"
#include "config.h"
using namespace std;

class SAT{
    int numvariable;
    int numclause;

    Configuration* config;
    vector<Output*> exploredOutputs;
    vector<State*> exploredStates;
    Graph* g;
    
    int** clause;                   // clauses to be satisfied indexed as clause[clause_num][literal_num]
    int** occurrence;	// where each literal occurs indexed as occurrence[literal][occurrence_num]

    
    int* size;             // length of each clause
    int* numOccurence; // number of times each literal occurs
    int* numOccurencePos;
    int* numOccurenceNeg;
    
    //statistics variables
    double* signalProbabilityStat;
    double** causalityStat;

public:
    SAT(Configuration*);
    ~SAT();
    void init();
    void solve();
    Output* search(Output*);
    Output* update(State* state);
    Node* flip(Node*, int);
    int GetNumberOfVariables();
    int GetNumberOfClauses();

};