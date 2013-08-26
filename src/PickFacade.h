#pragma once

#include "SAT.h"
#include "database.h"
#include "Node.h"

class PickFacade{
public:
    SAT* sat;
    Database* db;
    
    PickFacade(SAT* sat, Database* db);
    virtual int pick(Node* node)=0;
};

