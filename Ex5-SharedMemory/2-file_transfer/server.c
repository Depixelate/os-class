#include <unistd.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdbool.h>

void perrorc(const char *msg) {
    perror(msg);
    exit(1);
}

#define BUF_SIZE 4096

int main() {
	key_t key = ftok("/tmp", 65);
	int id = shmget(key, BUF_SIZE, 0);
	if (id < 0) perrorc("An error occurred while getting the shared memory segment");
	char *buf = (char *)shmat(id, NULL, 0);
	if((int)buf == -1) perrorc("An error occurred while attaching to the shared memory segment");
	
	char *file_name = buf + 1;
	char *content = file_name + strlen(file_name) + 1;
	int *size_ptr = (int *)content;
	content = content + 4;
	int fd = open(file_name, O_RDONLY);
	if(fd == -1) {
		buf[0] = -1;
		perrorc("An error occurred while reading the file");
	}
	
	printf("File Content: \n");
	
	for(int i = 0; true; i++) {
		int res = read(fd, content + i, 1);
		if (res == 0) break;
		if (res == -1) {
			buf[0] = -1;
			perrorc("An error occurred while reading the file");	
		}
		putchar(buf[i]);
		*size_ptr += 1;
	}
	
	buf[0] = 1;
	close(fd);
	int result = shmdt(buf);
	if (result < 0) perrorc("An error occurred while detaching from the shared memory segment");
}
