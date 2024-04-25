#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#include <semaphore.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/wait.h>
#include <signal.h>
#include <time.h>
#define BUF_SIZE 4

typedef struct SharedMem {
    sem_t full;
    sem_t empty;
    sem_t mutex;
    sem_t wait;
    sem_t cnxn;
    int n;
    int min;
    int max;
    int buf[BUF_SIZE];
    bool finished;
} SharedMem;

SharedMem *shm = NULL;

void at_exit() {
    shm->finished=true;
    sem_destroy(&shm->full);
    sem_destroy(&shm->empty);
    sem_destroy(&shm->mutex);
    munmap(shm, sizeof(SharedMem));
    shm_unlink("/rand_prod_cons");
}

void handle_quit(int sig) {
    at_exit();    
    exit(0);
}

void perrorc(const char *msg)
{
    perror(msg);
    exit(1);
}

void init_semaphore(sem_t *sem, bool pshare, unsigned int value) {
    if(sem_init(sem, pshare, value) == -1) {
        perrorc("An error occurred while initializing the semaphore");
    }
}

void destroy_semaphore(sem_t *sem) {
    if(sem_destroy(sem) == -1) {
        perrorc("An error occurred while destroying the semaphore");
    }
}

void wait_semaphore(sem_t *sem) {
    if(sem_wait(sem) == -1) {
        perrorc("An error occurred while waiting for a semaphore");
    }
}

void post_semaphore(sem_t *sem) {
    if(sem_post(sem) == -1) {
        perrorc("An error occurred while posting to semaphore");
    }
}

void destroy_semaphores() {
    destroy_semaphore(&shm->full);
    destroy_semaphore(&shm->empty);
    destroy_semaphore(&shm->mutex);
}

void reinit_semaphores() {
	init_semaphore(&shm->full, true, 0);
	init_semaphore(&shm->empty, true, BUF_SIZE);
	init_semaphore(&shm->mutex, true, 1);
}

void init_semaphores() {
    reinit_semaphores();
    init_semaphore(&shm->wait, true, 0);
	init_semaphore(&shm->cnxn, true, 1);
}

int main() {
    struct sigaction action;
    action.sa_handler = handle_quit;
    action.sa_flags = 0;
    sigemptyset(&action.sa_mask);
    sigaction(SIGINT, action, NULL);
    sigaction(SIGQUIT, action, NULL);
    sigaction(SIGTERM, action, NULL);
    atexit(at_exit);

    int shmfd = shm_open("/rand_prod_cons", O_RDWR | O_CREAT | O_TRUNC, 0777);
    if(shmfd < 0)
        perrorc("An error occurred while creating shared memory");
    if(ftruncate(shmfd, sizeof(SharedMem)) == -1)
        perrorc("An error occurred when resizing shared memory");
    SharedMem *shm = mmap(NULL, sizeof(SharedMem), PROT_READ | PROT_WRITE, MAP_SHARED, shmfd, 0);
    if (shm == MAP_FAILED)
        perrorc("An error occurred during mmap");
    srand(time(NULL));
    
    
    init_semaphores();
    
    while(true) {
        printf("Waiting for new connection...\n");
        wait_semaphore(&shm->wait);
        printf("Reinitializing Shared Memory\n")
        destroy_semaphores();
        reinit_semaphores();
        shm->finished = false;

        int n;
        n = shm->n;

        post_semaphore(&shm->wait);
        printf("Entering main loop\n");
        int in = 0;
    	for(int i = 0; i < n; i++) {
            printf("Waiting for available space in buf\n");
            wait_semaphore(&shm->empty);
            printf("Waiting for mutex\n");
            wait_semaphore(&shm->mutex);
            int rand_num = rand() % (shm->max - shm->min) + shm->min;
            printf("Producing \'%d\'\n", rand_num);
            shm->buf[in] = rand_num;
            in = (in + 1) % BUF_SIZE;
            post_semaphore(&shm->full);
            if(i == n - 1) {
                printf("Finished sending all data\n");
    	        shm->finished = true;
            }
            printf("Unlocking Mutex\n");
            post_semaphore(&shm->mutex);
        }
        printf("Finished with connection\n");   
    }
    return 0;
}
