
#include "list.h"

int initList(struct List *l) {
    l->front = 0;
    l->length = 0;

    return 0;
}

struct Node *insertNode(struct List *l, void *data, int data_length, int data_type) {
    struct Node *end, *new;

    if(data == 0) {
        return 0;
    }

    // initiliazed new node
    new = malloc(sizeof(struct Node));
    new->data_length = data_length;
    new->data_type = data_type;
    new->data = malloc(data_length);
    new->next = 0;
    new->prev = 0;
    memcpy(new->data, data, data_length);

    // Insert at the end
    end = l->front;
    while(end != 0 && end->next != 0) {
        end = end->next;
    }

    // do something special at beginning of list
    if(end == 0) {
        l->front = new;
        new->prev = 0;
    }
    else {
        end->next = new;
        new->prev = end;
        new->next = 0;
    }

    l->length ++;

    return new;
}

int removeNode(struct List *l, struct Node *node) {
    if(node == 0) {
        return -1;
    }

    if(node->next != 0) {
        node->next->prev = node->prev;
    }
    if(node->prev != 0) {
        node->prev->next = node->next;
    }
    if(node == l->front) {
        l->front = node->next;
    }
    free(node->data);
    free(node);

    l->length --;

    return 0;
}

void destroyList(struct List *l) {
    struct Node *current, *temp;

    // free all dynamically allocated memory
    current = l->front;
    while(current != 0) {
        temp = current;
        current = current->next;
        free(temp->data);
        free(temp);
    }
}