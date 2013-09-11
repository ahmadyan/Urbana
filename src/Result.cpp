#include "Result.h"
#include <sstream>

string Result::toString(){
    stringstream ss ;
    ss << success <<  "  " << iterations << " " << time << endl;
    return ss.str();
}