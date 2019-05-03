#ifndef LIST_H
#define LIST_H

#include <stdlib.h>
#include <string.h>

struct List {
    int data_length;
    struct Node *front;
};

struct Node {
    void *data;
    struct Node *next;
    struct Node *prev;
};

// initializes a linked list
int initList(struct List *l, int data_length);

// inserts into linked list
int insertNode(struct List *l, void *data);

// removes from linked list
int removeNode(struct List *l, struct Node *node);

// destroys a linked list
void destroyList(struct List *l);

#endif