#pragma once
#include <vector>
#include "object.h"
#include "Node.h"

using namespace std;

class Graph:public Object{
    vector<Node*> nodes;
    vector<int>   parents;
public:
    Graph();
    ~Graph();
    
    void addRoot(Node*);
    void addNode(Node*, Node*);
    
    int getSize();
    Node* getNode(int);
};