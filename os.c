#include <stdio.h>
#include <stdlib.h>
#define MAX 5
/*
** Data
*/

typedef struct Process {
    int PID;              //Identification number of the process
    int queue;            //Initial queue
    int arr_t;            //Arrival time
    int cycle_num;        //cycle number
    int cycle_index;      //index for cycle array
    int* seq_burst;
} Process;

typedef struct Node {
    struct Node* next;
    Process* data;
} Node;

Process* process_list[MAX];
Node* ready_queue0;
Node* ready_queue1;
Process* ready_queue2[MAX];
Node* ready_queue3;
Process* sleep_queue[MAX];
int global_time = 0;
int process_num;
int time_quantum;

/*
** Function
*/

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

int get_burst_time(Process* process) {
    return process->seq_burst[process->cycle_index];
}

// ���� �������� ���μ����� burst_time���� �ϳ� ���ҽ�Ŵ
// ���Ŀ� ���� �� �Ƚ����� 1 ��ȯ
// I/O ��û������ 0
// ����Ǿ����� -1
int cpu_running(Process* process) {
    int remain = process->seq_burst[process->cycle_index];
    remain -= 1;
    process->seq_burst[process->cycle_index] = remain;
    if (remain == 0) {
        int cycle_num = process->cycle_num;
        int cycle_index = process->cycle_index;
        int arr_size = (cycle_num * 2) - 1;
        cycle_index += 1;
        process->cycle_index = cycle_index;
        //���μ����� ������
        if (arr_size == cycle_index) {
            printf("%d ���� Ȯ��. ", process->PID);
            return -1;
        }
        printf("%d ����� ��û Ȯ��. ", process->PID);
        return 0;
    }
    return 1;
}

void delete_process(Process* process) {
    free(process->seq_burst);
    free(process);
    return;
}

//input.txt ���Ϸ� �Է¹޾Ƽ� ���μ��� ����
void init_process() {
    FILE* file = fopen("input.txt", "r");
    if (file == NULL) {
        printf("������ ã�� �� �����ϴ�!");
        return;
    }
    fscanf(file, "%d", &process_num);
    for (int i = 0; i < process_num; i++) {
        int pid, init_q, arr_t, cycle;
        int size_arr;
        fscanf(file, "%d %d %d %d", &pid, &init_q, &arr_t, &cycle);
        size_arr = (cycle * 2) - 1;
        Process* new_process = set_process(pid, init_q, arr_t, cycle);
        for (int j = 0; j < size_arr; j++) {
            int tmp;
            fscanf(file, "%d", &tmp);
            new_process->seq_burst[j] = tmp;
        }
        process_list[i] = new_process;
    }
    fclose(file);
    return;
}

void init_queue() {
    ready_queue0 = (Node*)malloc(sizeof(Node));
    ready_queue1 = (Node*)malloc(sizeof(Node));
    ready_queue3 = (Node*)malloc(sizeof(Node));
    ready_queue0->data = NULL;
    ready_queue0->next = NULL;
    ready_queue1->data = NULL;
    ready_queue1->next = NULL;
    ready_queue3->data = NULL;
    ready_queue3->next = NULL;
}

void push_queue(Process* process) {
    Node* head;
    //�ش� ���μ����� ��� ť�� ���� Ȯ��
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

// I/O burst time�� ����� ���μ��� Ȯ��
void io_check() {
    for (int i = 0; i < process_num; i++) {
        if (sleep_queue[i] != NULL) {
            int time = get_burst_time(sleep_queue[i]);
            // ����� ���μ������
            if (time == 0) {
                printf("%d�� ������ϴ�. ", sleep_queue[i]->PID);
                // �켱������ �ϳ� �ø��� ����ť�� �ִ´�
                sleep_queue[i]->cycle_index += 1;
                int queue = sleep_queue[i]->queue;
                queue = queue > 0 ? queue - 1 : 0;
                sleep_queue[i]->queue = queue;
                push_queue(sleep_queue[i]);
                sleep_queue[i] = NULL;
            }
        }
    }
}

// job queue�� �����ִ� ���μ����� arrival Ȯ��
// �����ִ� ���μ����� ������ 1, ������ 0 ��ȯ
int arrival_check() {
    int result = 0;
    for (int i = 0; i < process_num; i++) {
        if (process_list[i] != NULL) {
            result = 1;
            if (process_list[i]->arr_t <= global_time) {
                push_queue(process_list[i]);
                process_list[i] = NULL;
            }
        }
    }
    if (result == 1) {
        return 1;
    }
    else {
        return 0;
    }
}

int burst_check(Process* process, int queue, int PID) {
    int result = cpu_running(process);
    //I/O ��û
    if (result == 0) {
        sleep_queue[PID] = process;
    }
    return result;
}

// �ش� ����ť���� �����ٸ��Ͽ� ���μ��� ��ȯ
// ������ NULL ��ȯ
Process* fcfs(int type) {
    Process* result = NULL;
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

    result = head->next->data; // ���� �߻�
    remove = head->next;
    head->next = remove->next;
    
    free(remove);
    return result;
}

// �ش� ����ť���� �����ٸ��Ͽ� ���μ��� ��ȯ
// ������ NULL ��ȯ
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

// ���� burst_time���� ª�� ���μ����� �ִٸ�
// preemtion �߻�
// ������ NULL ��ȯ
// preemption �߻��ϸ� �켱���� �ϳ� ����
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

// ����ť�� ���μ����� �ϳ��� ������ 1 ��ȯ
// ������ 0 ��ȯ
int sleep_check() {
    for (int i = 0; i < process_num; i++) {
        if (sleep_queue[i] != NULL) {
            return 1;
        }
    }
    return 0;
}

void sleep(Process* process) {
    int pid = process->PID;
    pid -= 1;
    sleep_queue[pid] = process;
    return;
}

// ����ť�� ���μ����� ������ �켱������ ���� ���μ��� ��ȯ
// ����ť�� ����ť ��� ������ NULL ��ȯ
Process* scheduling() {
    Process* result = NULL;

    //Queue 0 Ȯ��
    result = fcfs(0);
    if (result != NULL) {
        time_quantum = 2;
        return result;
    }

    //Queue 1 Ȯ��
    result = fcfs(1);
    if (result != NULL) {
        time_quantum = 6;
        return result;
    }
    //Queue 2 Ȯ��
    result = srtn();
    if (result != NULL) {
        time_quantum = -1;
        return result;
    }
    //Queue 3 Ȯ��
    result = fcfs(3);
    if (result != NULL) {
        time_quantum = -1;
        return result;
    }
    //�ƹ��� ���μ����� �������� ����
    return result;
}

void start_simulation() {
    Process* current_process = NULL;
    int current_process_id = 0;
    int current_queue = -1;
    int remain_process = 1;
    printf("�ùķ��̼��� �����մϴ�.\n");
    while (1) {
        // I/O burst time ����� ���μ��� Ȯ�� -> ����ť���� ����ť�� �ű�
        //printf("I/O Ȯ��\n");
        io_check();

        // job_queue�� �����ִ� ���μ����� �ִ��� Ȯ��
        // arrival time Ȯ�� -> ����ť�� �ֱ�
        //printf("job_queue Ȯ��\n");
        if (remain_process == 1) {
            int result = arrival_check();
            if (result == 0) {
                remain_process = 0;
            }
        }

        //printf("�������� ���μ��� Ȯ��: ");
        // �������� ���μ��� �ִ��� Ȯ�� 
        if (current_process == NULL) {
            printf("���� �����ٸ���...");
            // ������ ���� �����층
            current_process = scheduling();
            // ���� �����ٸ��� ���μ����� ������ 
            if (current_process == NULL) {
                printf(" ����...");
                int check = sleep_check();
                // �����ִ� ���μ����� ����ť�� Ȯ����
                // �� �� ������ ������
                if (remain_process == 0 && check == 0) {
                    break;
                }
                // �ϳ��� �������� �����
                printf(" ���.\n");
            }
            // ���� �����ٸ��� ����������
            else {
                printf(" ����, ������ �����մϴ�.\n");
                // ������ ������
                current_process_id = current_process->PID;
                current_queue = current_process->queue;
            }
        }
        // �������� ���μ��� �ִ� ���,
        else {
            //printf("Ȯ�� �Ϸ�... ");
            // �������� ���μ����� Q2���� �� ���μ������� Ȯ�� -> Q2 �ٽ� �˻�
            if (current_queue == 2) {
                Process* preemtion_process;
                int burst_time = get_burst_time(current_process);
                preemtion_process = preemtion(burst_time);
                // ��Ż �߻���
                if (preemtion_process != NULL) {
                    printf("��Ż �߻���.");
                    current_process->queue = 3;
                    push_queue(current_process);
                    current_process = preemtion_process;
                    current_process_id = current_process->PID;
                    current_queue = 2;
                }
            }
            printf("\n");
        }

        printf("%5d %5d %3d ", global_time, current_process->PID, current_queue);
        // global time�� 1���� (cpu running)
        global_time += 1;
        time_quantum -= 1;
        for (int i = 0; i < process_num; i++) {
            if (sleep_queue[i] != NULL) {
                int index = sleep_queue[i]->cycle_index;
                sleep_queue[i]->seq_burst[index] -= 1;
            }
        }

        // �������� ���μ����� burst time�� 1 ����
        // -> �������� �������� ���μ������� �����ϱ�
        // -> I/O ��û������ ����ť�� �ֱ�
        int check = burst_check(current_process, current_queue, current_process_id);
        // ���μ����� ����ǰų� I/O�� ��û��
        if (check != 1) {
            current_process = NULL;
            current_process_id = 0;
            current_queue = -1;
        }

        //Ÿ�� ������ �ٽ���� Ȯ�� -> �켱���� ���߰� ����ť ����
        if (time_quantum == 0) {
            printf("Ÿ�������� �����ϴ�. ");
            current_process->queue += 1;
            push_queue(current_process);
            current_process = NULL;
            current_process_id = 0;
            current_queue = -1;
        }
    }
}

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

int main() {
    init_process();
    init_queue();
    //print_info();
    start_simulation();
}