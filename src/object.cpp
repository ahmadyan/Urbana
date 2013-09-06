#include "object.h"

int Object::objectCount = 0 ;

Object::Object(){
    dbFlag=false;
    nullObject=true;
    //id = generateObjectID() ;
}

Object::Object(int n){
    dbFlag=false;
    if(n<=0){
        nullObject=true;
    }else{
        nullObject=false;
        id = generateObjectID() ;
        size=n;
        data=new int[n];
    }
}

Object::~Object(){
    if(!nullObject)
        delete data;
}
    
int Object::generateObjectID(){
    return objectCount++;
}
    
int Object::getID(){
    return id;
}

void Object::setData(int* x){
    for(int i=0;i<size;i++){
        data[i] = x[i];
    }
}

int* Object::getData(){
    return data;
}

int Object::get(int i){
    return data[i];
}

void Object::set(int i, int v){
    data[i]=v;
}

void Object::setSize(int s){
    size = s;
}
int Object::getSize(){
    return size;
}

void Object::reset(){
    for(int i=0;i<size;i++)
        data[i]=0;
}

//assigns a random bit-vector to data
void Object::randomize(){
    for(int i=0;i<size;i++){
        data[i] = rand()%2;
    }
}

//assigns a biased random bit-vector to data, the probability of each bit being 1 is p.
void Object::randomize(double p){
    if (p<0 || p>1)
        cout << "error in random number generation with bias" << endl ;
    for(int i=0;i<size;i++){
        double x = (double)rand()/INT_MAX;
        if(x<p)
            data[i]=1;
        else
            data[i]=0;
    }
}

void Object::randomize(int* d){
    delete data;
    data=d;
}

string Object::toString(){
    stringstream ss ;
    for(int i=0;i<size;i++){
        ss << data[i] ;
    }
    return ss.str();
}

bool Object::null(){
    return nullObject;
}

void Object::setDBFlag(bool d){
    dbFlag=d;
}

bool Object::getDBFlag(){
    return dbFlag;
}

//Computes the hamming distance between this and object v
//Basically we can use any other distance function here, as long as they are metric.
double Object::distance(Object* v){
    double distance=0;
    for(int i=0;i<v->getSize(); i++){
        if(v->get(i)!=data[i]){
            distance++;
        }
    }
    return distance;
}
