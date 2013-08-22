#include "database.h"

Database::Database(){
    
}

Database::~Database(){
    
}

void Database::insert(Node* node){
    node->setDBFlag(true);
    nodes.push_back(node);
    insert(node->getState());
    insert(node->getOutput());
}

//Insert a new state to the database, Complexity: O(log n)
void Database::insert(State* state){
    cout << "DB-flag is " << state->getDBFlag() << " " << state->getID()<< endl ;
    if(!state->getDBFlag()){
        stateAVL.insert_unique(*state);
        states.push_back(state);
        state->setDBFlag(true);
    }
}

//Insert a new output to the database, Complexity: O(1)
void Database::insert(Output* output){
    if(!output->getDBFlag()){
        outputs.push_back(output);
        outputVPT.insert(output);
        output->setDBFlag(true);
    }
}

State* Database::getState(int* data){
    cout << "Searching" << endl ;for(int i=0;i<5;i++){ cout << data[i] ; } cout << endl ;
    boost::intrusive::avltree<State>::iterator it = stateAVL.find(data, Object());
    if( it != stateAVL.end() ){
        cout << "Object found: " << (&*it)->toString() << endl ;
        return &*it;
    }else{
        cout << "Object not found" << endl ;
        return new State();
    }
}

void Database::toString(){
    for(boost::intrusive::avltree<State>::iterator it = stateAVL.begin(); it!= stateAVL.end(); it++){
        cout << it->toString() << endl ;
    }
}

int Database::size(){
    return (int)(nodes.size());
}

int Database::numberOfStates(){
    return (int)(states.size());
}

int Database::numberOfOutputs(){
    return (int)(outputs.size());
}

State* Database::getState(int i){
    return states[i];
}

Output* Database::getOutput(int i){
    return outputs[i];
}

Node* Database::operator[](const int i) throw (const char*){
    if(i<0 || i>nodes.size() ) throw "Invalid access to database" ;
    else return nodes[i];
}
