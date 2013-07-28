#include "object.h"

int Object::objectCount = 0 ;

Object::Object(){
    id = generateObjectID() ;
}

Object::Object(int n){
    id = generateObjectID() ;
    size=n;
    data=new int[n];
}

Object::~Object(){
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

void Object::randomize(){
    for(int i=0;i<size;i++){
        data[i] = rand()%2;
    }
}

string Object::toString(){
    stringstream ss ;
    for(int i=0;i<size;i++){
        ss << data[i] ;
    }
    return ss.str();
}
