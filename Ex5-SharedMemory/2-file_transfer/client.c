#include <unistd.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdlib.h>

void perrorc(const char *msg) {
    perror(msg);
    exit(1);
}

#define BUF_SIZE 4096

int main() {
	key_t key = ftok("/tmp", 65);
	int id = shmget(key, BUF_SIZE, IPC_CREAT | 0777);
	if (id < 0) perrorc("An error occurred while creating the shared memory segment");
	char *buf = (char *)shmat(id, NULL, 0);
	if((int)buf == -1) perrorc("An error occurred while attaching to the shared memory segment");
	buf[0] = 0;
	printf("Enter file path: ");
	char *file_name = buf + 1;
	fgets(file_name, BUF_SIZE - 1, stdin);
	file_name[strlen(file_name)-1] = '\0';
	char *content = file_name + strlen(file_name) + 1;
	while(buf[0] == 0)
		;
	if (buf[0] == -1) perrorc("An error occurred in the server");
	int *size_ptr = (int *)content;
	int size = *size_ptr;
	content = content + 4;
	int fd = open("new.txt", O_CREAT | O_WRONLY, S_IRWXU | S_IRWXG | S_IRWXO);
	printf("File Content: \n");
	
	for(int i = 0; i < size; i++) {
		write(fd, content + i, 1);
		putchar(content[i]);
	}
	
	buf[0] = 0;
	
	close(fd);
	int result = shmdt(buf);
	if (result < 0) perrorc("An error occurred while detaching from the shared memory segment");
}
