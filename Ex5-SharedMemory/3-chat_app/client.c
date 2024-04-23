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
int *buf = NULL;
char *msgs = NULL;
int shmid = 0;
int server_pid = -1;

void on_exit_func(int sig) {
	signal(SIGINT, on_exit_func);
	shmdt(buf);
	shmctl(shmid, IPC_RMID, NULL);
	exit(0);
}

void on_recieve_msg(int sig) {
	signal(SIGIO, on_recieve_msg);
	char msg[BUF_SIZE];
	for(int i = 0; true; i++) {
		msg[i] = msgs[pos];
		int new_pos = (pos + 1) % BUF_SIZE;
		if(msgs[pos] == '\n') {
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
	signal(SIGINT, on_exit_func);
	signal(SIGIO, on_recieve_msg);

	key_t key = ftok("/tmp", 65);

	shmid = shmget(key, BUF_SIZE, IPC_CREAT | 0777);
	buf = (int *)shmat(shmid, NULL, 0);
	buf[0] = getpid();
	buf[1] = 0;
	msgs = (char *)(&buf[2]);

	while(buf[1] == 0)
		;

	server_pid = buf[1];

	while(true) {
		while(waiting)
		;
		printf("Do you want to quit(y/n): ");
		char c = getchar();
		getchar();
		if (c == 'y')
		{
			kill(server_pid, SIGINT);
			on_exit_func(0);
		}
		printf("Enter a message: ");
		while(true) {
			char c = getchar();
			msgs[pos] = c;
			pos = (pos + 1) % BUF_SIZE;
			if(c == '\n') break;
		}
		kill(server_pid, SIGIO);
		waiting = true;
	}
}
