//Released under CRAPL license :) see http://matt.might.net/articles/crapl/
//Some part of this code is borrowed from WalkSAT algorithm v50 http://www.cs.rochester.edu/u/kautz/walksat/
#include <iostream>
#include <stdio.h>
#include "config.h"
#include "utility.h"
#include "SAT.h"

#include "Output.h"
using namespace std;

int main(int argc,char *argv[]){
    Configuration* config = new Configuration("config");
    parse_parameters(argc, argv, config);
    SAT* sat = new SAT(config);
    sat->solve();    
    return 0;
}


