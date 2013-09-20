#pragma once
#include <stdlib.h>
#include <algorithm>
#include <vector>
#include <stdio.h>
#include <queue>
#include "object.h"
#include <limits>
using namespace std;
namespace vptree{
    
    struct VPHeap{
        VPHeap( int index, double dist) :
        index(index), dist(dist) {}
        int index;
        double dist;
        bool operator<( const VPHeap& o ) const {
            return dist < o.dist;
            }
        };
            
        struct VPNode{
            int index;
            double threshold;
            VPNode* left;
            VPNode* right;
            VPNode() :index(0), threshold(25), left(0), right(0) {}
            
            ~VPNode() {
                delete left;
                delete right;
            }
        };
            
            class VPTree{
            public:
                VPNode* root;
                vector<Object*> _items;
                
                void add(Object* target){
                    root = add( root, target);
                    _items.push_back(target);
                }
                
                VPNode* newVPNode(){
                    VPNode* node = new VPNode;
                    node->index=int(_items.size());
                    node->left = NULL;
                    node->right = NULL;
                    return node;
                }
                
                VPNode* add(VPNode* node, Object* target){
                    if(node==NULL){
                        return newVPNode();
                    }else{
                        int distance=target->distance(_items[node->index]);
                        if(distance<=node->threshold){
                            node->left = add(node->left, target);
                        }else{
                            node->right = add(node->right, target);
                        }
                    }
                    return node;
                }
                
                void search( Object* target, int k, vector<Object*>* results, vector<double>* distances){
                    std::priority_queue<VPHeap> heap;
                    
                    double _tau = std::numeric_limits<double>::max();
                    search( root, target, k, heap, _tau );
                    
                    results->clear(); distances->clear();
                    
                    while( !heap.empty() ) {
                        results->push_back( _items[heap.top().index] );
                        distances->push_back( heap.top().dist );
                        heap.pop();
                    }
                    
                    std::reverse( results->begin(), results->end() );
                    std::reverse( distances->begin(), distances->end() );
                }
                
                void search(VPNode* node, Object* target, int k, std::priority_queue<VPHeap>& heap,  double& _tau){
                    if ( node == NULL ) return;
                    double dist = target->distance( _items[node->index]);
                    if ( dist < _tau ) {
                        if ( heap.size() == k ) heap.pop();
                        heap.push( VPHeap(node->index, dist) );
                        if ( heap.size() == k ) _tau = heap.top().dist;
                    }
                    
                    if ( node->left == NULL && node->right == NULL ) {
                        return;
                    }
                    
                    if (dist < node->threshold ) {
                        if ( dist - _tau <= node->threshold ) {
                            search( node->left, target, k, heap, _tau );
                        }
                        
                        if ( dist + _tau >= node->threshold ) {
                            search( node->right, target, k, heap, _tau );
                        }
                        
                    }else{
                        if ( dist + _tau >= node->threshold ) {
                            search( node->right, target, k, heap, _tau );
                        }
                        
                        if ( dist - _tau <= node->threshold ) {
                            search( node->left, target, k, heap, _tau );
                        }
                    }
                }
            };
            };
            
