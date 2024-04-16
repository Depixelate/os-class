#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
/*
any argument starting with -n is an option
first non-option text is pattern, every one after that is files
multiple patterns separated by \n
if no file given, read from standard input
If multiple files given, display file name before line
highlight matching part in red, maybe put file name in pink
accepts options -c, -v, -n
treats pattern as text to match, not regexp.
if -c with multiple files, print count for each file.
*/

/*
stands for "perror convenience"
*/
#define LEN(X) (sizeof(X) / sizeof(X[0]))

const char *options[] = {
    "-c", 
    "-v", 
    "-n"
};

char emsg[4096] = "";

const char *descriptions[] = {
    "print only a count of selected lines per FILE",
    "select non-matching lines",
    "print line number with output lines"
};

void print_format() {
    fprintf(stderr, "Format: grep [OPTIONS] [PATTERNS] [FILES]\n");
    fprintf(stderr, "Options: \n\n");
    for(int i = 0; i < LEN(options); i++) {
        fprintf(stderr, "%s\t%s\n", options[i], descriptions[i]);
    }
    fprintf(stderr, "\n");
}

void errorc(const char *msg) {
    fprintf(stderr, msg);
    print_format();
    exit(1);
}

void perrorc(const char *msg) {
    perror(msg);
    print_format();
    exit(1);
}
/*
This stands for string interpolate
basically a convenience wrapper
around sprintf that returns the string 
created.
*/

char *strint(char buf[], const char *format, ...) {
    va_list args;
    va_start(args, format);
    vsprintf(buf, format, args);
    va_end(args);
    return buf;
}

int replace_char(char buf[], char find, char replace) {
    int num_replacements = 0;
    int len = strlen(buf);
    for(int i = 0; i < len; i++) {
        if(buf[i] == find) {
            buf[i] = replace;
            num_replacements += 1;
        }
    }
    return num_replacements;
}


/* bool to return if EOF has been reached or not*/
bool readline(char buf[], int file, const char *file_name) {
    int len = 0; /* The reason for this is to avoid
    incrementing at highlight_range_end of loop, rather than 
    beginning. The reason for this is that in the 
    case EOF is reached, read will not add any 
    characters to */
    int chars_read = 0;
    do { 
        chars_read = read(file, buf + len, 1);
        if(chars_read == -1) {
            perrorc(strint(emsg, "An error occurred while reading from %s", file_name));
        }
        len+=chars_read; // The reason for using 
        // chars_read, rather than one, is in the 
        // case EOF is reached, where no extra char 
        // will be read into the buffer, so 
        // incrementing by 1 here would make the 
        // length too long.

    } while(chars_read != 0 && buf[len-1] != '\n');
    buf[buf[len-1] == '\n' ? len - 1 : len] = '\0';

    return chars_read == 0;
}

typedef struct RangeIndex {
    int index;
    bool start;
} RangeIndex;

int comp_range_index(const void *vr1, const void *vr2) {
    RangeIndex *r1 = (RangeIndex *)vr1;
    RangeIndex *r2 = (RangeIndex *)vr2;
    if(r1->index < r2->index) return -1;
    if(r1->index > r2->index) return 1;
    // if(r1.start && r2.end) return -1;
    // if(r1.end && r2.start) return 1;
    return 0;
}

int merge_highlight_ranges(int start[], int end[], int num_matches) {
    RangeIndex indices[4096] = {0};
    for(int i = 0; i < num_matches; i++) {
        indices[2*i].index = start[i];
        indices[2*i].start = true;
        indices[2*i + 1].index = end[i];
        indices[2*i + 1].start = false;
    }

    qsort(indices, 2 * num_matches, sizeof(RangeIndex), comp_range_index);

    int num_ranges = 0;
    int num_start = 0;
    for(int i = 0; i < 2 * num_matches; i++) {
        if(indices[i].start) {
            if(num_start == 0) {
                start[num_ranges] = indices[i].index;
            }
            num_start += 1;
        } else {
            num_start--;
            if(num_start == 0) {
                end[num_ranges] = indices[i].index;
                num_ranges++;
            }
        }
    }

    return num_ranges;
}

void print_highlighted_line(const char *line, int line_number, int start[], int end[], int num_ranges, int number_lines) {
    if(number_lines) {
        printf("%d:", line_number);
    }

    char buf[4096];
    int prev_end = 0;
    for(int i = 0; i < num_ranges; i++) {
        int len1 = start[i] - prev_end;
        memcpy(buf, line + prev_end, len1);
        buf[len1]='\0';
        printf("%s",buf);
        int len2 = end[i] - start[i];
        memcpy(buf, line + start[i], len2);
        buf[len2] = '\0';
        printf("\033[1;31m%s\033[0m", buf);
        prev_end = end[i];
    }
    printf("%s\n", line + prev_end);
}

void process_file(const char *path, const char *patterns, int num_patterns, bool count, bool invert, bool number_lines) {
    char line[4096];
    int file;
    if(strcmp(path, "-") == 0) {
        file = 0; // file handle for stdin
    } else {
        file = open(path, O_RDONLY);
        if(file == -1) {
            perrorc(strint(emsg, "Error opening file %s", path));
        }
    }

    int num_lines_matched = 0;

    int line_number = 0;

    while(!readline(line, file, path)) {
        line_number += 1;
        int highlight_range_start[4096];
        int highlight_range_end[4096];
        int num_matches = 0;
        const char *pattern = patterns;
        for(int i = 0; i < num_patterns; i++) {
            int pos = 0;
            const char *temp;
            while((temp=strstr(line + pos, pattern)) != NULL) {
                num_matches += 1;
                if(count || invert) goto outerLoop;
                pos = temp - line;
                highlight_range_start[num_matches-1] = pos;
                highlight_range_end[num_matches-1] = pos + strlen(pattern);
                pos = highlight_range_end[num_matches-1];
            }
            pattern += strlen(pattern) + 1;
        }
        outerLoop:

        if((!invert && num_matches == 0) || (invert && num_matches != 0)) continue;
        num_lines_matched += 1;
        if(count) continue;
        int num_ranges = merge_highlight_ranges(highlight_range_start, highlight_range_end, num_matches);
        printf("%s:", path);
        print_highlighted_line(line, line_number, highlight_range_start, highlight_range_end, num_ranges, number_lines);
    }
    if(count) {
        printf("%s:%d\n", path, num_lines_matched);
    }

    if(close(file) == -1) {
        perrorc(strint(emsg, "Error closing file %s", path));
    }
}


int main(int argc, char *argv[]) {

    bool count = false, invert = false, number_lines = false;
    for(int i = 1; i < argc; i++) {
        if(argv[i][0] == '-' && strlen(argv[i]) != 1) {
            if(strcmp(argv[i], "-c") == 0) {
                count = true;
            } else if(strcmp(argv[i], "-v") == 0) {
                invert = true;
            } else if(strcmp(argv[i], "-n") == 0){
                number_lines = true;
            } else {
                errorc(strint(emsg, "Error! %s is not a valid option!\n", argv[i]));
            }
        }
    }

    const char *patterns = NULL;
    int num_patterns; 

    int num_args = 0;
    for(int i = 1; i < argc; i++) {
        if(argv[i][0] == '-' && strlen(argv[i]) != 1) continue;
        num_args += 1;
        if(patterns == NULL) {
            num_patterns = replace_char(argv[i], '\n', '\0') + 1;
            patterns = argv[i];
        } else { /*argv[i] is a path*/
            process_file(argv[i], patterns, num_patterns, count, invert, number_lines);
        }
    }

    if(num_args == 0) {
        errorc("Not enough arguments! No file specified!\n");
    }

    if(num_args == 1) {
        process_file("-", patterns, num_patterns, count, invert, number_lines);
    }
}