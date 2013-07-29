#pragma once
#include <vector>
#include "object.h"

using namespace std;
class State;
class Output: public Object{
    vector<State*> states;
public:
    Output(int size);
    ~Output();
    
    void addState(State*);
    int getStateSize();
    vector<State*> getStates();
    State* getState(int);
    bool isSatisfiable();
};