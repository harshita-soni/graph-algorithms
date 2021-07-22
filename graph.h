/*
graph.h

Visible structs and functions for graph construction and manipulation.

Skeleton written by Grady Fitzpatrick for COMP20007 Assignment 1 2021 and
  modified for Assignment 2 2021
*/
#include "pq.h"

/* Definition of a graph. */
struct graph;

enum problemPart;

struct solution;

/* A particular solution to a graph problem. */
#ifndef SOLUTION_STRUCT
#define SOLUTION_STRUCT
struct solution {
  int connectedSubnets;
  int largestSubnet;
  int *largestSubnetSIDs;
  int postOutageDiameter;
  int postOutageDiameterCount;
  int *postOutageDiameterSIDs;
  int criticalServerCount;
  int *criticalServerSIDs;
};
#endif

/* Which part the program should find a solution for. */
#ifndef PART_ENUM
#define PART_ENUM
enum problemPart {
  TASK_2=0,
  TASK_3=1,
  TASK_4=2,
  TASK_7=3
};
#endif

/* Creates an undirected graph with the given numVertices and no edges and
returns a pointer to it. NumEdges is the number of expected edges. */
struct graph *newGraph(int numVertices);

/* Adds an edge to the given graph. */
void addEdge(struct graph *g, int start, int end);

/* Finds:
  - Number of connected subnetworks (before outage) (Task 2)
  - Number of servers in largest subnetwork (before outage) (Task 3)
  - SIDs of servers in largest subnetwork (before outage) (Task 3)
  - Diameter of largest subnetworks (after outage) (Task 4)
  - Number of servers in path with largest diameter - should be one more than
    Diameter if a path exists (after outage) (Task 4)
  - SIDs in largest subnetwork (after outage) (Task 4)
  - Number of critical servers (before outage) (Task 7)
  - SIDs of critical servers (before outage) (Task 7)
 */
struct solution *graphSolve(struct graph *g, enum problemPart part,
  int numServers, int numOutages, int *outages);

/* Frees all memory used by graph. */
void freeGraph(struct graph *g);

/* Sets all values to initial values so free can work for all tasks without
  change. */
void initaliseSolution(struct solution *solution);

/* Frees all data used by solution. */
void freeSolution(struct solution *solution);

/* finds the number of connected subnetworks by doing DFS traversal of graph */
void getConnectedSubnets(struct graph *g, int v, int visited[], int nvisited);

/* finds the number of servers is the largest subnetwork */
int getLargestSubnet(struct graph *g, int v, int visited[], int n);

/* helper function for task 3. finds the servers in the largest subnetwork */
void getservers(struct graph *g, int tempserver, int visited[]);

/* comparison function for inbuilt qsort */
int cmpfunc (const void * a, const void * b);

/* checks whether "vertex" has been affected by the outage */
int isOut(int vertex, int outageSIDs[], int numoutages);

/* implementation of dijkstra's algorithm to find the shortest path */
void dijkstras(struct graph *g, int start, int dist[], int prev[], 
int n, int outageSIDs[], int numoutages);

/* helper function for dijkstra's. updates cost for adjacent nodes */
void updatecosts(struct graph *g, struct pq *priq, int u, int dist[], 
int prev[], int outageSIDs[], int numoutages);

/* helper function for task 4. finds the local maximum shortest path length */
void getLocalMax(int dist[], int prev[], int *max, int *end, int n);

/* helper function for task 4. finds the global maximum shortest path length */
void getMax(int *start, int *end, int *maxpath, int tempmax, int prev[], 
int finalprev[], int numServers, int tempend, int i);

/* helper function for task 4. finds the servers in the longest shortest path */
void populateServers(int servers[], int finalprev[], int startserver, 
int endserver, int n);

/* does DFS traversal on the graph to find the push orders and highest reachable ancestors for all nodes */
void getOrderAndHRA(struct graph *g, int iscritical[], int u, int visited[], int order[], int hra[], int parent[], int numservers, int *count, int *crticalcount) ;

/* finds the minimum of the two integers provided */
int min(int a, int b);

/* finds the adjacent node of server "v" */
int isAdjacent(struct graph *g, int i, int v);