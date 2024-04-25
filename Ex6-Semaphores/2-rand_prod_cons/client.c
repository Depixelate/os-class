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
    sem_t server_wait;
    sem_t client_wait;
    sem_t cnxn;
    int n;
    int min;
    int max;
    int buf[BUF_SIZE];
    bool finished;
} SharedMem;

void perrorc(const char *msg)
{
    perror(msg);
    exit(1);
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

int main() {
    int shmfd = shm_open("/rand_prod_cons", O_RDWR, 0777);
    if(shmfd < 0)
        perrorc("An error occurred while opening shared memory");
    SharedMem *shm = mmap(NULL, sizeof(SharedMem), PROT_READ | PROT_WRITE, MAP_SHARED, shmfd, 0);
    if (shm == MAP_FAILED)
        perrorc("An error occurred during mmap");
    
    int n, min, max;
    printf("Enter n: ");
    scanf("%d", &n);
    printf("Enter min: ");
    scanf("%d", &min);
    printf("Enter max: ");
    scanf("%d", &max);
    
    printf("Waiting to connect to server...\n");
    wait_semaphore(&shm->cnxn);
    printf("Connected!\n");
    shm->min = min;
    shm->max = max;
    shm->n = n;
    printf("Waiting for Server to reinitialize\n");
    post_semaphore(&shm->client_wait);
    wait_semaphore(&shm->server_wait);

    printf("Entering main loop..\n");
    int out = 0;            
    while(true) {
        printf("Waiting for available items\n");
        wait_semaphore(&shm->full);
        printf("Waiting for mutex\n");
        wait_semaphore(&shm->mutex);
        int d = shm->buf[out];
        printf("Consuming \'%d\'\n", d);
        out = (out + 1) % BUF_SIZE;                            
        post_semaphore(&shm->empty);
        int val;
        if(sem_getvalue(&shm->full, &val) == -1) perrorc("An error occurred while trying to get semaphore value");
        if(val <= 0 && shm->finished) break;
        printf("Unlocking Mutex\n");
        post_semaphore(&shm->mutex);            
    }
    printf("Finished\n");
    printf("Ending connection...\n");
    post_semaphore(&shm->cnxn);
    printf("Deinitializing shared memory...\n");
    munmap(shm, sizeof(SharedMem));
    close(shmfd);
    printf("Done\n");   
    return 0;
}
