#include "Node.h"

Node::Node(State* s, Output* o){
    int numclause=o->getSize();
    int numvariable=s->getSize();
    state=s;
    output=o;
    nodeNumber=-1;
    dbFlag=false;
    graphFlag=false;
    
    s->setOutput(o);
    s->setNode(this);
    o->addState(s);
    
    nodeMask=false;
    stateMask = new bool[numvariable]();
    outputMask = new bool[numclause]();
    
    
    numtruelit = new int[numclause]();		// number of true literals in each clause
    wherefalse = new int[numclause]();		// where each clause is listed in false
    falseClause= new int[numclause]();		// clauses which are false
    breakcount = new int[numvariable]();     // number of clauses that become unsat if var if flipped
    makecount = new int[numvariable]();      // number of clauses that become sat if var if flipped
    numfalse=0;                               // number of false clauses
    
}

Node::~Node(){
    delete numtruelit;
    delete wherefalse;
    delete falseClause;
    delete breakcount;
    delete makecount;
    delete stateMask;
    delete outputMask;
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

string Node::toString(){
    stringstream ss ;
    ss << "Streaming node information" << endl ;
    ss << "state=" << state->toString() << endl ;
    ss << "output=" << output->toString() << endl ;
        
    ss << "numtruelit:" ;
    for(int i=0;i<output->getSize();i++){
        ss << numtruelit[i] << " ,";
    }
    ss << endl ;
    
    
    ss << "wherefalse:" ;
    for(int i=0;i<output->getSize();i++){
        ss << wherefalse[i] << " ,";
    }
    ss << endl ;
    
    
    
    ss << "falseClause:" ;
    for(int i=0;i<output->getSize();i++){
        ss << falseClause[i] << " ,";
    }
    ss << endl ;
    
    
    ss << "breakcount:" ;
    for(int i=0;i<state->getSize();i++){
        ss << breakcount[i] << " ,";
    }
    ss << endl ;
    
    ss << "makecount:" ;
    for(int i=0;i<state->getSize();i++){
        ss << makecount[i] << " ,";
    }
    ss << endl ;
    
    ss << "numfalse=" << numfalse << endl ;
    
    
    return ss.str();
}
