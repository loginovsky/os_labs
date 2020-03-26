#define INIT_SIZE 512
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
    size_t table_size = INIT_SIZE;
    table_element* table = malloc(table_size * sizeof(table_element));
    if(table == NULL) {
        perror("Unable to use malloc()");
        exit(1);
    }
    char symbol;
    ssize_t read_ret = 0;
    size_t line_count = 0;
    size_t symbol_count = 0;
    table[line_count].line_offset = 0;
    while((read_ret = read(fd, &symbol, 1)) > 0) {
        symbol_count++;
        if(symbol == '\n') {
            table[line_count].line_size = symbol_count - 1;
            symbol_count = 0;
            line_count++;
            if(line_count == table_size) {
                table_size *= 2;
                table = realloc(table ,table_size * sizeof(table_element));
            }
            table[line_count].line_offset = lseek(fd, 0L, SEEK_CUR);
             
        }
    }
    if(read_ret == -1) {
        perror(argv[2]);
        close(fd);
        free(table);
        exit(1);
    }
    if(read_ret == 0) {
        table[line_count].line_size = symbol_count;
    }
    printf("Enter number of line (0 to stop):\n");
    char buf[BUFSIZ];
    int line_num = 1;
    while((read_ret = read(1, buf, BUFSIZ)) != -1) {
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
            perror(argv[2]);
            close(fd);
            free(table);
            exit(1);
        }
        buf[read_ret] = '\0';
        printf("%s\n", buf);
    }
    close(fd);
    free(table);
    exit(0);
}
