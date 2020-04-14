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

// 현재 실행중인 프로세스의 burst_time에서 하나 감소시킴
// 이후에 아직 다 안썼으면 1 반환
// I/O 요청했으면 0
// 종료되었으면 -1
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
        //프로세스가 끝났음
        if (arr_size == cycle_index) {
            printf("%d 종료 확인. ", process->PID);
            return -1;
        }
        printf("%d 입출력 요청 확인. ", process->PID);
        return 0;
    }
    return 1;
}

void delete_process(Process* process) {
    free(process->seq_burst);
    free(process);
    return;
}

//input.txt 파일로 입력받아서 프로세스 생성
void init_process() {
    FILE* file = fopen("input.txt", "r");
    if (file == NULL) {
        printf("파일을 찾을 수 없습니다!");
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
    //해당 프로세스가 어느 큐에 들어갈지 확인
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

// I/O burst time이 종료된 프로세스 확인
void io_check() {
    for (int i = 0; i < process_num; i++) {
        if (sleep_queue[i] != NULL) {
            int time = get_burst_time(sleep_queue[i]);
            // 종료된 프로세스라면
            if (time == 0) {
                printf("%d가 깨어났습니다. ", sleep_queue[i]->PID);
                // 우선순위를 하나 올리고 레디큐에 넣는다
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

// job queue에 남아있는 프로세스의 arrival 확인
// 남아있는 프로세스가 있으면 1, 없으면 0 반환
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
    //I/O 요청
    if (result == 0) {
        sleep_queue[PID] = process;
    }
    return result;
}

// 해당 레디큐에서 스케줄링하여 프로세스 반환
// 없으면 NULL 반환
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

    result = head->next->data; // 문제 발생
    remove = head->next;
    head->next = remove->next;
    
    free(remove);
    return result;
}

// 해당 레디큐에서 스케줄링하여 프로세스 반환
// 없으면 NULL 반환
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

// 현재 burst_time보다 짧은 프로세스가 있다면
// preemtion 발생
// 없으면 NULL 반환
// preemption 발생하면 우선순위 하나 낮춤
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

// 슬립큐에 프로세스가 하나라도 있으면 1 반환
// 없으면 0 반환
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

// 레디큐에 프로세스가 있으면 우선순위에 따라 프로세스 반환
// 레디큐와 슬립큐 모두 없으면 NULL 반환
Process* scheduling() {
    Process* result = NULL;

    //Queue 0 확인
    result = fcfs(0);
    if (result != NULL) {
        time_quantum = 2;
        return result;
    }

    //Queue 1 확인
    result = fcfs(1);
    if (result != NULL) {
        time_quantum = 6;
        return result;
    }
    //Queue 2 확인
    result = srtn();
    if (result != NULL) {
        time_quantum = -1;
        return result;
    }
    //Queue 3 확인
    result = fcfs(3);
    if (result != NULL) {
        time_quantum = -1;
        return result;
    }
    //아무런 프로세스가 존재하지 않음
    return result;
}

void start_simulation() {
    Process* current_process = NULL;
    int current_process_id = 0;
    int current_queue = -1;
    int remain_process = 1;
    printf("시뮬레이션을 시작합니다.\n");
    while (1) {
        // I/O burst time 종료된 프로세스 확인 -> 슬립큐에서 레디큐로 옮김
        //printf("I/O 확인\n");
        io_check();

        // job_queue에 남아있는 프로세스가 있는지 확인
        // arrival time 확인 -> 레디큐에 넣기
        //printf("job_queue 확인\n");
        if (remain_process == 1) {
            int result = arrival_check();
            if (result == 0) {
                remain_process = 0;
            }
        }

        //printf("실행중인 프로세스 확인: ");
        // 실행중인 프로세스 있는지 확인 
        if (current_process == NULL) {
            printf("새로 스케줄링중...");
            // 없으면 새로 스케쥴링
            current_process = scheduling();
            // 새로 스케줄링한 프로세스가 없으면 
            if (current_process == NULL) {
                printf(" 실패...");
                int check = sleep_check();
                // 남아있는 프로세스와 슬립큐를 확인함
                // 둘 다 없으면 종료함
                if (remain_process == 0 && check == 0) {
                    break;
                }
                // 하나라도 남았으면 대기함
                printf(" 대기.\n");
            }
            // 새로 스케줄링에 성공했으면
            else {
                printf(" 성공, 정보를 갱신합니다.\n");
                // 정보를 갱신함
                current_process_id = current_process->PID;
                current_queue = current_process->queue;
            }
        }
        // 실행중인 프로세스 있는 경우,
        else {
            //printf("확인 완료... ");
            // 실행중인 프로세스가 Q2에서 온 프로세스인지 확인 -> Q2 다시 검사
            if (current_queue == 2) {
                Process* preemtion_process;
                int burst_time = get_burst_time(current_process);
                preemtion_process = preemtion(burst_time);
                // 강탈 발생함
                if (preemtion_process != NULL) {
                    printf("강탈 발생함.");
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
        // global time을 1증가 (cpu running)
        global_time += 1;
        time_quantum -= 1;
        for (int i = 0; i < process_num; i++) {
            if (sleep_queue[i] != NULL) {
                int index = sleep_queue[i]->cycle_index;
                sleep_queue[i]->seq_burst[index] -= 1;
            }
        }

        // 실행중인 프로세스의 burst time을 1 줄임
        // -> 끝났으면 실행중인 프로세스에서 제거하기
        // -> I/O 요청했으면 슬립큐에 넣기
        int check = burst_check(current_process, current_queue, current_process_id);
        // 프로세스가 종료되거나 I/O를 요청함
        if (check != 1) {
            current_process = NULL;
            current_process_id = 0;
            current_queue = -1;
        }

        //타임 퀀텀을 다썼는지 확인 -> 우선순위 낮추고 레디큐 진입
        if (time_quantum == 0) {
            printf("타임퀀텀이 없습니다. ");
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