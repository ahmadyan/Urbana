#pragma once
#include "object.h"
// We consider a set of inputs as state,
// There is a one to one relation between State class and Clauses class, but not vice versa
class Output;
class Node;

class State: public Object{
    Output* output;
    Node* node;
public:
    State();            //default constructor will create a null boject
    State(int size);
    State(State*);
    ~State();
    void setOutput(Output*);
    Output* getOutput();
    Node* getNode();
    void setNode(Node*);
};
