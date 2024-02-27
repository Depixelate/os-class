#include <string.h>

void print_format() {
    fprintf(stderr, "format: ls [-l] <path1> <path2> ...\n");
}

int main(int argc, char** argv) {
    // check if options are valid
    bool detailed = false;
    for(int i = 1; i < argc; i++) {
        if(argv[i][0] == '-') {
            if(strcmp(argv[i], '-l') != 0) {
                fprintf(stderr, "Error! Invalid option!\n")
                print_format();
            }
            detailed = true;
        }
    }

    
}
