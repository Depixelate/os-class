#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include <stdlib.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <grp.h>
#include <pwd.h>
#include <dirent.h>

void print_format() {
    fprintf(stderr, "format: ls [-l] <path1> <path2> ...\n");
}

bool chk(int var, int mask) {
    return (var & mask) > 0;
}

void load_perms(int perm_bits, char buf[4]) {
    //printf("Some Permissions: %o\n", perm_bits);

    buf[0] = buf[1] = buf[2] = '-';
    buf[3] = '\0';
    if(chk(perm_bits,S_IROTH)) {
        buf[0] = 'r';
    }
    if(chk(perm_bits,S_IWOTH)) {
        buf[1] = 'w';
    }
    if(chk(perm_bits,S_IXOTH)) {
        buf[2] = 'x';
    }

}

void print_file(char *path, char *file_name, bool detailed) {
    if(!detailed) {
        printf("%s ", file_name);
        return;
    }
    struct stat fileinfo;

    if(stat(path, &fileinfo) < 0) {
        perror("The following error occurred while trying to get information about a file");
        print_format();
        exit(1);
    }

    char dir = S_ISDIR(fileinfo.st_mode) ? 'd' : '-';

    char user_perms[4];
    char group_perms[4];
    char other_perms[4];

    //printf("Permissions: %o\n", fileinfo.st_mode);

    load_perms(fileinfo.st_mode & S_IRWXU >> 6, user_perms);
    load_perms(fileinfo.st_mode & S_IRWXG >> 3, group_perms);
    load_perms(fileinfo.st_mode & S_IRWXO, other_perms);

    int num_hard_links = fileinfo.st_nlink;

    struct passwd *p;
    p = getpwuid(fileinfo.st_uid);
    if(p == NULL) {
        perror("The Following Error occurred while trying to get owner info about a file");
        print_format();
        exit(1);
    }

    char *owner_name = p->pw_name;

    struct group *g = getgrgid(fileinfo.st_gid);

    if(g == NULL) {
        perror("The Following Error occurred while trying to get group info about a file");
        print_format();
        exit(1);
    }

    char *group_name = g->gr_name;

    int size = fileinfo.st_size;

    char *last_modified = ctime(&fileinfo.st_mtime);
    last_modified[strlen(last_modified) - 1] = '\0';

    printf("%c%s%s%s\t%d\t%s\t%s\t%d\t%s\t%s\n", dir, user_perms, group_perms, other_perms, num_hard_links, owner_name, group_name, size, last_modified, file_name);
}

void print_arg_info(char *path, bool detailed, bool recursive, bool file) {
    char subdir_paths[100][4096] = {0};
    int num_subdirs = 0;

    struct stat fileinfo;
    if(stat(path, &fileinfo) < 0) {
        perror("The following error occurred while trying to get information about a file");
        print_format();
        exit(1);
    }
    if(!S_ISDIR(fileinfo.st_mode)) {
        if(!file) return;
        print_file(path, path, detailed);
        printf("\n");
    } else {
        if(file) return;
        printf("\n");
        printf("%s: \n", path);
        if(detailed) {
            printf("total %d\n", fileinfo.st_blocks);
        }
        DIR *dir = opendir(path);
        if(dir == NULL) {
            perror("The following error occurred while trying to open a directory");
            print_format();
            exit(1);
        }

        struct dirent *cur_item = NULL;
        while(true) {
            cur_item = readdir(dir);
            if(cur_item == NULL) break;
            char *item_name = cur_item->d_name;
            if(item_name[0] == '.'/*strcmp(item_name, ".") == 0 || strcmp(item_name, "..") == 0*/) continue;
            char real_path[4096] = "";
            strcat(real_path, path);
            if(path[strlen(path)-1] != '/') {
                const char *sep = "/";
                strcat(real_path, sep);       
            }
            
            strcat(real_path, item_name);
            print_file(real_path, item_name, detailed);

            if(recursive && cur_item->d_type == DT_DIR) {
                strcpy(subdir_paths[num_subdirs], real_path);
                num_subdirs++;
            }
        }
        printf("\n");

        if(recursive) {
            for(int i = 0; i < num_subdirs; i++) {
                print_arg_info(subdir_paths[i], detailed, recursive, true);
            }

            for(int i = 0; i < num_subdirs; i++) {
                print_arg_info(subdir_paths[i], detailed, recursive, false);
            }
        }
    }
    printf("\n");
}



int main(int argc, char** argv) {
    // check if options are valid
    int file_args = argc - 1;
    bool detailed = false;
    bool recursive = false;
    for(int i = 1; i < argc; i++) {
        if(argv[i][0] == '-') {
            file_args -= 1;
            const char *option_l = "-l";
            const char *option_R = "-R";
            if(strcmp(argv[i], option_l) == 0) {
                detailed = true;   
            } else if(strcmp(argv[i], option_R) == 0) {
                recursive = true;
            } else {
                fprintf(stderr, "Error! Invalid option!\n");
                print_format();
            }
        }
    }

    if(file_args == 0) {
        print_arg_info(".", detailed, recursive, false);
        return 0;
    }

    for(int i = 1; i < argc; i++) {
        if(argv[i][0] == '-') continue;
        print_arg_info(argv[i], detailed, recursive, true);
    }

    for(int i = 1; i < argc; i++) {
        if(argv[i][0] == '-') continue;
        print_arg_info(argv[i], detailed, recursive, false);
    }
}
