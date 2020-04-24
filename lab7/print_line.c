#define INIT_SIZE 512
#define READ_COUNT 1024
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <poll.h>

typedef struct table_element_s table_element;
struct table_element_s {
    off_t line_offset;
    size_t line_size;
};

void create_line_table(char* file_map, off_t file_size, table_element* table, size_t* table_size, size_t* line_count);
void print_lines(char* file_map, table_element* table, size_t line_count);

int main(int argc, char** argv) {
    if(argc < 2) {
        printf("Usage: %s filename\n", argv[0]);
        exit(1);
    }
    int fd;
    if((fd = open(argv[1], O_RDONLY)) == -1) {
       perror(argv[1]); 
       exit(1);
    }
    size_t line_count = 0;
    size_t table_size = INIT_SIZE;
    table_element* table = malloc(table_size * sizeof(table_element));
    if(table == NULL) {
        perror("Unable to use malloc()");
        exit(1);
    }
    off_t file_size = lseek(fd, 0, SEEK_END);
    char* file_map = (char*) mmap(NULL, file_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if(file_map == MAP_FAILED) {
        perror("Map failed");
        close(fd);
        free(table);
        exit(1);
    }
    close(fd);
    create_line_table(file_map, file_size, table, &table_size, &line_count);
    print_lines(file_map, table, line_count);
    free(table);
    exit(0);
}

void create_line_table(char* file_map, off_t file_size, table_element* table, size_t* table_size, size_t* line_count) {
    size_t symbol_count = 0;
    table[*line_count].line_offset = 0;
    for(int i = 0; i < file_size; i++) {
        symbol_count++;
        if(file_map[i] == '\n' ) {
            table[*line_count].line_size = symbol_count - 1;
            symbol_count = 0;
            (*line_count)++;
            if(*line_count == *table_size) {
                *table_size *= 2;
                table = realloc(table, *table_size * sizeof(table_element));
            }
            table[*line_count].line_offset = i + 1;
        }
    }
    table[*line_count].line_size = symbol_count;
}

void print_lines(char* file_map, table_element* table, size_t line_count) {
    ssize_t read_ret;
    char buf[READ_COUNT];
    int line_num = 1;
    struct pollfd tty_pollfd;
    tty_pollfd.fd = 0;
    tty_pollfd.events = POLLIN;
    int poll_time = 5000;
    printf("Enter number of line in 5 seconds to start:\n");
    int poll_ret = poll(&tty_pollfd, 1, poll_time); 
    if(poll_ret == -1) {
        perror("Cannot work with terminal");
        free(table);
        exit(1);
    }
    if(poll_ret == 0) {
        printf("Time is over!\n");
        printf("%s", file_map);
        free(table);
        exit(0);
    }

    char new_line_ch = '\n';
    while((read_ret = read(1, buf, READ_COUNT)) != -1) {
        buf[read_ret] = '\0';
        line_num = atoi(buf);
        if(line_num == 0)
            break;
        if(line_num < 0) {
            printf("Line number cannot be negative. Please enter positive number of line to be printed.\n");
            continue;
        }
        if(line_num > line_count) {
            printf("Line number cannot be above total number of lines in file.\n");
            continue;
        }
        write(1, file_map + table[line_num - 1].line_offset, table[line_num - 1].line_size);
        write(1, &new_line_ch, 1);
    }
}
