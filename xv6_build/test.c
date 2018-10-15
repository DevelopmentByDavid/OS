#include "types.h"
#include "user.h"

//#include "defs.h"


int main() {
	
	int child_pid = fork();
	if (child_pid == 0) {
		printf(1, "I exit first!\n");
	} else {
	//	int returnVal = wait(0);

		int returnVal = waitpid(child_pid, 0, 0);
		if (returnVal > 0) {
			printf(1, "I exit second because I wait on first to exit!\n");
		} else {
			printf(1, "hmm\n");
		}
		
	}
	
	printf(1, "Hello World \n");
	exit(0);
}
