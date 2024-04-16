#include <unistd.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdlib.h>

#define BUF_SIZE 4096

void perrorc(const char *msg) {
    perror(msg);
    exit(1);
}

bool waiting = false;
int pos = 0;
char *buf = NULL;
char *msgs = NULL;
int shmid = 0;
int server_pid = -1;

void on_exit(int signal) {
	signal(SIGINT, on_exit);
	shmdt(buf);
	shmctl(shmid, IPC_RMID, NULL);
	exit(0);
}

void on_recieve_msg(int signal) {
	signal(SIGINFO, on_recieve_msg);
	char msg[BUF_SIZE];
	for(int i = 0; true; i++) {
		msg[i] = buf[pos];
		int new_pos = (pos + 1) % BUF_SIZE;
		if(buf[pos] == '\n') {
			pos = new_pos;
			msg[i+1] = '\0';
			break;
		}
		pos = new_pos;
	}
	
	printf("Message from server: %s", msg);
	waiting = false;	
}





int main() {
	signal(SIGINT, on_exit);
	signal(SIGINFO, on_recieve_msg);
	
	
	
	while(true) {
		while(waiting)
		;
		printf("Do you want to quit(y/n): ");
		char c = getchar();
		if (c == 'y') {
			kill(server_pid, SIGINT);
			on_exit();
		}
		printf("Enter a message: ");
		while(true) {
			char c = getchar();
			buf[pos] = c;
			pos = (pos + 1) % BUF_SIZE;
			if(c == '\n') break;
		}
		kill(server_pid, SIGINFO);
		waiting = true;
	}
}
