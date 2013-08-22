#include "Graph.h"
#include <boost/graph/graphviz.hpp>
GraphFacade::GraphFacade(){
    Graph g;    
}

GraphFacade::~GraphFacade(){
}

// Adds the new node
void GraphFacade::add(Node* v){
    if(!v->getGraphFlag()){ //If this node has not been already stored in the grpah
        nodes.push_back(v);
        v->setNodeNumber(getSize()-1);
        Vertex_t vertex = boost::add_vertex(v, g);
        v->setVertex(vertex);
    }
}

//Adds node v to the graph,
//We assume that node u has already been added to the grpah (The grpahFlag is set and the u->vertex is not null).
void GraphFacade::add(Node* v, Node* u){
    add(v);
    add(u);
    // Add edge between u and v
    Edge_t edge;
    bool edgeExists;
    boost::tie(edge, edgeExists) = boost::edge(v->getVertex() , u->getVertex(), g); // check if edge allready exist 
    if(!edgeExists)
        boost::add_edge(v->getVertex()  , u->getVertex(), g);
}

int GraphFacade::getSize(){
    return nodes.size();
}

Node* GraphFacade::getNode(int i){
    return nodes[i];
}

void GraphFacade::toString(){
    cout << "\n-- graphviz output START --" << endl;
    boost::write_graphviz(cout, g);
    cout << "\n-- graphviz output END --" << endl;
}