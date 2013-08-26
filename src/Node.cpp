#include "Node.h"

Node::Node(State* s, Output* o){
    state=s;
    output=o;
    nodeNumber=-1;
    dbFlag=false;
    graphFlag=false;
    
    s->setOutput(o);
    s->setNode(this);
    o->addState(s);
    
    nodeMask=false;
    stateMask = new bool[s->getSize()]();
    outputMask = new bool[o->getSize()]();
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

bool Node::getDBFlag(){
    return dbFlag;
}

void Node::setDBFlag(bool db){
    dbFlag=db;
}

bool Node::getGraphFlag(){
    return graphFlag;
}
void Node::setGraphFlag(bool flag){
    graphFlag=flag;
}

void Node::setVertex(Vertex_t v){
    graphFlag=true;
    vertex=v;
}

Vertex_t Node::getVertex(){
    return vertex;
}

bool Node::getNodeMask(){
    return nodeMask;
}

bool Node::getStateMask(int i){
    return stateMask[i];
}

bool Node::getOutputMask(int i){
    return outputMask[i];
}

void Node::setNodeMask(bool value){
    nodeMask = value;
}

void Node::setStateMask(int i, bool value){
    stateMask[i]=value;
}

void Node::setOutputMask(int i, bool value){
    outputMask[i] = value;
}
