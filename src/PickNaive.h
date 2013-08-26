#pragma once

#include "PickFacade.h"

class PickNaive: public PickFacade{
public:
    PickNaive(SAT* sat, Database* db);
    int pick(Node* node);
};