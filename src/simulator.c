#include "main.h"

int         get_burst_time(Process* process);
int         cpu_running(Process* process);
int         push_queue(Process* process);
int         io_check(void);
int         arrival_check(void);
int         burst_check(Process* process, int queue, int PID);
Process     *fcfs(int type);
Process     *srtn(void);
Process     *preemtion(int burst_time);
int         sleep_check(void);
void        sleep(Process* process);
Process     *scheduling(void);
int         start_simulation(void);
void        delete_process(Process* process, int global_time);
void        delete_queue(void);

extern Process **  job_queue;       // processes before arriving ready queue
extern Node    *   ready_queue0;    // Q0, RR(time quantum = 2)
extern Node    *   ready_queue1;    // Q1, RR(time quantum = 6)
extern Process **  ready_queue2;    // Q2, SRTN
extern Node    *   ready_queue3;    // Q3, FCFS
extern Process **  sleep_queue;     // processes requesting I/O system call
extern int     **  process_table;   // result of the simulation
extern int         process_num;     // total number of processes to schedule

int global_time = 0;
int time_quantum;

// main simulation
int start_simulation(void) {
    Process *current_process;
    Process* preemtion_process;
    int current_process_id;
    int current_queue;
    int prev_process_id;
    int remain_process;
    int line;
    int arrival_result;
    int break_check;
    int burst_time;

    current_process = NULL;
    preemtion_process = NULL;
    current_process_id = 0;
    current_queue = -1;
    prev_process_id = -1;
    remain_process = 1;
    line = 0;
    printf("\nGantt Chart: \n\n");
    printf("  Process ID\n");
    while (1) {
        // 1. Check the process in the job queue to see if the process has arrived 
        if (remain_process == 1) {
            arrival_result = arrival_check();
            if (arrival_result == 0) {
                remain_process = 0;
            }
        }
        // 2. Check processes for which I/O system call have been completed
        if (io_check() == -1)
            return (-1);
        // 3. Check if there is an existing process
        // 3-1. Schedule new process if there is no existing process
        if (current_process == NULL) {
            current_process = scheduling();
            // Fail to schedule
            if (current_process == NULL) {
                break_check = sleep_check();
                // Finish if all processes are terminated
                if (remain_process == 0 && break_check == 0) {
                    line /= 5;
                    for(int i=0; i<line; i++) {
                        printf("|            | \n");
                    }
                    line = 0;
                    printf("|   %4d     | \n", prev_process_id);
                    printf("+-------------- %d\n\n", global_time);
                    break;
                }
                // Wait if the I/O requesting process remains
                current_process_id = 0;
                current_queue = -1;
            } else {
                current_process_id = current_process->PID;
                current_queue = current_process->queue;
            }
        }
        // 3-2. Run an existing process
        else {
            // 3-2-1. Verifies that the preemtion has occurred (only ready queue 2)
            if (current_queue == 2) {
                burst_time = get_burst_time(current_process);
                preemtion_process = preemtion(burst_time);
                if (preemtion_process != NULL) {
                    // 3-2-2. Push the previous process if the preemtion has occurred
                    current_process->queue = 3;
                    if (push_queue(current_process) == -1)
                        return (-1);
                    current_process = preemtion_process;
                    current_process_id = current_process->PID;
                    current_queue = 2;
                }
            }
        }
        if(prev_process_id != current_process_id) {
            line /= 5;
            for(int i=0; i<line; i++) {
                printf("|            | \n");
            }
            line = 0;
            if(prev_process_id == 0) {
                printf("|   wating   | \n");
            } else {
                if(prev_process_id != -1)
                printf("|   %4d     | \n", prev_process_id);
            }
            printf("+-------------- %d\n", global_time);
            if(current_process_id == 0) {
                printf("|   wating   | \n");
            } else {
                printf("|   %4d     | \n", current_process_id);
            }
            prev_process_id = current_process_id;
        } else {
            line++;
        }
        // 4. Increase global time by 1
        global_time += 1;
        time_quantum -= 1;
        for (int i = 0; i < process_num; i++) {
            if (sleep_queue[i] != NULL) {
                int index = sleep_queue[i]->cycle_index;
                sleep_queue[i]->seq_burst[index] -= 1;
            }
        }
        // (Pass if the only I/O requesting process remains)
        if( current_process == NULL) continue;
        // 5. Consume allocated time quantum 1
        // 5-1. if the process consumes its allocated time but is not finished, push the process to proper ready queue
        // 5-2. if the process requests I/O system call, push the process to sleep queue
        // 5-3. if the process is finished, terminate the process
        // 5-4. else, just continue to execute
        int check = burst_check(current_process, current_queue, current_process_id);
        if (check != 1) {
            current_process = NULL;
            current_process_id = 0;
            current_queue = -1;
        } else {
            if (time_quantum == 0) {
                current_queue = current_queue < 3 ? current_queue + 1 : 3 ;
                current_process->queue = current_queue;
                if (push_queue(current_process) == -1)
                    return (-1);
                current_process = NULL;
                current_process_id = 0;
                current_queue = -1;
            }
        }
    }
    return (0);
}

// return remaining burst time of the process
int get_burst_time(Process* process) {
    return process->seq_burst[process->cycle_index];
}

// execute the process and consume allocated time quantum 1
// if the process is finished, then return (-1)
// else if the process call I/O system call, then return (0)
// else, that means the process has not spent all time quantum, then return (1)
int cpu_running(Process* process) {
    int remain;
    int cycle_num;
    int index;
    int arr_size;
    
    remain = get_burst_time(process) - 1;
    process->seq_burst[process->cycle_index] = remain;
    if (remain == 0) {
        cycle_num = process->cycle_num;
        index = process->cycle_index;
        arr_size = (cycle_num * 2) - 1;
        index += 1;
        process->cycle_index = index;
        if (arr_size == index) {
            delete_process(process, global_time);
            return (-1);
        }
        return (0);
    }
    return (1);
}

// push the process proper ready queue
int push_queue(Process* process) {
    Node* head;
    Node* node;
    int queue_num;
    int pid;

    queue_num = process->queue;
    pid = process->PID - 1;
    switch (queue_num) {
    case 0:
        head = ready_queue0;
        break;
    case 1:
        head = ready_queue1;
        break;
    case 2:
        ready_queue2[pid] = process;
        return (0);
    case 3:
        head = ready_queue3;
        break;
    default:
        break;
    }
    node = (Node*)malloc(sizeof(Node));
    if (node == NULL)
    {
        printf("Error code 05: failed to allocate memory bytes.\n");
        return (-1);
    }
    node->data = process;
    node->next = NULL;
    while (head->next != NULL) {
        head = head->next;
    }
    head->next = node;
    return (0);
}

// check processes for which I/O system call have been completed
// push them to proper ready queue
int io_check(void) {
    for (int i = 0; i < process_num; i++) {
        if (sleep_queue[i] != NULL) {
            int time = get_burst_time(sleep_queue[i]);
            if (time == 0) {
                sleep_queue[i]->cycle_index += 1;
                int queue = sleep_queue[i]->queue;
                queue = queue > 0 ? queue - 1 : 0;
                sleep_queue[i]->queue = queue;
                if (push_queue(sleep_queue[i]) == -1)
                    return (-1);
                sleep_queue[i] = NULL;
            }
        }
    }
    return (0);
}

// check processes that remain in the job queue
// if the job queue is empty, then return (0)
// else, that means the job queue is not empty, then return (1)
// if failed to push ready queue, then return (-1)
int arrival_check(void) {
    int result;

    result = 0;
    for (int i = 0; i < process_num; i++) {
        if (job_queue[i] != NULL) {
            if (job_queue[i]->arr_t == global_time) {
                if (push_queue(job_queue[i]) == -1)
                    return (-1);
                job_queue[i] = NULL;
            } else {
                result = 1;
            }
        }
    }
    return (result);
}

// after executing the process and consuming allocated time quantum 1,
// check the burst time of the process
// if the process request I/O system call, this function push the process to sleep queue
// if the process request I/O system call, then return (0)
// else if the process has not spent all time quantum, then return (1)
// else, that means the process is finished, then return (-1)
int burst_check(Process* process, int queue, int PID) {
    int result;
    
    result = cpu_running(process);
    if (result == 0) {
        PID -= 1;
        sleep_queue[PID] = process;
    }
    return (result);
}

// schedule a process from receiving ready queue (= type)
// if there's nothing to schedule, then return (NULL)
// else, return scheduled process pointer
Process* fcfs(int type) {
    Process* result;
    Node* head;
    Node* remove;

    switch (type) {
    case 0:
        head = ready_queue0;
        break;
    case 1:
        head = ready_queue1;
        break;
    case 3:
        head = ready_queue3;
        break;
    default:
        break;
    }
    if (head == NULL || head->next == NULL) {
        return (NULL);
    } 
    remove = head->next;
    result = remove->data;
    head->next = remove->next;
    free(remove);
    return (result);
}

// schedule a process from ready queue 2, SRTN method
// if there's nothing to schedule, then return (NULL)
// else, return scheduled process pointer
Process* srtn(void) {
    Process* result;
    int min_time;
    int index;
    int burst_time;

    result = NULL;
    min_time = -1;
    index = -1;
    for (int i = 0; i < process_num; i++) {
        if (ready_queue2[i] != NULL) {
            burst_time = get_burst_time(ready_queue2[i]);
            if (min_time == -1 || burst_time < min_time) {
                index = i;
                result = ready_queue2[i];
                min_time = burst_time;
            }
        }
    }
    if (index != -1) {
        ready_queue2[index] = NULL;
        return (result);
    }
    return (result);
}

// verifies that the preemtion has occurred (only ready queue 2)
// if there's nothing to schedule, that means the preemption does not happend, then return (NULL)
Process* preemtion(int burst_time) {
    Process* result;
    int min_time;
    int index = -1;

    result = NULL;
    min_time = burst_time;
    index = -1;
    for (int i = 0; i < process_num; i++) {
        if (ready_queue2[i] != NULL) {
            int time = get_burst_time(ready_queue2[i]);
            if (time < min_time) {
                index = i;
                result = ready_queue2[i];
                min_time = time;
            }
        }
    }
    if (index != -1) {
        ready_queue2[index] = NULL;
        return (result);
    }
    return (NULL);
}

// if sleep queue is empty, then return (0)
// else, that means any process is present in the sleep queue, then return (1)
int sleep_check(void) {
    for (int i = 0; i < process_num; i++) {
        if (sleep_queue[i] != NULL) {
            return (1);
        }
    }
    return (0);
}

// sleep the process
// that means push the process to sleep queue
void sleep(Process* process) {
    int pid = process->PID;
    pid -= 1;
    sleep_queue[pid] = process;
    return ;
}

// Schedule a priority-considering process 
// if all the ready queues are empty, then return (NULL)
// else, return the process pointer
Process* scheduling(void) {
    Process* result;

    result = fcfs(0);
    if (result != NULL) {
        time_quantum = 2;
        return (result);
    }
    result = fcfs(1);
    if (result != NULL) {
        time_quantum = 6;
        return (result);
    }
    result = srtn();
    if (result != NULL) {
        time_quantum = -1;
        return (result);
    }
    result = fcfs(3);
    if (result != NULL) {
        time_quantum = -1;
        return (result);
    }
    return (result);
}

void delete_process(Process* process, int global_time) {
    int pid;
    int arrival_time;
    int total;
    int TT;

    pid = process->PID - 1;
    arrival_time = process->arr_t;
    total = process->cycle_total;
    TT = global_time - arrival_time;
    process_table[pid][0] = TT;
    process_table[pid][1] = TT - total;
    free(process->seq_burst);
    free(process);
    return;
}

void delete_queue(void) {
    free(job_queue);
    free(ready_queue0);
    free(ready_queue1);
    free(ready_queue2);
    free(ready_queue3);
    free(sleep_queue);
    free(process_table);
}