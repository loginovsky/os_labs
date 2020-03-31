#define INIT_SIZE 512
#define READ_COUNT 1024
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>

typedef struct table_element_s table_element;
struct table_element_s {
    off_t line_offset;
    size_t line_size;
};

void create_line_table(int fd, table_element* table, size_t* table_size, size_t* line_count);
void print_lines(int fd, table_element* table, size_t line_count);

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
    /*for(int i = 0; i < line_count; i++) {
        printf("%lu %lu\n", table[i].line_offset, table[i].line_size);
    }*/
    size_t table_size = INIT_SIZE;
    table_element* table = malloc(table_size * sizeof(table_element));
    if(table == NULL) {
        perror("Unable to use malloc()");
        exit(1);
    }
    create_line_table(fd, table, &table_size, &line_count);
    print_lines(fd, table, line_count);
    close(fd);
    free(table);
    exit(0);
}

void create_line_table(int fd, table_element* table, size_t* table_size, size_t* line_count) {
    size_t symbol_count = 0;
    char symbols[READ_COUNT];
    ssize_t read_ret = 0;
    table[*line_count].line_offset = 0;
    while((read_ret = read(fd, symbols, READ_COUNT)) > 0) {
        for(int i = 0; i < read_ret; i++) {
            symbol_count++;
            if(symbols[i] == '\n' ) {
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
    }
    if(read_ret == -1) {
        perror("Cannot read from file");
        close(fd);
        free(table);
        exit(1);
    }
    if(read_ret == 0) {
        table[*line_count].line_size = symbol_count;
    }
}

void print_lines(int fd, table_element* table, size_t line_count) {
    printf("Enter number of line (0 to stop):\n");
    char buf[READ_COUNT];
    int line_num = 1;
    int read_ret;
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
        lseek(fd, table[line_num - 1].line_offset, SEEK_SET);
        if((read_ret = read(fd, buf, table[line_num - 1].line_size)) == -1) {
            perror("Cannot read from file");
            close(fd);
            free(table);
            exit(1);
        }
        buf[read_ret] = '\0';
        printf("%s\n", buf);
    }
}
