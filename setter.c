#include "main.h"

int         set_simulation(void);
int         init_queue(void);
int         set_processes(FILE *file);
Process     *init_process(int _PID, int _queue, int _arr_t, int _cycle_num);

Process    **  job_queue;           // processes before arriving ready queue
Node       *   ready_queue0;        // Q0, RR(time quantum = 2)
Node       *   ready_queue1;        // Q1, RR(time quantum = 6)
Process    **  ready_queue2;        // Q2, SRTN
Node       *   ready_queue3;        // Q3, FCFS
Process    **  sleep_queue;         // processes requesting I/O system call
int        **  process_table;       // result of the simulation
int            process_num;

// setting for simulation
// 1. open file read-only
// 2. get process number
// 3. initiate ready queues
// 4. initiate processes and set their info
// catch exception (code 01 ~ 04)
int set_simulation(void) {
    FILE *file;

    file = fopen("input.txt", "r");
    if (file == NULL)
    {
        printf("Error code 01: failed to find \"input.txt\" file.\n");
        return (-1);
    }
    if (fscanf(file, "%d", &process_num) == -1)
    {
        printf("Error code 02: failed to read \"input.txt\" file.\n");
        return (-1);
    }
    if (init_queue() == -1)
    {
        printf("Error code 03: failed to allocate memory bytes.\n");
        return (-1);
    }
    if (set_processes(file) == -1)
    {
        printf("Error code 04: failed to read \"input.txt\" file or allocate memory bytes.\n");
        return (-1);
    }
    return (0);
}

// allocate memory for ready Q0, Q1, Q2, Q3, job_queue, and sleep_queue
// catch exception (code 03)
int init_queue(void) {
    job_queue = (Process**)malloc(sizeof(Process) * process_num);
    if (job_queue == NULL)
        return (-1);
    ready_queue0 = (Node*)malloc(sizeof(Node));
    if (ready_queue0 == NULL)
        return (-1);
    ready_queue0->data = NULL;
    ready_queue0->next = NULL;
    ready_queue1 = (Node*)malloc(sizeof(Node));
    if (ready_queue1 == NULL)
        return (-1);
    ready_queue1->data = NULL;
    ready_queue1->next = NULL;
    ready_queue2 = (Process**)malloc(sizeof(Process) * process_num);
    if (ready_queue2 == NULL)
        return (-1);
    for(int i=0; i<process_num; i++) {
        ready_queue2[i] = NULL;
    }
    ready_queue3 = (Node*)malloc(sizeof(Node));
    if (ready_queue3 == NULL)
        return (-1);
    ready_queue3->data = NULL;
    ready_queue3->next = NULL;
    sleep_queue = (Process**)malloc(sizeof(Process) * process_num);
    if (sleep_queue == NULL)
        return (-1);
    for (int i = 0; i < process_num; i++) {
        sleep_queue[i] = NULL;
    }
    process_table = (int**)malloc(sizeof(int*) * process_num);
    if (process_table == NULL)
        return (-1);
    for (int i = 0; i < process_num; i++)
    {
        process_table[i] = malloc(sizeof(int) * 2);
        if (process_table[i] == NULL)
            return (-1);
    }
    return (0);
}

// read each process info from input.txt file
// init process and put the info on each process
// catch exception (code 04)
int set_processes(FILE *file) {
    Process* new_process;
    int pid, init_q, arr_t, cycle;
    int size_arr;
    int file_value;

    for (int i = 0; i < process_num; i++) {
        int total = 0;
        file_value = 0;
        if (fscanf(file, "%d %d %d %d", &pid, &init_q, &arr_t, &cycle) == -1)
            return (-1);
        size_arr = (cycle * 2) - 1;
        new_process = init_process(pid, init_q, arr_t, cycle);
        if (new_process == NULL)
            return (-1);
        for (int j = 0; j < size_arr; j++) {
            if (fscanf(file, "%d", &file_value) == -1)
                return (-1);
            total += file_value;
            new_process->seq_burst[j] = file_value;
        }
        new_process->cycle_total = total;
        job_queue[i] = new_process;
    }
    fclose(file);
    return (0);
}

// allocate memory for each process and initiate
Process* init_process(int _PID, int _queue, int _arr_t, int _cycle_num) {
    int arr_size;

    Process* new_process = (Process*)malloc(sizeof(Process));
    if (new_process == NULL)
        return (new_process);
    new_process->PID = _PID;
    new_process->queue = _queue;
    new_process->arr_t = _arr_t;
    new_process->cycle_num = _cycle_num;
    new_process->cycle_index = 0;
    new_process->cycle_total = 0;
    arr_size = (_cycle_num * 2) - 1;
    new_process->seq_burst = (int*)malloc(sizeof(int) * arr_size);
    if (new_process->seq_burst == NULL)
        return (NULL);
    return (new_process);
}