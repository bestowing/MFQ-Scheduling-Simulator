#ifndef __MFQ_HEADER
# define __MFQ_HEADER

#include <stdio.h>
#include <stdlib.h>

// Define custom struct 'Process'
typedef struct Process {          
    int PID;                      // identification number of the process
    int queue;                    // initial queue
    int arr_t;                    // arrival time
    int cycle_num;
    int cycle_index;
    int cycle_total;
    int* seq_burst;               // int array for storing burst times
} Process;

// Define custom struct 'Node'
typedef struct Node {
    struct Node* next;   // Linked list
    Process* data;       // Data: 'Process' struct
} Node;

#endif