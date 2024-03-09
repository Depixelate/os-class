#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main() {
    printf("Parent Process starting, before fork!\n");
    int pid = fork();
    if(pid != 0) {
        printf("In Parent process after fork!\n");
        printf("Parent's current ID: %d\n", getpid());
        printf("Parent's parent ID: %d\n", getppid());
        printf("Parent waiting for child...\n");
        int wstatus;
        wait(&wstatus);
        printf("Waiting finished!, Did child exit normally: %s, Child exit status: %d\n", WIFEXITED(wstatus) ? "true" : "false", WEXITSTATUS(wstatus));
        printf("Parent exiting...\n");
    } else {
        printf("In Child process after fork!\n");
        printf("Child's current ID: %d\n", getpid());
        printf("Child's parent ID: %d\n", getppid());
        int n = 3;
        printf("Child sleeping for %d seconds...\n", n);
        sleep(n);
        printf("Child done sleeping!\n");
        printf("Child exiting...\n");
    }
    return 0;
}