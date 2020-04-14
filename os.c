#include <stdio.h>
#include <stdlib.h>

/*
** Data
*/

// ���μ��� ����ü ����
typedef struct Process {          
    int PID;                      // Identification number of the process
    int queue;                    // Initial queue
    int arr_t;                    // Arrival time
    int cycle_num;                // Cycle number
    int cycle_index;              // Index for cycle array
    int* seq_burst;               // Integer array storing burst time
} Process;

// ���Ḯ��Ʈ ����ü ����
typedef struct Node {
    struct Node* next;
    Process* data;
} Node;

// ��������
Process **  job_queue;            // ready queue�� arrival�ϱ� �� ���μ���
Node    *   ready_queue0;         // Q0, RR(time quantum = 2)
Node    *   ready_queue1;         // Q1, RR(time quantum = 6)
Process **  ready_queue2;         // Q2, SRTN
Node    *   ready_queue3;         // Q3, FCFS
Process **  sleep_queue;          // I/O�� ��û�� ���μ���
int process_num;                  // �����ٸ��� ���μ��� �� ����
int global_time = 0;
int time_quantum;                 


/*
** Function
*/

// ���μ��� �����Ϳ� �޸𸮸� �Ҵ��ϰ� �Է¹��� ���μ��� ������ ����
Process* set_process(int _PID, int _queue, int _arr_t, int _cycle_num) {
    Process* new_process = (Process*)malloc(sizeof(Process));
    int arr_size;
    new_process->PID = _PID;
    new_process->queue = _queue;
    new_process->arr_t = _arr_t;
    new_process->cycle_num = _cycle_num;
    new_process->cycle_index = 0;
    arr_size = (_cycle_num * 2) - 1;
    new_process->seq_burst = (int*)malloc(sizeof(int) * arr_size);
    return new_process;
}

// �Է¹��� ���μ����� �޸𸮸� �����ϴ� �Լ�
void delete_process(Process* process) {
    free(process->seq_burst);  // burst time�� �����ϴ� �迭
    free(process);
    return;
}

// Q0, Q1, Q2, Q3�� job_queue, sleep_queue�� �޸� �Ҵ�
void init_queue() {
    job_queue = (Process**)malloc(sizeof(Process) * process_num);
    ready_queue0 = (Node*)malloc(sizeof(Node));
    ready_queue0->data = NULL;
    ready_queue0->next = NULL;
    ready_queue1 = (Node*)malloc(sizeof(Node));
    ready_queue1->data = NULL;
    ready_queue1->next = NULL;
    ready_queue2 = (Process**)malloc(sizeof(Process) * process_num);
    for(int i=0; i<process_num; i++) {
        ready_queue2[i] = NULL;
    }
    ready_queue3 = (Node*)malloc(sizeof(Node));
    ready_queue3->data = NULL;
    ready_queue3->next = NULL;
    sleep_queue = (Process**)malloc(sizeof(Process) * process_num);
    for(int i=0; i<process_num; i++) {
        sleep_queue[i] = NULL;
    }
}

// input.txt ������ �о� ����ڷκ��� ������ �Է¹���
// �Է¹��� ������ ���μ����� ����ť�� �޸� �Ҵ��Ͽ� ������
void set_simulation() {
    FILE* file = fopen("input.txt", "r");
    int pid, init_q, arr_t, cycle;
    int size_arr;
    int tmp;
    if (file == NULL) {
        printf("������ ã�� �� �����ϴ�!");
        return;
    }
    fscanf(file, "%d", &process_num);
    init_queue();
    for (int i = 0; i < process_num; i++) {
        fscanf(file, "%d %d %d %d", &pid, &init_q, &arr_t, &cycle);
        size_arr = (cycle * 2) - 1;
        Process* new_process = set_process(pid, init_q, arr_t, cycle);
        for (int j = 0; j < size_arr; j++) {
            fscanf(file, "%d", &tmp);
            new_process->seq_burst[j] = tmp;
        }
        job_queue[i] = new_process;
    }
    fclose(file);
    return;
}

// �Է¹��� ���μ����� ���� burst time�� ��ȯ�ϴ� �Լ�
int get_burst_time(Process* process) {
    return process->seq_burst[process->cycle_index];
}

// ���� �������� ���μ����� burst_time�� �ϳ� ����
// ���� �ð��� ���Ҵٸ� 1, I/O�� ��û������ 0, ����Ǿ����� -1�� ��ȯ
int cpu_running(Process* process) {
    int remain = get_burst_time(process);
    remain -= 1;
    process->seq_burst[process->cycle_index] = remain;
    if (remain == 0) {
        int cycle_num = process->cycle_num;
        int index = process->cycle_index;
        int arr_size = (cycle_num * 2) - 1;
        index += 1;
        process->cycle_index = index;
        if (arr_size == index) {
            delete_process(process);
            return -1;
        }
        return 0;
    }
    return 1;
}

// �Է¹��� ���μ����� �ش��ϴ� ť�� push��
void push_queue(Process* process) {
    Node* head;
    // �ش� ���μ����� ��� ť�� ���� Ȯ��
    int queue_num = process->queue;
    int pid = process->PID - 1;
    switch (queue_num) {
    case 0:
        head = ready_queue0;
        break;
    case 1:
        head = ready_queue1;
        break;
    case 2:
        ready_queue2[pid] = process;
        return;
    case 3:
        head = ready_queue3;
        break;
    default:
        break;
    }
    Node* node = (Node*)malloc(sizeof(Node));
    node->data = process;
    node->next = NULL;
    while (head->next != NULL) {
        head = head->next;
    }
    head->next = node;
    return;
}

// I/O burst time�� ����� ���μ����� Ȯ���ϰ� ����ť�� ����
void io_check() {
    for (int i = 0; i < process_num; i++) {
        if (sleep_queue[i] != NULL) {
            int time = get_burst_time(sleep_queue[i]);
            if (time == 0) {
                // I/O burst�� ��ģ ���μ����� �켱������ �ϳ� �ø�
                sleep_queue[i]->cycle_index += 1;
                int queue = sleep_queue[i]->queue;
                queue = queue > 0 ? queue - 1 : 0;
                sleep_queue[i]->queue = queue;
                // ����ť�� ����
                push_queue(sleep_queue[i]);
                sleep_queue[i] = NULL;
            }
        }
    }
}

// job queue�� �����ִ� ���μ����� arrival time�� Ȯ���ϰ� ����ť�� ����
// �����ִ� ���μ����� ������ 1, ������ 0 ��ȯ
int arrival_check() {
    int result = 0;
    for (int i = 0; i < process_num; i++) {
        if (job_queue[i] != NULL) {
            if (job_queue[i]->arr_t == global_time) {
                push_queue(job_queue[i]);
                job_queue[i] = NULL;
            } else {
                result = 1;
            }
        }
    }
    return result;
}

// �Է¹��� ���μ����� burst time�� Ȯ��,
// I/O�� ��û�� ��� sleep_queue�� ����
// ���� �ð��� ���Ҵٸ� 1, I/O�� ��û������ 0, ����Ǿ����� -1�� ��ȯ
int burst_check(Process* process, int queue, int PID) {
    int result = cpu_running(process);
    //I/O ��û
    if (result == 0) {
        PID -= 1;
        sleep_queue[PID] = process;
    }
    return result;
}

// �Է¹��� ����ť (Q0 or Q1 or Q3)���� �����ٸ��Ͽ� ���μ��� ��ȯ
// ��ȯ�� ���μ����� ������ NULL ��ȯ
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

    if (head->next == NULL) {
        return NULL;
    } 

    remove = head->next;
    result = remove->data;
    head->next = remove->next;
    // �����ٸ��� ���μ����� ����ť���� ����
    free(remove);
    return result;
}

// ����ť (Q2)���� �����ٸ��Ͽ� ���μ��� ��ȯ
// ��ȯ�� ���μ����� ������ NULL ��ȯ
Process* srtn() {
    Process* result = NULL;
    int min_time = -1;
    int index = -1;
    for (int i = 0; i < process_num; i++) {
        if (ready_queue2[i] != NULL) {
            int time = get_burst_time(ready_queue2[i]);
            if (min_time == -1 || time < min_time) {
                index = i;
                result = ready_queue2[i];
                min_time = time;
            }
        }
    }
    if (index != -1) {
        ready_queue2[index] = NULL;
        return result;
    }
    return result;
}

// ����ť Q2����, ���� �������� ���μ����� burst_time���� ª�� burst_time�� ���� ���μ����� Ȯ���ϰ� ��ȯ��( preemption �߻� )
// ��ȯ�� ���μ����� ������ NULL ��ȯ
Process* preemtion(int burst_time) {
    Process* result = NULL;
    int min_time = burst_time;
    int index = -1;
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
        return result;
    }

    return NULL;
}

// sleep_queue�� ���μ����� �ϳ��� ������ 1, ������ 0 ��ȯ��
int sleep_check() {
    for (int i = 0; i < process_num; i++) {
        if (sleep_queue[i] != NULL) {
            return 1;
        }
    }
    return 0;
}

// �Է¹��� ���μ����� asleep ���·� �������
// ��, sleep_queue�� ����
void sleep(Process* process) {
    int pid = process->PID;
    pid -= 1;
    sleep_queue[pid] = process;
    return;
}

// �켱������ ����� �����ٸ����� ���μ��� ��ȯ
// ����ť Q0, Q1, Q2, Q3�� ��� ����ٸ� NULL ��ȯ
Process* scheduling() {
    Process* result = NULL;
    // Queue 0 Ȯ��
    result = fcfs(0);
    if (result != NULL) {
        time_quantum = 2;
        return result;
    }
    // Queue 1 Ȯ��
    result = fcfs(1);
    if (result != NULL) {
        time_quantum = 6;
        return result;
    }
    // Queue 2 Ȯ��
    result = srtn();
    if (result != NULL) {
        time_quantum = -1;
        return result;
    }
    // Queue 3 Ȯ��
    result = fcfs(3);
    if (result != NULL) {
        time_quantum = -1;
        return result;
    }
    // ��ȯ�� ���μ����� ����( ����ť�� ��� ����� )
    return result;
}

// �ùķ��̼��� ������
void start_simulation() {
    Process* current_process = NULL; // cpu�ڿ��� ���� ���μ���
    int current_process_id = 0;
    int current_queue = -1;
    int prev_process_id = 0;
    int remain_process = 1;
    // ��� ���μ����� ����ɶ����� �ݺ���
    while (1) {
        // I/O burst time ����� ���μ��� Ȯ��, sleep_queue���� ready_queue�� ����
        io_check();

        // job_queue�� �����ִ� ���μ����� �ִ��� Ȯ��, ��� ���μ����� arrive�Ҷ����� Ȯ����
        if (remain_process == 1) {
            int arrival_result = arrival_check();
            if (arrival_result == 0) {
                remain_process = 0;
            }
        }

        // �������� ���μ��� �ִ��� Ȯ��, ���� �����ٸ���
        if (current_process == NULL) {
            current_process = scheduling();
            if (current_process == NULL) {
                int check = sleep_check();
                if (remain_process == 0 && check == 0) {
                    // ��� ���μ����� ����� ���� Ȯ���ϸ� �ݺ��� ����
                    break;
                }
                // I/O�� ��ٸ��� ���μ����� ���� ���, cpu�� �׳� �����
                current_process_id = 0;
                current_queue = -1;
            } else {
                // ���� �����ٸ��� ���μ����� ������ ������
                current_process_id = current_process->PID;
                current_queue = current_process->queue;
            }
        }
        // �������� ���μ��� �ִ� ���
        else {
            // �������� ���μ����� SRTN ����� ���, preemption Ȯ����
            if (current_queue == 2) {
                Process* preemtion_process;
                int burst_time = get_burst_time(current_process);
                preemtion_process = preemtion(burst_time);
                // preemption �߻�
                if (preemtion_process != NULL) {
                    // �������̴� ���μ����� Q3���� ����
                    current_process->queue = 3;
                    push_queue(current_process);
                    current_process = preemtion_process;
                    current_process_id = current_process->PID;
                    current_queue = 2;
                }
            }
        }
        
        // cpu�� �Ҵ���� ���μ����� ����Ǵ� ���, �����
        if(prev_process_id != current_process_id) {
            if(current_process_id == 0) {
                // cpu�� �������� ���μ����� ���� ���
                printf("%5d %5c\n", global_time, ' ');
            } else {
                printf("%5d %5d\n", global_time, current_process_id);
            }
            prev_process_id = current_process_id;
        }

        // cpu�� time quantum�� 1 ����
        // I/O ������̴� ���μ����� burst time�� 1�� ���ҽ�Ŵ
        global_time += 1;
        time_quantum -= 1;
        for (int i = 0; i < process_num; i++) {
            if (sleep_queue[i] != NULL) {
                int index = sleep_queue[i]->cycle_index;
                sleep_queue[i]->seq_burst[index] -= 1;
            }
        }

        // I/O�� ��ٸ��� ���μ����� ���Ƽ� �������� ���μ��� ���� ���, �׳� �Ѿ
        if( current_process == NULL) continue;

        // �������� ���μ����� cpu burst time�� 1 ����
        // cpu burst time�� �������� �������� ���μ������� �����ϰ� ready_queue�� ����
        // - I/O ��û������, ����ť�� �ֱ�
        // - ���μ����� ����Ǿ����� ������
        int check = burst_check(current_process, current_queue, current_process_id);
        if (check != 1) {
            current_process = NULL;
            current_process_id = 0;
            current_queue = -1;
        } else {
            // time quantum�� ��� �Ҹ��ϰ� ready_queue�� ���ư��� ���, �켱������ 1 ����
            if (time_quantum == 0) {
                current_queue = current_queue < 3 ? current_queue + 1 : 3 ;
                current_process->queue = current_queue;
                push_queue(current_process);
                current_process = NULL;
                current_process_id = 0;
                current_queue = -1;
            }
        }
    }
}

/*
void print_info() {
    for (int i = 0; i < process_num; i++) {
        if (process_list[i] != NULL) {
            printf("%d %d %d %d\n", process_list[i]->PID, process_list[i]->queue, process_list[i]->arr_t, process_list[i]->cycle_num);
            int cycle = process_list[i]->cycle_num;
            cycle = cycle * 2 - 1;
            for (int j = 0; j < cycle; j++) {
                printf("%d ", process_list[i]->seq_burst[j]);
            }
            printf("\n");
        }
    }
}
*/

// ť�� �Ҵ��� �޸𸮸� ��� ��ȯ��
void delete_queue() {
    free(job_queue);
    free(ready_queue0);
    free(ready_queue1);
    free(ready_queue2);
    free(ready_queue3);
    free(sleep_queue);
}

int main() {
    set_simulation();   // �ùķ��̼� ����
    start_simulation(); // �ùķ��̼� ����
    delete_queue();     // �ڿ� �ݳ�
}