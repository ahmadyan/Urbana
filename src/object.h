#pragma once
#include <boost/intrusive/avl_set.hpp>
#include <iostream>
#include <string>
#include <sstream>
#include <cmath>
using namespace std;

class Object : public boost::intrusive::avl_set_base_hook<boost::intrusive::optimize_size<true> >{
    
public:
    int id;
    bool nullObject;    //indicates whether this object is a null object, so it is not initialized and doesn't have any data/id (either used for its operators or returned when object is not found)
    bool dbFlag;        //indicates whether this object has already been stored.
    static int objectCount ;
    int* data;
    int  size;
    
    boost::intrusive::avl_set_member_hook<> member_hook_;
    
    Object();
    Object(int n);
    ~Object();
    
    static int generateObjectID();
    int getID();
    
    void setData(int*);
    int* getData();
    int get(int);
    void set(int, int);
    void setSize(int s);
    int getSize();
    void reset();
    void randomize();
    void randomize(double);
    void randomize(int*);
    string toString();
    bool null();
    void setDBFlag(bool);
    bool getDBFlag();
    double distance(Object*);
    
    //Comparison operators.
    //Comparing object with another object
    friend bool operator < (const Object &a, const Object &b){
        for(int i=0;i<a.size;i++){
            if(a.data[i]!=b.data[i]){
                if( a.data[i]==0) return true;
                else return false;
            }
        }
        return true;
    }
    friend bool operator > (const Object &a, const Object &b){
        for(int i=0;i<a.size;i++){
            if(a.data[i]!=b.data[i]){
                if( a.data[i]==1) return true;
                else return false;
            }
        }
        return true;
    }
    friend bool operator == (const Object &a, const Object &b){
        for(int i=0;i<a.size;i++){
            if(a.data[i]!=b.data[i]){
                return false;
            }
        }
        return true;
    }
    bool operator () (const Object &a, const Object &b) const {
        if (a>b) return false;
        else return true;
    }
    
    //Comparing objects, using only data
    friend bool operator < (const int* a, const Object &b){
        for(int i=0;i<b.size;i++){
            if(a[i]!=b.data[i]){
                if( a[i]==0) return true;
                else return false;
            }
        }
        return true;
    }
    
    friend bool operator < (const Object &a, const int* b){
        for(int i=0;i<a.size;i++){
            if(a.data[i]!=b[i]){
                if( a.data[i]==0) return true;
                else return false;
            }
        }
        return true;
    }
    
    friend bool operator > (const int* a, const Object &b){
        for(int i=0;i<b.size;i++){
            if(a[i]!=b.data[i]){
                if( a[i]==1) return true;
                else return false;
            }
        }
        return true;
    }
    
    friend bool operator > (const Object &a, const int* b){
        for(int i=0;i<a.size;i++){
            if(a.data[i]!=b[i]){
                if( a.data[i]==1) return true;
                else return false;
            }
        }
        return true;
    }
    
    friend bool operator == (int* d, const Object &a){
        for(int i=0;i<a.size;i++){
            if( a.data[i]!=d[i])
                return false;
        }
        return true;
    }
    
    friend bool operator == (const Object &a, int* d){
        for(int i=0;i<a.size;i++){
            if( a.data[i]!=d[i])
                return false;
        }
        return true;
    }
    
    bool operator () (const Object &a, const int* d) const {
        //The () operator for find() is very similar to strcmp.
        if (a>d) return false;
        else return true;
    }
    
    bool operator () (int* d, const Object &a) const{
        if(d>a) return false;
        else return true;
    }
};
    
    
