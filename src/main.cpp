//Released under CRAPL license :) see http://matt.might.net/articles/crapl/
//Some part of this code is borrowed from WalkSAT algorithm v50 http://www.cs.rochester.edu/u/kautz/walksat/

#include <iostream>
#include <stdio.h>
#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics/stats.hpp>
#include <boost/accumulators/statistics/variance.hpp>
#include <boost/accumulators/statistics/count.hpp>
#include <boost/accumulators/statistics/mean.hpp>
#include <boost/accumulators/statistics/moment.hpp>
#include "config.h"
#include "utility.h"
#include "SAT.h"
#include "Result.h"


#include "vptree.h"
using namespace std;
using namespace  boost::accumulators;

int main(int argc,char *argv[]){
    srand((int)time(0));
    accumulator_set<double, stats<tag::variance> > acc;
    accumulator_set<double, stats<tag::variance> > acc2;
    srand((int)time(0));
    Configuration* config = new Configuration("config");
    parse_parameters(argc, argv, config);
    vector<Result*> results;
    int run=100; config->getParameter("param.run", &run);
    SAT* golden = new SAT(config);
    golden->init();
    golden->solve();
    results.push_back(golden->getResult());
    if(golden->getResult()->success) acc(golden->getResult()->iterations);
    if(golden->getResult()->success) acc2(golden->getResult()->states);
    
    for(int i=0;i<run-1;i++){
        SAT* sat = new SAT(golden);
        sat->init();
        sat->solve();
        results.push_back(sat->getResult());
        if(sat->getResult()->success){
            acc(sat->getResult()->iterations);
            acc2(sat->getResult()->states);
        }
    }
    
    for(int i=0;i<results.size();i++){
        cout << i << ": " << results[i]->toString() ;
    }
        
    cout <<"Count : " << boost::accumulators::count(acc) << "/" << results.size() << endl;
    cout <<"Mean  :  "<<mean(acc)<< "    " << mean(acc2) << endl;
    //cout<<"moment : "<<moment<2>(acc)<<endl;
    cout<<"deviation : "<< sqrt(variance(acc)) << "     " << sqrt(variance(acc2)) <<endl;
    
    
    return 0;
}