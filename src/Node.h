#pragma once

#include "Output.h"
#include "State.h"

class Node{
    Output* output;
    State* state;
    int nodeNumber;
public:
    Node(State*, Output*);
    int getNodeNumber();
    void setNodeNumber(int);
    
    State* getState();
    Output* getOutput();
    bool isSatisfiable();
};