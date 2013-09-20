#include "utility.h"
#include <iostream>
#include <ctime>
#include <cstring>
#include <cstdlib>
#include <stdio.h>
#include <stdlib.h>
#include "config.h"
#include <sstream>
using namespace std;

double initial_time = clock(), final_time=0, total_time=0;
time_t t1=0, t2=0, t3=0;
    
    
// Generate a random number between 0 and 1
// return a uniform number in [0,1].
double unifRand(){
    return rand() / double(RAND_MAX);
}


// Generate a random number in a real interval.
// param a one end point of the interval
// param b the other end of the interval
// return a inform rand numberin [a,b].
double unifRand(double a, double b){
    return (b-a)*unifRand() + a;
}

// Generate a random integer between 1 and a given value.
// param n the largest value
// return a uniform random value in [1,...,n]
long unifRand(long n){
    if (n < 0) n = -n;
    if (n==0) return 0;
    /* There is a slight error in that this code can produce a return value of n+1
    **  return long(unifRand()*n) + 1;
    */
    //Fixed code
    long guard = (long) (unifRand() * n) +1;
    return (guard > n)? n : guard;
}

// Reset the random number generator with the system clock.
void seed(){
    srand(time(0));
}
    
void tick(){
    initial_time=clock();
    time(&t1);
}
    
void tock(){
    final_time = clock();
    total_time = (double)(final_time-initial_time) / (double) CLOCKS_PER_SEC ;
    time(&t2);
    t3=t2-t1;
    if(t3>1000) total_time=t3 ;
    cout << total_time << " seconds " << endl ;
}

template<class T>
string toString(T temp){
    stringstream ss;
    ss << temp;
    return ss.str();
}


void scanone(int argc, char *argv[], int i, int *varptr){
	if (i>=argc || sscanf(argv[i],"%i",varptr)!=1){
		fprintf(stderr, "Bad argument %s\n", i<argc ? argv[i] : argv[argc-1]);
		exit(-1);
	}
}

void scanoneu(int argc, char *argv[], int i, unsigned int *varptr){
	if (i>=argc || sscanf(argv[i],"%u",varptr)!=1){
		fprintf(stderr, "Bad argument %s\n", i<argc ? argv[i] : argv[argc-1]);
		exit(-1);
	}
}

void scanonell(int argc, char *argv[], int i, long long int *varptr){
	if (i>=argc || sscanf(argv[i],"%Li",varptr)!=1){
		fprintf(stderr, "Bad argument %s\n", i<argc ? argv[i] : argv[argc-1]);
		exit(-1);
	}
}

void parse_parameters(int argc,char *argv[], Configuration* config){
	int i;
	int temp;
    
    //The CNF stream is the input file, we store a pointer to input file in configuration class
    config->setCNFStream(stdin); //FILE* cnfStream = stdin;
	for (i=1;i < argc;i++){
		if (argv[i][0] != '-' && config->getCNFStream() == stdin){
            config->setParameter("param.inputfilename", argv[i]);
            config->setCNFStream(fopen(argv[i],"r"));
			if (config->getCNFStream() == NULL){
				fprintf(stderr, "Cannot open file named %s\n", argv[i]);
				exit(-1);
			}
			++i;
		}
		else if (strcmp(argv[i],"-flag")==0){
            config->setParameter("param.flag", "1");
		}
		else if (strcmp(argv[i],"-seed") == 0){
			scanone(argc,argv,++i,&temp);
            config->setParameter("param.seed", toString<int>(temp));
		}
		else
		{
            cout << "General parameters                                                     " << endl ;
            cout << "filename = CNF file, if not specified read stdin                       " << endl ;
			exit(-1);
		}
	}

    /* do post argument parsing here */
}

