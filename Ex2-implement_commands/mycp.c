#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>

void print_format() {
    fprintf(stderr, "Format: cp [-i] <source> <dest>\n");
}

int main(int argc, char *argv[]) {
    if(argc < 3) {
        fprintf(stderr, "Error! Not Enough Arguments!\n");
        print_format();
        return 1;
    }

    if(argc > 4) {
        fprintf(stderr, "Error! Too Many Arguments!\n");
        print_format();
        return 1;
    }

    bool interactive = false;
    int src_index = 1, dest_index = 2;

    if(argc == 4) {
        if(strcmp(argv[1], "-i") != 0 && strcmp(argv[2], "-i") != 0 && strcmp(argv[3], "-i") != 0) {
            fprintf(stderr, "Error! Invalid Format/Option!\n");
            print_format();
            return 1;
        }
        interactive = true;
        src_index++;
        dest_index++;
    }

    char *src_path = argv[src_index];
    char *dest_path = argv[dest_index];


    int src_handle = open(src_path, O_RDONLY);
    if(src_handle < 0) {
        perror("The following error occurred while trying to open the source file");
        print_format();
        return 1;
    }

    struct stat fileinfo;

    if(fstat(src_handle, &fileinfo) < 0) {
        perror("The following error occurred while trying to get information about the source file");
        print_format();
        return 1;
    }

    int size = fileinfo.st_size;

    char *buf = (char *)malloc(size);

    int bytes_read = 0;
    while(bytes_read < size) {
        int b = read(src_handle, buf + bytes_read, size - bytes_read);
        if(b < 0) {
            perror("The following error occurred while trying to read the file");
            print_format();
            close(src_handle);
            return 1;
        }
        bytes_read += b;
    }

    if(close(src_handle) < 0) {
        perror("The following error occurred while trying to close the source file");
        print_format();
        return 1;
    }

    if(interactive && access(dest_path, F_OK) == 0) {
        printf("Do you want to overwrite the already existing file(y/n): ");
        char answer;
        scanf("%c", &answer);
        if(answer != 'y' && answer != 'Y') {
            return 0;
        }
    }

    int dest_handle = open(dest_path, O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU | S_IRWXG | S_IRWXO);
    if(dest_handle < 0) {
        perror("The following error occurred while trying to create/open the destination file");
        print_format();
        return 1;
    }

    int bytes_written = 0;
    while(bytes_written < size) {
        int b = write(dest_handle, buf+bytes_written, size-bytes_written);
        if(b < 0) {
            perror("The following error occurred while writing to the destination file");
            print_format();
            close(dest_handle);
            return 1;
        }
        bytes_written += b;
    }

    if(close(dest_handle) < 0) {
        perror("The following error occurred while trying to close the destination file");
        print_format();
        return 1;
    }

    return 0;
}