/*
pq.c

Unsorted Array Implementation

Implementations for helper functions for priority queue construction and
manipulation.

Skeleton written by Grady Fitzpatrick for COMP20007 Assignment 1 2021
*/
#include <stdlib.h>
#include <assert.h>
#include "pq.h"

#define INITIALITEMS 32

struct pq {
  int count;
  int allocated;
  int *queue;
  int *priorities;
};


struct pq *newPQ(){
  struct pq *pq = (struct pq *) malloc(sizeof(struct pq));
  assert(pq);
  pq->count = 0;
  pq->allocated = 0;
  pq->queue = NULL;
  pq->priorities = NULL;
  return pq;
}

void enqueue(struct pq *pq, int item, int priority){
  assert(pq);
  if((pq->count + 1) > pq->allocated){
    if (pq->allocated == 0){
      pq->allocated = INITIALITEMS;
    } else {
      pq->allocated *= 2;
    }
    pq->queue = (int *) realloc(pq->queue, pq->allocated * sizeof(int));
    assert(pq->queue);
    pq->priorities = (int *) realloc(pq->priorities, pq->allocated *
      sizeof(int));
    assert(pq->priorities);
  }
  (pq->queue)[pq->count] = item;
  (pq->priorities)[pq->count] = priority;
  (pq->count)++;
}

/* returns 1 if the pq has a given node else 0 */
int pqhasnode(struct pq *pq, int node)
{
    int i;
    for (i = 0; i < pq->count; i++)
    {
        if ((pq->queue)[i] == node) {
          return 1;
        }
    }
    return 0;
}

/* Scan through all the priorities linearly and find lowest. */
int deletemin(struct pq *pq){
  int i;
  int lowestElement = 0;
  int returnVal;
  if (pq->count <= 0){
    return -1;
  }
  for(i = 0; i < pq->count; i++){
    if((pq->priorities)[i] < (pq->priorities)[lowestElement]){
      lowestElement = i;
    }
  }
  returnVal = (pq->queue)[lowestElement];
  /* Delete item from queue by swapping final item into place of deleted
    element. */
  if(pq->count > 0){
    (pq->priorities)[lowestElement] = (pq->priorities)[pq->count - 1];
    (pq->queue)[lowestElement] = (pq->queue)[pq->count - 1];
    (pq->count)--;
  }
  return returnVal;
}

/* update the cost of a given node in the pq */
void updatecost(struct pq *pq, int node, int newcost)
{
    int i;
    for (i = 0; i < pq->count; i++)
    {
        if ((pq->queue)[i] == node) {
          (pq->priorities)[i] = newcost;
        }
    }
}

int empty(struct pq *pq){
  return pq->count == 0;
}

void freePQ(struct pq *pq){
  if(! pq) {
    return;
  }
  if(pq->allocated > 0){
    free(pq->queue);
    free(pq->priorities);
  }
  free(pq);
}