#ifndef LIST_H
#define LIST_H

#include <stdlib.h>
#include <string.h>

struct List {
    int length;
    struct Node *front;
};

struct Node {
    void *data;
    int data_length;
    int data_type;
    struct Node *next;
    struct Node *prev;
};

// initializes a linked list
int initList(struct List *l);

// inserts into linked list
// copies data from data pointer of length data length into a new node
struct Node *insertNode(struct List *l, void *data, int data_length, int data_type);

// removes from linked list
int removeNode(struct List *l, struct Node *node);

// destroys a linked list
void destroyList(struct List *l);

#endif