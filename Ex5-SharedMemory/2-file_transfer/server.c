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
#include <errno.h>

void print_error(const char *msg) {
	fprintf(stderr, "%s", msg);
	exit(1);
}

void gen_error(char *restrict buf, const char *restrict msg) {
	sprintf(buf, "%s: %s", msg, strerror(errno));
}

void perrorc(const char *msg) {
    perror(msg);
    exit(1);
}

#define NAME_SIZE 100
#define FILE_SIZE 4096

typedef enum Status
{
	WAITING,
	SUCCESS,
	ERROR
} Status;

typedef struct SharedMem
{
	Status client_status;
	Status server_status;
	char filename[NAME_SIZE];
	int size;
	char file[FILE_SIZE];
} SharedMem;

int main() {
	key_t key = ftok("/tmp", 65);
	if (key < 0)
		perrorc("An error occurred while generating the key");
	int id = shmget(key, sizeof(SharedMem), 0);
	if (id < 0) perrorc("An error occurred while getting the shared memory segment");
	void *temp = shmat(id, NULL, 0);
	if (temp == (void *)-1)
		perrorc("An error occurred while attaching to shared memory");
	SharedMem *shm = (SharedMem *)temp;

	int fd = open(shm->filename, O_RDONLY);
	if(fd < 0) {
		shm->server_status = ERROR;
		gen_error(shm->file, "An error occurred while opening the file");
		//sprintf(shm->file, "An error occurred while opening the file: %s", strerror(errno));
		print_error(shm->file);
	}
	
	printf("File Content: \n");
	
	for(int i = 0; true; i++) {
		int res = read(fd, shm->file + i, 1);
		if (res == 0) break;
		if (res == -1) {
			shm->server_status = ERROR;
			gen_error(shm->file, "An error occurred while reading the file");
			print_error(shm->file);
		}
		putchar(shm->file[i]);
		shm->size += 1;
	}

	shm->server_status = SUCCESS;
	close(fd);
	int result = shmdt(shm);
	if (result < 0) perrorc("An error occurred while detaching from the shared memory segment");
}
