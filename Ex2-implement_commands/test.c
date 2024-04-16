#include <stdio.h>
#include <unistd.h>
#define _GNU_SOURCE 
#include <stdlib.h>

int main() {
    printf("%s", canonicalize_file_name("f3.txt"));
}
