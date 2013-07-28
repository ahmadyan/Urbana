#include "State.h"

State::State(int size):Object(size){
}

State::State(State* s):Object(s->getSize()){
    for(int i=0;i<size;i++){
        data[i] = s->data[i];
    }
}

State::~State(){
}

void State::setOutput(Output* o){
    output = o;
}

Output* State::getOutput(){
    return output;
}

Node* State::getNode(){
    return node;
}

void State::setNode(Node* n){
    node = n;
}
