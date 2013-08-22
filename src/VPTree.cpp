#include "VPTree.h"

VintagePointTree::VintagePointTree(){

}

VintagePointTree::~VintagePointTree(){
    
}
 
void VintagePointTree::insert(Object* node){
    root=node;
}

Object* VintagePointTree::search(int* data){
    return root;
}
