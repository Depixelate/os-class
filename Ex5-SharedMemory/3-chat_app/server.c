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

void perrorc(const char *msg)
{
	perror(msg);
	exit(1);
}

bool waiting = true;
int pos = 0;
int *buf = NULL;
char *msgs = NULL;
int shmid = 0;
int client_pid = -1;

void new_oe(int sig)
{
	signal(SIGINT, new_oe);
	shmdt(buf);
	//shmctl(shmid, IPC_RMID, NULL);
	exit(0);
}

void on_recieve_msg(int sig)
{
	signal(SIGIO, on_recieve_msg);
	char msg[BUF_SIZE];
	for (int i = 0; true; i++)
	{
		msg[i] = msgs[pos];
		int new_pos = (pos + 1) % BUF_SIZE;
		if (msgs[pos] == '\n')
		{
			pos = new_pos;
			msg[i + 1] = '\0';
			break;
		}
		pos = new_pos;
	}

	printf("Message from server: %s", msg);
	waiting = false;
}

int main()
{
	signal(SIGINT, new_oe);
	signal(SIGIO, on_recieve_msg);

	key_t key = ftok("/tmp", 65);

	shmid = shmget(key, BUF_SIZE, 0);
	buf = (int *)shmat(shmid, NULL, 0);
	client_pid = buf[0];
	buf[1] = getpid();
	msgs = (char *)(&buf[2]);
	while (true)
	{
		while (waiting)
			;
		printf("Do you want to quit(y/n): ");
		char c = getchar();
		getchar();
		if (c == 'y')
		{
			kill(client_pid, SIGINT);
			new_oe(0);
		}
		printf("Enter a message: ");
		while (true)
		{
			char c = getchar();
			msgs[pos] = c;
			pos = (pos + 1) % BUF_SIZE;
			if (c == '\n')
				break;
		}
		kill(client_pid, SIGIO);
		waiting = true;
	}
}
