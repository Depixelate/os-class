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

typedef enum Status
{
	WAITING,
	SUCCESS,
} Status;

typedef struct SharedMem
{
	Status server_status;
	int client_pid;
	int server_pid;
	char msg[BUF_SIZE];
} SharedMem;

bool waiting = true;
int shmid = 0;
SharedMem *shm = NULL;

void on_exit_func(int sig)
{
	if (signal(SIGUSR1, on_exit_func) == SIG_ERR)
		perrorc("An error occurred when setting a signal");
	int result = shmdt(shm);
	if (result < 0)
		perrorc("An error occurred while detaching from the shared memory segment");
	exit(0);
}

void on_recieve_msg(int sig)
{
	if (signal(SIGUSR2, on_recieve_msg) == SIG_ERR)
		perrorc("An error occurred when setting a signal");
	printf("Message from client: %s", shm->msg);
	waiting = false;
}

int main()
{
	if (signal(SIGUSR1, on_exit_func) == SIG_ERR)
		perrorc("An error occurred when setting a signal");
	if (signal(SIGUSR2, on_recieve_msg) == SIG_ERR)
		perrorc("An error occurred when setting a signal");

	key_t key = ftok("/tmp", 65);
	if (key < 0)
		perrorc("An error occurred while generating the key");

	shmid = shmget(key, BUF_SIZE, IPC_CREAT | 0777);
	if (shmid < 0)
		perrorc("An error occurred while getting the shared memory segment");
	void *temp = shmat(shmid, NULL, 0);
	if (temp == (void *)-1)
		perrorc("An error occurred while attaching to shared memory");
	shm = (SharedMem *)temp;
	shm->server_pid = getpid();
	shm->server_status = SUCCESS;

	while (true)
	{
		while (waiting)
			;
		printf("Do you want to quit(y/n): ");
		char buf[BUF_SIZE];
		fgets(buf, BUF_SIZE, stdin);

		if (buf[0] == 'y')
		{
			if (kill(shm->client_pid, SIGUSR1) < 0)
				perrorc("An error occurred while trying to send a signal");
			on_exit_func(0);
		}
		printf("Enter a message: ");
		fgets(shm->msg, BUF_SIZE, stdin);
		if (kill(shm->client_pid, SIGUSR2) < 0)
			perrorc("An error occurred while trying to send a signal");
		waiting = true;
	}
}
