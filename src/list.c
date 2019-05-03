
#include "list.h"

int initList(struct List *l, int data_length) {
    if(data_length <= 0) {
        return -1;
    }
    l->data_length = data_length;
    l->front = 0;

    return 0;
}

int insertNode(struct List *l, void *data) {
    struct Node *end, *new;

    if(data == 0) {
        return -1;
    }

    // initiliazed new node
    new = malloc(sizeof(struct Node));
    new->data = malloc(l->data_length);
    new->next = 0;
    new->prev = 0;
    memcpy(new->data, data, l->data_length);

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

    return 0;
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