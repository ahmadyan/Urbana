#include "database.h"

Database::Database(Configuration* config){
    config->getParameter("param.mvp.MVP_BRANCHFACTOR", &MVP_BRANCHFACTOR);
    config->getParameter("param.mvp.MVP_PATHLENGTH", &MVP_PATHLENGTH);
    config->getParameter("param.mvp.MVP_LEAFCAP", &MVP_LEAFCAP);
    cout << "Booting DB" << endl;
    cout << MVP_BRANCHFACTOR << " " << MVP_PATHLENGTH << " " << MVP_LEAFCAP << endl ;
    mvp::CmpFunc distance_func = hamming_distance;
    outputVPT = mvp::mvptree_alloc(NULL, distance_func, MVP_BRANCHFACTOR, MVP_PATHLENGTH, MVP_LEAFCAP);
    
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
        //insert the output into the vector for direct access, just in case
        outputs.push_back(output);
        
        //insert the output to the MVT tree for nearest node queries.
        //Creating the MVT node
        mvp::MVPDP* node = new mvp::MVPDP();
        node->datalen = output->getSize();
        node->data = output;
        char scratch[32];
        snprintf(scratch, 32, "point%llu", output->getID());
        node->id = strdup(scratch);
        cout << node->id << "   " << output->toString() << endl ;
        
        int err = mvp::mvptree_add(outputVPT, &node, 1);
        assert(err == mvp::MVP_SUCCESS);
        
       // outputVPT.insert(output);
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
    mvp::MVPError err;
    float radius=7;
    unsigned int nbresults;
    
    mvp::MVPDP* node = new mvp::MVPDP();
    node->datalen = goal->getSize();
    node->data = goal;
    char scratch[32];
    snprintf(scratch, 32, "point%llu", goal->getID());
    node->id = strdup(scratch);
    cout << node->id << "   " << goal->toString() << endl ;
    

    unsigned int knearest = 10;
    
    mvp::MVPDP **results = mvp::mvptree_retrieve(outputVPT, node, knearest, radius, &nbresults, &err);
    int minimumDistance = 9999;

    for (int i=0;i<nbresults;i++){
        fprintf(stdout,"  FOUND --> (%d) %s  : ", i, results[i]->id);
        cout << ((Output*)(results[i]->data))->toString() << endl ;
   
        
        Output* out = ((Output*)(results[i]->data));
        if(!out->getMask()){
            return out;
        }
        
                
         }
    assert(results);
    assert(err == mvp::MVP_SUCCESS);
    
    

    /*    int minimumDistance = 9999;
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
*/
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

float Database::hamming_distance(mvp::MVPDP *pointA, mvp::MVPDP *pointB){
    if (!pointA || !pointB) {
        return -1.0f;
    }
    if (!pointA->data || !pointB->data) {
        return -1.0f;
    }
    return ((Output*)(pointA->data))->distance( (Output*)(pointB->data));
}

