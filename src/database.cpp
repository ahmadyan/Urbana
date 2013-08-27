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
    boost::intrusive::avltree<State>::iterator it = stateAVL.find(data, Object());
    if( it != stateAVL.end() ){
        return &*it;
    }else{
        return new State();
    }
}


//This needs some serious work, this is the most naive way of search.
//Todo: add a weight factor to the search, if we search a node once, counter++
//Successfull searches (searched that resulted in clauses with higher positives) vs unsuccessfull searches
//if we pick one node a lot, next time, lower the chance of picking it
//or among nodes with distance <=k, pick a node with lowest pick-up.
Output* Database::search(Output* goal){
    int minimumDistance = 9999;
    Output* result = getOutput(0);
    for(int i=0;i< outputSize();i++){
        if(!getOutput(i)->getMask()){
            if( getOutput(i)->getDistanceFactor()*distance(goal, getOutput(i)) < minimumDistance ){
                result = getOutput(i);
            }
        }
    }
    result->miss();
    if(result->getMask()){
        cout << "Woah! The search space is empty, we should restart the urbana" << endl ;
    }
    return result;
}


void Database::toString(){
    for(boost::intrusive::avltree<State>::iterator it = stateAVL.begin(); it!= stateAVL.end(); it++){
        cout << it->toString() << endl ;
    }
}

int Database::size(){
    return (int)(nodes.size());
}

int Database::stateSize(){
    return (int)(states.size());
}

int Database::outputSize(){
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
