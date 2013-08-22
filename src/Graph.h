#pragma once
#include <vector>
#include <boost/graph/adjacency_list.hpp>
#include "object.h"
#include "Node.h"
#include "Edge.h"

using namespace std;
using namespace boost;

typedef boost::adjacency_list<boost::vecS,boost::vecS,boost::undirectedS, Node*> Graph;
typedef graph_traits<Graph>::vertex_descriptor Vertex_t;
typedef graph_traits<Graph>::edge_descriptor Edge_t;

// The underlying data stuctore for our algorithm is boost::graph,
// This class works as an interface between boost::graph and our algorithm by providing methods for
// adding nodes and iterators
class GraphFacade:public Object{
    Graph g;                    
    vector<Node*> nodes;        //for direct access to the nodes
public:
    GraphFacade();
    ~GraphFacade();
    
    void add(Node*);            //usually used for adding the root
    void add(Node*, Node*);     //Use this one in typical situations, add two nodes and the makes the edges between them.
    
    int getSize();
    Node* getNode(int);
    void toString();
};