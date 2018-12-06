#include "types.h" 
#include "user.h"

long double recurse(long double i){
 volatile double array[500] = {0.0};
 printf(1, "%d\n", array[0]); 
   if(i >= 1)
        return 1;
   else
        return recurse(i + 1);
}
int main(){
    //long double array[500] = {0.0};
    recurse(0);
    printf(1, "I didn't error\n"); 
    exit();
}
