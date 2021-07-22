/*
graph.c

Set of vertices and edges implementation.

Implementations for helper functions for graph construction and manipulation.

Skeleton written by Grady Fitzpatrick for COMP20007 Assignment 1 2021
*/
#include <stdlib.h>
#include <assert.h>
#include <limits.h>
#include <string.h>
#include "graph.h"
#include "utils.h"
#include "pq.h"

#define INITIALEDGES 32
#define UNVISITED 0
#define VISITED 1
#define MYINTMAX 99999

struct edge;

/* Definition of a graph. */
struct graph {
  int numVertices;
  int numEdges;
  int allocedEdges;
  struct edge **edgeList;
};

/* Definition of an edge. */
struct edge {
  int start;
  int end;
};

struct graph *newGraph(int numVertices){
  struct graph *g = (struct graph *) malloc(sizeof(struct graph));
  assert(g);
  /* Initialise edges. */
  g->numVertices = numVertices;
  g->numEdges = 0;
  g->allocedEdges = 0;
  g->edgeList = NULL;
  return g;
}

/* Adds an edge to the given graph. */
void addEdge(struct graph *g, int start, int end){
  assert(g);
  struct edge *newEdge = NULL;
  /* Check we have enough space for the new edge. */
  if((g->numEdges + 1) > g->allocedEdges){
    if(g->allocedEdges == 0){
      g->allocedEdges = INITIALEDGES;
    } else {
      (g->allocedEdges) *= 2;
    }
    g->edgeList = (struct edge **) realloc(g->edgeList,
      sizeof(struct edge *) * g->allocedEdges);
    assert(g->edgeList);
  }

  /* Create the edge */
  newEdge = (struct edge *) malloc(sizeof(struct edge));
  assert(newEdge);
  newEdge->start = start;
  newEdge->end = end;

  /* Add the edge to the list of edges. */
  g->edgeList[g->numEdges] = newEdge;
  (g->numEdges)++;
}

/* Frees all memory used by graph. */
void freeGraph(struct graph *g){
  int i;
  for(i = 0; i < g->numEdges; i++){
    free((g->edgeList)[i]);
  }
  if(g->edgeList){
    free(g->edgeList);
  }
  free(g);
}

/* Finds:
  - Number of connected subnetworks (before outage) (Task 2)
  - Number of servers in largest subnetwork (before outage) (Task 3)
  - SIDs of servers in largest subnetwork (before outage) (Task 3)
  - Diameter of largest subnetworks (after outage) (Task 4)
  - Number of servers in path with largest diameter - should be one more than
    Diameter if a path exists (after outage) (Task 4)
  - SIDs in path with largest diameter (after outage) (Task 4)
  - Number of critical servers (before outage) (Task 7)
  - SIDs of critical servers (before outage) (Task 7)
*/
struct solution *graphSolve(struct graph *g, enum problemPart part,
  int numServers, int numOutages, int *outages){
  struct solution *solution = (struct solution *)
    malloc(sizeof(struct solution));
  assert(solution);
  /* Initialise solution values */
  initaliseSolution(solution);
  int visited[numServers];
  if(part == TASK_2){
    /* TASK 2 SOLUTION */
    solution->connectedSubnets = 0;
    int i;

    for(i = 0; i < numServers; i++) {
      visited[i] = UNVISITED;
    }

    for(i = 0; i < numServers; i++) {
      if (visited[i]) continue;
      /* do a dfs traversal of the subnetwork */
      getConnectedSubnets(g, i, visited, numServers);

      /* each time the function getConnectedSubnets is called, it means we have found another subnetwork which has node "i" in it */
      solution->connectedSubnets += 1;
    }

  } else if(part == TASK_3) {
    /* TASK 3 SOLUTION */
    solution->largestSubnet = 0;
    solution->largestSubnetSIDs = NULL;

    int i, tempnum;
    int tempserver = -1, *servers;

    for(i = 0; i < numServers; i++) {
      visited[i] = UNVISITED;
    }

    for(i = 0; i < numServers; i++) {
      if (!(visited[i])) {
        /* tempnum stores the length of the current subnetwork which has node "i" in it. This is the local maximum */
        tempnum = 1 + getLargestSubnet(g, i, visited, numServers);

        /* compare with the global maximum and update if bigger */
        if (tempnum > solution->largestSubnet) {
          solution->largestSubnet = tempnum;
          tempserver = i;
        } else if (tempnum == solution->largestSubnet && tempserver > i) {
          /* if two subnetworks with the same number of nodes found, pick the one with the smaller server */
          solution->largestSubnet = tempnum;
          tempserver = i;
        }
      }
    }

    for(i = 0; i < numServers; i++) {
      visited[i] = UNVISITED;
    }

    /* populate the array visited to find which servers are in the largest subnetwork found previously */
    getservers(g, tempserver, visited);

    servers = (int*)malloc(sizeof(int)*solution->largestSubnet);
    assert(servers);
    int j = 0;
    for (i = 0; i < numServers && j < solution->largestSubnet; i++) {
      if (visited[i]) {
        servers[j] = i;
        j++;
      }
    }
    qsort(servers, solution->largestSubnet, sizeof(int), cmpfunc);
    solution->largestSubnetSIDs = servers;

  } else if(part == TASK_4) {
    /* TASK 4 SOLUTION */
    solution->postOutageDiameter = 0;
    solution->postOutageDiameterCount = 0;
    solution->postOutageDiameterSIDs = NULL;

    int dist[numServers], prev[numServers], finalprev[numServers], i;
    int startserver, endserver, maxpathlength = 0, tempmax = 0, tempend;

    for (i = 0; i<numServers; i++) {
      if (isOut(i, outages, numOutages)) continue;
      /* "i" is a node that hasn't been affected by the outage, get the shortest paths from i to other nodes in the network */
      dijkstras(g, i, dist, prev, numServers, outages, numOutages);

      /* find the length of longest shortest path from this node and the end server of that path */
      getLocalMax(dist, prev, &tempmax, &tempend, numServers);

      /* compare the current maximum with local maximum path length, update if bigger */
      getMax(&startserver, &endserver, &maxpathlength, tempmax, prev, finalprev, numServers, tempend, i);
    }
    solution->postOutageDiameter = maxpathlength;
    solution->postOutageDiameterCount = solution->postOutageDiameter + 1;

    int *servers = (int*)malloc(sizeof(int)*solution->postOutageDiameterCount);
    assert(servers);

    /* get the servers in the previously found longest shortest path */
    populateServers(servers, finalprev, startserver, endserver, solution->postOutageDiameter);

    solution->postOutageDiameterSIDs = servers;

  } else if(part == TASK_7) {
    /* TASK 7 SOLUTION */
    solution->criticalServerCount = 0;
    solution->criticalServerSIDs = NULL;

    /* The following arrays are populated during the dfs traversal of graph:
     - visited[]: keeps a track of which nodes have been visited 
     - order[]: stores the push orders of the nodes
     - parent[]: stores the parent node of each node
     - hra[]: stores the highest reachable ancestor(hra) of nodes
     - iscritical[]: 1 if a node is a critical vertex else 0
    */
    int visited[numServers], order[numServers], parent[numServers], hra[numServers], iscritical[numServers];
    int i, j = 0, count = 0, *critical, criticalServerCount = 0;

    for(i = 0; i < numServers; i++) {
      visited[i] = UNVISITED;
      iscritical[i] = 0;
      parent[i] = -1;
    }

    for(i = 0; i < numServers; i++) {
      if (!(visited[i])) {
        /* the follwing function does dfs traversal, finds the push orders and hra. then for every node, it checks whether it is a critical node and populates the iscrtical[] array */
        getOrderAndHRA(g, iscritical, i, visited, order, hra, parent, numServers, &count, &criticalServerCount);
      }
    }
    
    critical = (int*) malloc(sizeof(int) * criticalServerCount);
    assert(critical);

    for (i = 0; i < numServers && j < criticalServerCount; i++) {
      if (iscritical[i]) {
        critical[j] = i;
        j++;
      }
    }
    solution->criticalServerCount = criticalServerCount;
    solution->criticalServerSIDs = critical; 
  }
  return solution;
}

void getConnectedSubnets(struct graph *g, int v, int visited[], int nvisited) {
  /* dfs of the network containing node "v" */
  visited[v] = VISITED;

  int i, w;
  /* visit all the adjacent nodes */
  for (i = 0; i < g->numEdges; i++) {
      w = isAdjacent(g, i, v);
      if (w == -1) continue;
      
      if (!(visited[w])) {
        getConnectedSubnets(g, w, visited, nvisited);
      }
    }
}

int getLargestSubnet(struct graph *g, int v, int visited[], int n){
  visited[v] = VISITED;
  int i, w, count = 0;

  for (i = 0; i < g->numEdges; i++) {
      w = isAdjacent(g, i, v);
      if (w == -1) continue;
      
      if (!(visited[w])) {
        /* "count" keeps a track of the #nodes in the current network */
        count = count + 1 + getLargestSubnet(g, w, visited, n);
      }
    }
  return count;
}

void getservers(struct graph *g, int tempserver, int visited[]) {
  /* vitied[i] will be 1 if the node is visited else 0. this finds which nodes are in the current network */
  visited[tempserver] = VISITED;
  int i, w;

  for (i = 0; i < g->numEdges; i++) {
      w = isAdjacent(g, i, tempserver);
      if (w == -1) continue;
      
      if (!(visited[w])) {
        getservers(g, w, visited);
      }
    }
}

int cmpfunc (const void * a, const void * b) {
  /* helper function for inbuilt qsort */
  return ( *(int*)a - *(int*)b );
}

int isOut(int vertex, int outageSIDs[], int numoutages) {
  /* checks whether "vertex" has been affected by the outage */
  int i;
  for (i = 0; i < numoutages; i++) {
    if (outageSIDs[i] == vertex) {
      return 1;
    }
  }
  return 0;
}

void dijkstras(struct graph *g, int start, int dist[], int prev[], int n, int outageSIDs[], int numoutages) {
  int i, u;

  for (i = 0; i < n; i++) {
    dist[i] = MYINTMAX;
    prev[i] = -1;
  }

  dist[start] = 0;

  struct pq *priq = newPQ();
  for (i = 0; i < n; i++) {
    /* initialise priority queue with distance from source node as priority */
    enqueue(priq, i, dist[i]);
  }
  
  while(!empty(priq)) {
    u = deletemin(priq);
    updatecosts(g, priq, u, dist, prev, outageSIDs, numoutages);
  }
}

void updatecosts(struct graph *g, struct pq *priq, int u, int dist[], int prev[], int outageSIDs[], int numoutages) {
  int i, w;

  for (i = 0; i < g->numEdges; i++) {
      w = isAdjacent(g, i, u);
      if (w == -1) continue;

      /* do not take into account the servers affected by outage */
      if (isOut(w, outageSIDs, numoutages)) continue;
      
      if (pqhasnode(priq, w) && dist[u] + 1 < dist[w]) {
        dist[w] = dist[u] + 1;
        prev[w] = u;
        /* update cost for w; shorter path found */
        updatecost(priq, w, dist[w]);
      }
    }
}

void getLocalMax(int dist[], int prev[], int *max, int *end, int n) {
  int j;
  /* helper function for task 4 */
  for (j = 0; j<n; j++) {
    if(dist[j] > *max && prev[j] != -1) {
      *max = dist[j];
      *end = j;
    }
  }
}

void getMax(int *start, int *end, int *maxpath, int tempmax, int prev[], int finalprev[], int numServers, int tempend, int i) {
  /* helper function for task 4 */
  int j;
  if (tempmax > *maxpath) {
    /* longer shortest path found */
    for(j = 0; j<numServers; j++) {
      finalprev[j] = prev[j];
    }
    *start = i;
    *maxpath = tempmax;
    *end = tempend;
    
  } else if (tempmax == *maxpath) {
    /* same length & same start server, pick the path with smaller end server */
      if (*start == i) {
        if (*end > tempend) {
          *end = tempend;
          for(j = 0; j<numServers; j++) {
            finalprev[j] = prev[j];
          }
        }
      } /* same length, pick the path with smaller start server */
      else {
        if(*start > i) {
          *start = i;
          *end = tempend;
          for(j = 0; j<numServers; j++) {
            finalprev[j] = prev[j];
          }
        }
      }
  }
}

void populateServers(int servers[], int finalprev[], int startserver, int endserver, int n) {
  /* helper function for task 4 */
  int i, j;
  servers[0] = startserver;
  servers[n] = endserver;
  i = n - 1;
  j = endserver;

  while(i!=0) {
    servers[i] = finalprev[j];
    j = servers[i];
    i--;
  }
}

void getOrderAndHRA(struct graph *g, int iscritical[], int u, int visited[], int order[], int hra[], int parent[], int numservers, int *count, int *crticalcount) 
{
  int i, v, children = 0;
  *count += 1;
  
  /* mark the given node as visited, store count as its push order and hra */
  visited[u] = VISITED;

  order[u] = *count;
  hra[u] = *count;

  for (i = 0; i < g->numEdges; i++) {
      v = isAdjacent(g, i, u);
      if (v == -1) continue;

      /* u is the parent, v is the child because u is visited before and v is an adjacent node to u */
      if (!(visited[v])) {
        children += 1;
        parent[v] = u;
        getOrderAndHRA(g, iscritical, v, visited, order, hra, parent, numservers, count, crticalcount);

        hra[u] = min(hra[u], hra[v]);

        /* root condition check */
        if (parent[u] == -1 && children > 1) {
          if (!iscritical[u]) {
            iscritical[u] = 1;
            *crticalcount += 1;
          }
        }
        /* condition for non-root non-leaf nodes */
        if (parent[u] != -1 && hra[v] >= order[u]) {
          if (!iscritical[u]) {
            iscritical[u] = 1;
            *crticalcount += 1;
          }
        }
      } else if (v != parent[u]) {
        /* back edge found */
        hra[u] = min(hra[u], order[v]);
      }
    }
}

int min(int a, int b) {
  if (a < b) return a;
  else return b;
}

int isAdjacent(struct graph *g, int i, int v) {
  /* finds the adjacent node of server "v" */
  int w = -1;
  if (g->edgeList[i]->start == v) {
    w = g->edgeList[i]->end;
  } else if (g->edgeList[i]->end == v) {
    w = g->edgeList[i]->start;
  }
  return w;
}