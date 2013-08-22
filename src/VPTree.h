#pragma once
#include "SearchTree.h"
#include "object.h"
class VintagePointTree: SearchTree{
  //  Object* root;
  //  int vp_size;
public:
    VintagePointTree();
    ~VintagePointTree();
    
    void insert(Object*);
    Object* search(int*);
 //   int size();
};