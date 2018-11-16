#include "types.h" 
#include "user.h"

uint recurse(uint i){
    if(i >= 200)
        return 1;
    else
        return recurse(i + 1);
}
int main(){
    recurse(1);
    exit();
}
