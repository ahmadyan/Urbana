#pragma once
#include <iostream>
#include <vector>
//#include <utility>

#include "Output.h"
#include "State.h"
#include "Graph.h"
#include "config.h"
#include "database.h"
#include "Result.h"
using namespace std;

class SAT{
    int iter;
    int numvariable;
    int numclause;
    int maxIterations;

    Configuration* config;
    Result* result;
    Database* db; 
    GraphFacade* g;
    
    std::pair<int, bool>** clause;                   // clauses to be satisfied indexed as clause[clause_num][literal_num]
    int** occurrence;               // where each literal occurs indexed as occurrence[literal][occurrence_num]

    
    int* size;             // length of each clause
    int* numOccurence; // number of times each literal occurs
    int* numOccurencePos;
    int* numOccurenceNeg;
    
    int enableLearning;
    //statistics variables
    double* signalProbabilityStat;
    double** posCorrelationStatVariableClause;
    double** posCorrelationStatClauseVariable;
    double** correlationStatVariableClause;
    double** correlationStatClauseVariable;
    
    double** probability;
    
    //mask
    int enablePreProcess;
    int* preValue;
    int* variableMask;
    int* clauseMask;
    
    int preDeterminedClauses;
    int preDeterminedVariables;

    int* size_original;
    int* numOccurence_original;
    int* numOccurencePos_original;
    int* numOccurenceNeg_original;
    std::pair<int, bool>** clause_original;
    int** occurrence_original;
    int numvariable_original;
    int numclause_original;
    int* variableMap1;
    int* variableMap2;
    int* clauseMap1;
    int* clauseMap2;

public:
    SAT(Configuration*);
    SAT(SAT*);
    ~SAT();
    int* goal();
    void init();
    void solve();
    Result* getResult();
    int select(Node*);
    
    int getClause(int, int);
    bool getSign(int, int);
    Output* search(Output*);
    Output* adjust(State* state);
    Node* flip(Node*, int);
    int GetNumberOfVariables();
    int GetNumberOfClauses();
    void update(Node*, int);
    void updateNodeStat(Node* node);    //update node statistics such as makecount/ break count, etc.
    
    int pick_naive(Node* node);
    int pick_walksat2(Node* node);
    pair<int, int> pick_best(Node* node);
    int pick_frequencist(Node* node);
    int pick_random(Node* node);
    
    //int (*pickcode[6])(Node*) = {pick_naive, pick_walksat2, pick_walksat1, pick_frequencist, pick_bayesian, pick_random};
};