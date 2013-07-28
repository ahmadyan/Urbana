#include "Output.h"

Output::Output(int size):Object(size){
}

Output::~Output(){
}

void Output::addState(State* s){
    states.push_back(s);
}

int Output::getStateSize(){
    return (int)(states.size());
}

vector<State*> Output::getStates(){
    return states;
}

State* Output::getState(int i){
    return states[i];
}

bool Output::isSatisfiable(){
    bool answer=true;
    for(int i=0;i<size;i++){
        if(data[i]==0)
            answer=false;
    }
    return answer;
}