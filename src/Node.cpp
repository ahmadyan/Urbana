#include "Node.h"

Node::Node(State* s, Output* o){
    state=s;
    output=o;
    nodeNumber=-1;
    
    s->setOutput(o);
    s->setNode(this);
    o->addState(s);
}

int Node::getNodeNumber(){
    return nodeNumber;
}

void Node::setNodeNumber(int n){
    nodeNumber=n;
}

State* Node::getState(){
    return state;
}

Output* Node::getOutput(){
    return output;
}

bool Node::isSatisfiable(){
    return output->isSatisfiable();
}