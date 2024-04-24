#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#include <semaphore.h>
#include <stdbool.h>
#include <string.h>

#define BUF_SIZE 8

typedef struct SharedMem {
    sem_t full;
    sem_t empty;
    sem_t mutex;
    char buf[BUF_SIZE]
} SharedMem;

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

int main() {
    int shmfd = shm_open("/prod_cons", O_RDWR | O_CREAT | O_TRUNC, 0777);
    if(shmfd < 0)
        perrorc("An error occurred while creating shared memory");
    if(ftruncate(shmfd, sizeof(SharedMem)) == -1)
        perrorc("An error occurred when resizing shared memory");
    
    if(pid < 0)
        perrorc("An error occurred when forking the process");
    SharedMem *shm = mmap(NULL, sizeof(SharedMem), PROT_READ | PROT_WRITE, MAP_SHARED, shmfd, 0);
    if (shm == MAP_FAILED)
        perrorc("An error occurred during mmap");
    init_semaphore(&shm->full, true, 0);
    init_semaphore(&shm->empty, true, BUF_SIZE);
    init_semaphore(&shm->mutex, true, 1);
    int pid = fork();
    if(pid != 0) { // Producer/Parent
        int in = 0;
        char input[4096];
        printf("Enter some text: ");
        fgets(input, 4096, stdin);
        for(int i = 0; i < strlen(input); i++) {
            printf("Producer: Waiting for available space in buf\n");
            wait_semaphore(&shm->empty);
            printf("Producer: Waiting for mutex\n");
            wait_semaphore(&shm->mutex);
            printf("Producer: Producing %c\n", input[i]);
            shm->buf[in] = input[i];
            in = (in + 1) % BUF_SIZE;
            post_semaphore(&shm->full);
            printf("Producer: Unlocking Mutex\n");
            post_semaphore(&shm->mutex);
        }
        
        
    } else { // Consumer/Child
        int out = 0;
        while(true) {
            printf("Producer: Waiting for available space in buf\n");
            wait_semaphore(&shm->empty);
            printf("Producer: Waiting for mutex\n");
            wait_semaphore(&shm->mutex);
            printf("Producer: Producing %c\n", input[i]);
            shm->buf[in] = input[i];
            in = (in + 1) % BUF_SIZE;
            post_semaphore(&shm->full);
            printf("Producer: Unlocking Mutex\n");
            post_semaphore(&shm->mutex);
        }
        destroy_semaphore(&shm->full);
        destroy_semaphore(&shm->empty);
        destroy_semaphore(&shm->mutex);
    }
    munmap(shm, sizeof(SharedMem));
    shm_unlink("/prod_cons");
    return 0;
}
