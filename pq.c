#include "pq.h"

#include "io.h"
#include "node.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

typedef struct ListElement ListElement;

struct ListElement {
    Node *tree;
    ListElement *next;
};

struct PriorityQueue {
    ListElement *list;
};

PriorityQueue *pq_create(void) {
    PriorityQueue *pq = (PriorityQueue *) calloc(1, sizeof(PriorityQueue));
    return pq;
}

void pq_free(PriorityQueue **q) {
    if (q == NULL || *q == NULL) {
        return;
    }
    free(*q);
    *q = NULL;
}

bool pq_is_empty(PriorityQueue *q) {
    return q->list == NULL;
}

bool pq_size_is_1(PriorityQueue *q) {
    return (q->list != NULL) && (q->list->next == NULL);
}

bool pq_less_than(Node *n1, Node *n2) {
    if (n1->weight < n2->weight)
        return true;
    if (n1->weight > n2->weight)
        return false;
    return n1->symbol < n2->symbol;
}

void enqueue(PriorityQueue *q, Node *tree) {
    ListElement *e = (ListElement *) calloc(1, sizeof(ListElement));
    e->tree = tree;

    if (pq_is_empty(q)) {
        // q is empty
        q->list = e;
    } else if (pq_less_than(tree, q->list->tree)) {
        // new weight < old head weight
        e->next = q->list;
        q->list = e;
    } else {
        ListElement *curr = q->list;
        while (curr->next != NULL && pq_less_than(curr->next->tree, tree)) {
            curr = curr->next;
        }
        e->next = curr->next;
        curr->next = e;
    }
}

bool dequeue(PriorityQueue *q, Node **tree) {
    if (pq_is_empty(q))
        return false;
    ListElement *e = q->list;
    *tree = e->tree;
    q->list = q->list->next;
    free(e);
    return true;
}

void pq_print(PriorityQueue *q) {
    assert(q != NULL);
    ListElement *e = q->list;
    int position = 1;
    while (e != NULL) {
        if (position++ == 1) {
            printf("=============================================\n");
        } else {
            printf("---------------------------------------------\n");
        }
        node_print_tree(e->tree, '<', 2);
        e = e->next;
    }
    printf("=============================================\n");
}
