#include <stdio.h>
#include <stdlib.h>

int queue_size;

typedef struct node {
    float val;
    struct node *next;
    
} node_t;

void enqueue(node_t **head, float val) {
    node_t *new_node = (node_t*) malloc(sizeof(node_t));
    if (!new_node) return;

    new_node->val = val;
    new_node->next = *head;

    *head = new_node;
    queue_size++;
}

float dequeue(node_t **head) {
    node_t *current, *prev = NULL;
    float retval = -1.0f;

    if (*head == NULL) return -66.6f;

    current = *head;
    while (current->next != NULL) {
        prev = current;
        current = current->next;
    }

    retval = current->val;
    free(current);
    
    if (prev)
        prev->next = NULL;
    else
        *head = NULL;
    
    queue_size--;

    return retval;
}

void print_list(node_t *head) {
    node_t *current = head;

    while (current != NULL) {
        printf("%f\n", current->val);
        current = current->next;
    }
}

/*
int main() {
    node_t *head = NULL;
    int ret;

    enqueue(&head, 11);
    enqueue(&head, 22);
    enqueue(&head, 33);
    enqueue(&head, 44);

    print_list(head);
    
    while ((ret=dequeue(&head)) > 0) {
        printf("dequeued %d\n", ret);
    }
    printf("done. head=%p\n", head);

    return 0;
}

*/