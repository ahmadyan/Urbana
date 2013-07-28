#pragma once
#include <iostream>
#include <string>
#include <sstream>
#include <cmath>
using namespace std;

class Object{
public:
    int id;
    static int objectCount ;
    int* data;
    int  size;
    
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
    string toString();
};
