#include "Graph.h"

Graph::Graph(){
    
}

Graph::~Graph(){
    
}

void Graph::addRoot(Node* root){
    nodes.push_back(root);
    root->setNodeNumber(-1);
}

void Graph::addNode(Node* node, Node* parent){
    nodes.push_back(node);
    node->setNodeNumber(getSize()-1);
    parents.push_back(parent->getNodeNumber());
}

int Graph::getSize(){
    return nodes.size();
}

Node* Graph::getNode(int i){
    return nodes[i];
}