#include "main.h"

int global_time = 0;
int time_quantum;

static int          get_burst_time(t_process *process)
{
    return process->seq_burst[process->cycle_index];
}

static void         delete_process(t_process *process, int global_time) {
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

// execute the process and consume allocated time quantum 1
// if the process is finished, then return (-1)
// else if the process call I/O system call, then return (0)
// else, that means the process has not spent all time quantum, then return (1)
static int          cpu_running(t_process *process)
{
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
static int          push_queue(t_process *process)
{
    t_node  *head;
    t_node  *node;
    int     queue_num;
    int     pid;

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
    node = (t_node*)malloc(sizeof(t_node));
    if (node == NULL)
    {
        printf("Error code 05: failed to allocate memory bytes.\n");
        return (ERROR);
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
static int          io_check(void)
{
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
static int          arrival_check(void)
{
    int result;

    result = 0;
    for (int i = 0; i < process_num; i++) {
        if (job_queue[i] != NULL) {
            if (job_queue[i]->arr_t == global_time) {
                if (push_queue(job_queue[i]) == -1)
                    return (ERROR);
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
static int          burst_check(t_process* process, int PID)
{
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
static t_process    *fcfs(int type)
{
    t_process   *result;
    t_node      *head;
    t_node      *remove;

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
		head = NULL;
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
static t_process    *srtn(void)
{
    t_process   *result;
    int         min_time;
    int         index;
    int         burst_time;

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
static t_process	*preemtion(int burst_time)
{
    t_process	*result;
    int			min_time;
    int			index;

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
static int			sleep_check(void)
{
    for (int i = 0; i < process_num; i++) {
        if (sleep_queue[i] != NULL) {
            return (1);
        }
    }
    return (0);
}

// Schedule a priority-considering process 
// if all the ready queues are empty, then return (NULL)
// else, return the process pointer
static t_process	*scheduling(void)
{
    t_process	*result;

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

// main simulation
int				start_simulation(void)
{
    t_process	*current_process;
    t_process   *preemtion_process;
    int         current_process_id;
    int         current_queue;
    int         prev_process_id;
    int         remain_process;
    int         line;
    int         arrival_result;
    int         break_check;
    int         burst_time;

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
                    if (push_queue(current_process) == ERROR)
                        return (ERROR);
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
        int check = burst_check(current_process, current_process_id);
        if (check != 1) {
            current_process = NULL;
            current_process_id = 0;
            current_queue = -1;
        } else {
            if (time_quantum == 0) {
                current_queue = current_queue < 3 ? current_queue + 1 : 3 ;
                current_process->queue = current_queue;
                if (push_queue(current_process) == ERROR)
                    return (ERROR);
                current_process = NULL;
                current_process_id = 0;
                current_queue = -1;
            }
        }
    }
    return (0);
}

void			delete_queue(void)
{
    free(job_queue);
    free(ready_queue0);
    free(ready_queue1);
    free(ready_queue2);
    free(ready_queue3);
    free(sleep_queue);
    free(process_table);
}
