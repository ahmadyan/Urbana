#pragma once
#include <string>
using namespace std;

class Result{
public:
    bool success;   
    int iterations;
    int states;
    double time;
    
    string toString();
};