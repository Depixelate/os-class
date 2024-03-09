#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
int factorial(int n) {
	if(n == 0) return 1;
	return n * factorial(n-1);
}

int main(int argc, char *argv[]) {

	int d = fork();
	//printf("first arg - %s\n", argv[0]);
	if(d == 0) {
		// child process
		int val = atoi(argv[1]);
		printf("In Child - factorial of %d = %d\n", val, factorial(val));		
	} else {
		// parent process
		int val = atoi(argv[2]);
		printf("In Parent, Series of %d: \n", val);
		for(int i = 1; i <= val; i++) {
			printf("%d ", i);
		}
		printf("\n");
		wait(NULL);
	}
	//printf("D: %d\n", d);
	//printf("D-PID: %d-%d\n", d, getpid());
	//printf("D-PPID: %d-%d\n", d, getppid());
	return 0;
}
