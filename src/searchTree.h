#pragma once
#include "object.h"

class SearchTree{
protected:
    Object* root;
public:
    virtual void insert(Object*)=0;
    virtual Object* search(int*)=0;
};