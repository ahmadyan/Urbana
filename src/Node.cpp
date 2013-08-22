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
