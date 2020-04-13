#include <stdio.h>
#include <stdlib.h>

typedef struct Process {
    int PID;              //Identification number of the process
    int queue;            //Initial queue
    int arr_t;            //Arrival time
    int cycle_num;        //cycle number
    int cycle_index;      //index for cycle array
    int * seq_burst;
} Process;

typedef struct Queue {
    int QID;
    Process * head;
};

Process * set_process(int _PID, int _queue, int _arr_t, int _cycle_num) {
    Process * new_process = malloc(sizeof(Process));
    int arr_size;
    new_process->PID = _PID;
    new_process->queue = _queue;
    new_process->arr_t = _arr_t;
    new_process->cycle_num = _cycle_num;
    new_process->cycle_index = 0;
    arr_size = (_cycle_num * 2) - 1;
    new_process->seq_burst = malloc(sizeof(int) * arr_size);
    return new_process;
}

void delete_process(Process * process) {
    free(process->seq_burst);
    free(process);
    return;
}

int main() {
    
}