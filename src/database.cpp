
#include "database.h"

Database::Database(Configuration* config){
    config->getParameter("param.mvp.MVP_BRANCHFACTOR", &MVP_BRANCHFACTOR);
    config->getParameter("param.mvp.MVP_PATHLENGTH", &MVP_PATHLENGTH);
    config->getParameter("param.mvp.MVP_LEAFCAP", &MVP_LEAFCAP);
    config->getParameter("param.mvp.enable", &enableMVP);
    cout << "Booting DB" << endl;
    if(enableMVP){
        cout << "initializing MVP" << endl ;
        cout << MVP_BRANCHFACTOR << " " << MVP_PATHLENGTH << " " << MVP_LEAFCAP << endl ;
        mvp::CmpFunc distance_func = hamming_distance;
        outputVPT = mvp::mvptree_alloc(NULL, distance_func, MVP_BRANCHFACTOR, MVP_PATHLENGTH, MVP_LEAFCAP);
    }
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
        //cout << "Inserting a new node to the DB" << endl ;
        //insert the output into the vector for direct access, just in case
        outputs.push_back(output);
        
        //insert the output into the AVL tree for quick lookup
        outputAVL.insert_unique(*output);
        
        if(enableMVP){
            //insert the output to the MVT tree for nearest node queries.
            //Creating the MVT node
            mvp::MVPDP* node = new mvp::MVPDP();
            node->datalen = output->getSize();
            node->data = output;
            char scratch[32];
            snprintf(scratch, 32, "point%llu", output->getID());
            node->id = strdup(scratch);
            mvp::MVPError err = mvp::mvptree_add(outputVPT, &node, 1);
            //cout << "Error code=" << err << endl ;
            
            if(err != mvp::MVP_SUCCESS){
                cout << "[error] db::mvp " << mvp::mvp_errstr( err ) << endl;
            }
            assert(err == mvp::MVP_SUCCESS);
        }
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

Output* Database::getOutput(int* data){
    boost::intrusive::avltree<Output>::iterator it = outputAVL.find(data, Object());
    if( it != outputAVL.end() ){
        return &*it;
    }else{
        return new Output();
    }
}

//Searching for nearest clauses
Output* Database::search(Output* goal){
    if(enableMVP==0){
        int minimumDistance = INT_MAX;
        Output* result = getOutput(0);
        for(int i=0;i< outputSize();i++){
            if(!getOutput(i)->getMask()){
                if( getOutput(i)->getDistanceFactor()*goal->distance(getOutput(i)) < minimumDistance ){
                    minimumDistance=getOutput(i)->getDistanceFactor()*goal->distance(getOutput(i));
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
    
    
    int K = goal->getSize();            //number of K-nearest neighbor that we are looking at, should be equal to #clauses at least
    //todo: find optimum K, how much trouble will I get into if I don't set K to #clauses?
    //in case that we have less than K output in the database
    if(outputSize()<=K){
        //perform linear search
        int minimumDistance = INT_MAX;
        Output* result = getOutput(0);
        for(int i=0;i< outputSize();i++){
            if(!getOutput(i)->getMask()){
                if( getOutput(i)->getDistanceFactor()*goal->distance(getOutput(i)) < minimumDistance ){
                    minimumDistance= getOutput(i)->getDistanceFactor()*goal->distance(getOutput(i));
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
    
    float radius = 1;
    mvp::MVPError err;
    unsigned int nbresults=0;
    mvp::MVPDP **results;
    while(nbresults<K){
        //gradually increase radius until we find at least K nearest neighbors
        //Clean-up
        if(results!=0) delete results;
        //Create a new MVP-node
        mvp::MVPDP* node = new mvp::MVPDP();
        node->datalen = goal->getSize();
        node->data = goal;
        char scratch[32];
        snprintf(scratch, 32, "point%llu", goal->getID());
        node->id = strdup(scratch);
        
        //search the mvp for K nearest neighbor within radius (the actual number of neaerest neighbor might be more than just K)
        results = mvp::mvptree_retrieve(outputVPT, node, K, radius, &nbresults, &err);
        
        radius*=5;      //todo: find the optimum variable, how sensitive are we on thing thing?
    }
    
    //The MVP is going to nag becuase there are usually more than just K nearest neighbor in the radius we are searching
    //if(err != mvp::MVP_SUCCESS){
    //    cout << "[error] db::mvp " << mvp::mvp_errstr( err ) << endl;
    //}
    
    //Finally, choose the suitable candidate among those N nodes
    int minimumDistance = INT_MAX;
    Output* result = ((Output*)(results[0]->data));
    for (int i=1;i<nbresults;i++){
        Output* current = (Output*)(results[i]->data);
        double currentDistance=goal->distance(current);
        if(!current->getMask()){
            if( current->getDistanceFactor()*currentDistance < minimumDistance ){
                minimumDistance=current->getDistanceFactor()*currentDistance;
                result = current;
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

float Database::hamming_distance(mvp::MVPDP *pointA, mvp::MVPDP *pointB){
    if (!pointA || !pointB) {
        return -1.0f;
    }
    if (!pointA->data || !pointB->data) {
        return -1.0f;
    }
    return ((Output*)(pointA->data))->distance( (Output*)(pointB->data));
}

