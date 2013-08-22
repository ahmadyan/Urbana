#include "State.h"

State::State(int size):Object(size){
    if(size>0)
        mask = new int[size]();
}

State::State(State* s):Object(s->getSize()){
    for(int i=0;i<s->getSize();i++){
        data[i] = s->data[i];
    }
    mask = new int[s->getSize()](); // the mask bits are not copied over. 
}

State::State(){
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

int State::getMask(int i){
    return mask[i];
}

void State::setMask(int i, int j){
    mask[i] = j;
}
