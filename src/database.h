#pragma once

#include <vector>
#include <boost/intrusive/avl_set.hpp>
#include <algorithm>

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <assert.h>

#include "State.h"
#include "object.h"
#include "Output.h"
#include "Node.h"
#include "config.h"

namespace mvp{
    extern "C"{
        #include "mvptree.h"
    }
}
class Database{
    int enableMVP;
    int MVP_BRANCHFACTOR;
    int MVP_PATHLENGTH;
    int MVP_LEAFCAP;
    std::vector<Node*> nodes;
    std::vector<Output*> outputs;
    std::vector<State*> states;
    
    boost::intrusive::avltree<State> stateAVL; // Binary search tree is used for fast exact search (O(log n))
    boost::intrusive::avltree<Output> outputAVL; // Binary search tree is used for fast exact search (O(log n))
    mvp::MVPTree* outputVPT;    // Vintage Point tree is used for fast nearest search (O(log d))
    void insert(State*);
    void insert(Output*);
public:
    
    Database(Configuration*);
    ~Database();
    
    void insert(Node* node);
    void toString();
    int stateSize();
    int outputSize();
    int size();
    static float hamming_distance(mvp::MVPDP *pointA, mvp::MVPDP *pointB);
    
    State* getState(int i);     // quick state lookup, O(1)
    State* getState(int*);      // searches for the state with given data, O(log n)
    Output* getOutput(int i);   // Quick output lookup, O(1)
    Output* getOutput(int* data);
    Output* search(Output*);
    Node* operator[](const int) throw (const char*);
};