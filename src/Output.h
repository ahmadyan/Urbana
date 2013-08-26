#pragma once
#include <vector>
#include "object.h"

using namespace std;
class State;
class Output: public Object{
    double distanceFactor;
    vector<State*> states;
    bool mask;              //The output mask,
                             //If the output mask is 1, it means that given *current* states, there is no viable bit to flip.
                             //The output mask can change (from 1 to 0) if a new state is added to the list
                             //via update method
public:
    Output(int size);
    ~Output();
    
    void addState(State*);
    int getStateSize();
    vector<State*> getStates();
    State* getState(int);
    bool isSatisfiable();
    bool getMask();
    void setMask(bool);
    void updateMask();
    double getDistanceFactor();
    void setDistanceFactor(double);
};