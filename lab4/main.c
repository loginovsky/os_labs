#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

typedef struct list_node_s list_node;
struct list_node_s {
    list_node* next;
    char* line;
};
void delete_list(list_node* head);
void push_node(list_node** head, list_node* new_node);

int main() {
    list_node* list_head = NULL;
    list_node* node;
    char buf[BUFSIZ];
    char* line_ptr;
    size_t line_size;
    printf("This program prints lines that were entered.\nEnter lines (begin line with \".\" to stop entering):\n");
    while(fgets(buf, sizeof(buf), stdin) && buf[0] != '.') {
        line_size = strlen(buf) + 1;
        line_ptr = malloc(line_size * sizeof(char));
        if(line_ptr == NULL) {
            perror("Cannot use malloc()");
            delete_list(list_head);
            exit(1);
        }
        strcpy(line_ptr, buf);
        node = malloc(sizeof(list_node));
        if(node == NULL) {
            perror("Cannot use malloc()");
            free(line_ptr);
            delete_list(list_head);
            exit(1);
        }
        node->line = line_ptr;
        node->next = NULL;
        push_node(&list_head, node);
    }
    node = list_head;
    while(node != NULL) {
        if(fputs(node->line, stdout) == EOF) {
            perror("Cannot use fputs()");
            delete_list(list_head);
            exit(1);
        }
        node = node->next;
    }
    delete_list(list_head);
    exit(0);
}

void delete_list(list_node* head) {
    list_node* tmp;
    while(head != NULL) {
        tmp = head;
        head = head->next;
        free(tmp->line); 
        free(tmp);
    }
}
void push_node(list_node** head_ptr, list_node* new_node) {
    if(*head_ptr == NULL) {
        *head_ptr = new_node;
        return;
    }
    list_node* head = *head_ptr;
    while(head->next != NULL) {
        head = head->next;
    }
    head->next = new_node;
}
