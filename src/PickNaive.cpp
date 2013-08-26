#include "PickNaive.h"
#include <iostream>
using namespace std;

PickNaive::PickNaive(SAT* _sat, Database* _db):PickFacade(_sat, _db){}

int PickNaive::pick(Node* node){
 /*   cout << "Pick Naive!" << endl ;
    //step 1: pick which clause to flip
    int flipClause=-1;
    for(int i=0;i<sat->numclause;i++){
        if( node->getOutputMask(i)==false && node->getOutput()->get(i)==0   ){
            flipClause=i;
        }
    }
    if(flipClause==-1) cout << "This shouldn't happen!, the node should already have been masked" << endl ;
    
    //Step 2: pick which variable to flip
    for(int i=0;i<size[flipClause]; i++){
        int flipBit = abs( clause[flipClause][i]);
        if( node->getStateMask(flipBit)==false ){
            return flipBit;
        }
    }
    cout << "This shouldn't happend! all variables are masked, but the clause itself isn't masked" << endl ;
    return -1;*/
}