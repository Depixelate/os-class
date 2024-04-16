#include <unistd.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

void perrorc(const char *msg) {
    perror(msg);
    exit(1);
}

#define BUF_SIZE 4096

int main() {
	int id = shmget(IPC_PRIVATE, BUF_SIZE, 0777);
	if (id < 0) perrorc("An error occurred while creating the shared memory segment");
	int pid = fork();
	if (pid < 0) perrorc("An error occurred while forking the process");
	char *buf = (char *)shmat(id, NULL, 0);
	buf[0] = '\0';
	if((int)buf == -1) perrorc("An error occurred while attaching to the shared memory segment");
	
	if (pid != 0) {
		printf("Parent: Enter a name to convert to uppercase: ");
		fgets(buf+1, BUF_SIZE-1, stdin);
		(buf+1)[strlen(buf+1)-1] = '\0';
		buf[0] = '.';
	} else {
		while(strlen(buf) == 0)
			;
		char temp[BUF_SIZE];
		strcpy(temp, buf+1);
		for(int i = 0; temp[i] != '\0'; i++) {
			temp[i] = toupper(temp[i]);
		}
		printf("Child: Name in uppercase: %s\n", temp);
	}
	int result = shmdt(buf);
	if (result < 0) perrorc("An error occurred while detaching from the shared memory segment");
}

