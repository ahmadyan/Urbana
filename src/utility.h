
#pragma once
#include "config.h"

double unifRand();
double unifRand(double a, double b);
long unifRand(long n);
void seed();
void tick();
void tock();
void parse_parameters(int argc,char *argv[], Configuration* cfg);
template<class T> string toString(T temp);
void scanone(int argc, char *argv[], int i, int *varptr);
void scanonell(int argc, char *argv[], int i, long long int *varptr);
void scanoneu(int argc, char *argv[], int i, unsigned int *varptr);

