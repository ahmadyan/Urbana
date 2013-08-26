#pragma once

#include <vector>
#include <boost/intrusive/avl_set.hpp>
#include <algorithm>
#include "State.h"
#include "object.h"
#include "Output.h"
#include "Node.h"
#include "VPTree.h"

class Database{
    std::vector<Node*> nodes;
    std::vector<Output*> outputs;
    std::vector<State*> states;
    
    boost::intrusive::avltree<State> stateAVL; // Binary search tree is used for fast exact search (O(log n))
    VintagePointTree outputVPT;   // Vintage Point tree is used for fast nearest search (O(log d))

    void insert(State*);
    void insert(Output*);
public:
    
    Database();
    ~Database();
    
    void insert(Node* node);
    void toString();
    int stateSize();
    int outputSize();
    int size();
    
    State* getState(int i);     // quick state lookup, O(1)
    State* getState(int*);      // searches for the state with given data, O(log n)
    Output* getOutput(int i);   // Quick output lookup, O(1)
    
    Node* operator[](const int) throw (const char*);
};