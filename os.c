#include <stdio.h>
#include <stdlib.h>

/*
** Data
*/

// 프로세스 구조체 선언
typedef struct Process {          
    int PID;                      // Identification number of the process
    int queue;                    // Initial queue
    int arr_t;                    // Arrival time
    int cycle_num;                // Cycle number
    int cycle_index;              // Index for cycle array
    int* seq_burst;               // Integer array storing burst time
} Process;

// 연결리스트 구조체 선언
typedef struct Node {
    struct Node* next;
    Process* data;
} Node;

// 전역변수
Process **  job_queue;            // ready queue에 arrival하기 전 프로세스
Node    *   ready_queue0;         // Q0, RR(time quantum = 2)
Node    *   ready_queue1;         // Q1, RR(time quantum = 6)
Process **  ready_queue2;         // Q2, SRTN
Node    *   ready_queue3;         // Q3, FCFS
Process **  sleep_queue;          // I/O를 요청한 프로세스
int process_num;                  // 스케줄링할 프로세스 총 개수
int global_time = 0;
int time_quantum;                 


/*
** Function
*/

// 프로세스 포인터에 메모리를 할당하고 입력받은 프로세스 정보를 넣음
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

// 입력받은 프로세스의 메모리를 해제하는 함수
void delete_process(Process* process) {
    free(process->seq_burst);  // burst time을 저장하는 배열
    free(process);
    return;
}

// Q0, Q1, Q2, Q3와 job_queue, sleep_queue에 메모리 할당
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

// input.txt 파일을 읽어 사용자로부터 정보를 입력받음
// 입력받은 정보로 프로세스와 레디큐에 메모리 할당하여 생성함
void set_simulation() {
    FILE* file = fopen("input.txt", "r");
    int pid, init_q, arr_t, cycle;
    int size_arr;
    int tmp;
    if (file == NULL) {
        printf("파일을 찾을 수 없습니다!");
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

// 입력받은 프로세스의 남은 burst time을 반환하는 함수
int get_burst_time(Process* process) {
    return process->seq_burst[process->cycle_index];
}

// 현재 실행중인 프로세스의 burst_time을 하나 줄임
// 아직 시간이 남았다면 1, I/O를 요청했으면 0, 종료되었으면 -1을 반환
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

// 입력받은 프로세스를 해당하는 큐에 push함
void push_queue(Process* process) {
    Node* head;
    // 해당 프로세스가 어느 큐에 들어갈지 확인
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

// I/O burst time이 종료된 프로세스를 확인하고 레디큐에 넣음
void io_check() {
    for (int i = 0; i < process_num; i++) {
        if (sleep_queue[i] != NULL) {
            int time = get_burst_time(sleep_queue[i]);
            if (time == 0) {
                // I/O burst를 마친 프로세스는 우선순위를 하나 올림
                sleep_queue[i]->cycle_index += 1;
                int queue = sleep_queue[i]->queue;
                queue = queue > 0 ? queue - 1 : 0;
                sleep_queue[i]->queue = queue;
                // 레디큐에 넣음
                push_queue(sleep_queue[i]);
                sleep_queue[i] = NULL;
            }
        }
    }
}

// job queue에 남아있는 프로세스의 arrival time을 확인하고 레디큐에 넣음
// 남아있는 프로세스가 있으면 1, 없으면 0 반환
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

// 입력받은 프로세스의 burst time을 확인,
// I/O를 요청한 경우 sleep_queue로 보냄
// 아직 시간이 남았다면 1, I/O를 요청했으면 0, 종료되었으면 -1을 반환
int burst_check(Process* process, int queue, int PID) {
    int result = cpu_running(process);
    //I/O 요청
    if (result == 0) {
        PID -= 1;
        sleep_queue[PID] = process;
    }
    return result;
}

// 입력받은 레디큐 (Q0 or Q1 or Q3)에서 스케줄링하여 프로세스 반환
// 반환할 프로세스가 없으면 NULL 반환
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
    // 스케줄링한 프로세스는 레디큐에서 제거
    free(remove);
    return result;
}

// 레디큐 (Q2)에서 스케줄링하여 프로세스 반환
// 반환할 프로세스가 없으면 NULL 반환
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

// 레디큐 Q2에서, 현재 실행중인 프로세스의 burst_time보다 짧은 burst_time을 가진 프로세스를 확인하고 반환함( preemption 발생 )
// 반환할 프로세스가 없으면 NULL 반환
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

// sleep_queue에 프로세스가 하나라도 있으면 1, 없으면 0 반환함
int sleep_check() {
    for (int i = 0; i < process_num; i++) {
        if (sleep_queue[i] != NULL) {
            return 1;
        }
    }
    return 0;
}

// 입력받은 프로세스를 asleep 상태로 만들어줌
// 즉, sleep_queue로 보냄
void sleep(Process* process) {
    int pid = process->PID;
    pid -= 1;
    sleep_queue[pid] = process;
    return;
}

// 우선순위를 고려한 스케줄링으로 프로세스 반환
// 레디큐 Q0, Q1, Q2, Q3가 모두 비었다면 NULL 반환
Process* scheduling() {
    Process* result = NULL;
    // Queue 0 확인
    result = fcfs(0);
    if (result != NULL) {
        time_quantum = 2;
        return result;
    }
    // Queue 1 확인
    result = fcfs(1);
    if (result != NULL) {
        time_quantum = 6;
        return result;
    }
    // Queue 2 확인
    result = srtn();
    if (result != NULL) {
        time_quantum = -1;
        return result;
    }
    // Queue 3 확인
    result = fcfs(3);
    if (result != NULL) {
        time_quantum = -1;
        return result;
    }
    // 반환할 프로세스가 없음( 레디큐가 모두 비었음 )
    return result;
}

// 시뮬레이션을 시작함
void start_simulation() {
    Process* current_process = NULL; // cpu자원을 받은 프로세스
    int current_process_id = 0;
    int current_queue = -1;
    int prev_process_id = 0;
    int remain_process = 1;
    // 모든 프로세스가 종료될때까지 반복함
    while (1) {
        // I/O burst time 종료된 프로세스 확인, sleep_queue에서 ready_queue로 넣음
        io_check();

        // job_queue에 남아있는 프로세스가 있는지 확인, 모든 프로세스가 arrive할때까지 확인함
        if (remain_process == 1) {
            int arrival_result = arrival_check();
            if (arrival_result == 0) {
                remain_process = 0;
            }
        }

        // 실행중인 프로세스 있는지 확인, 새로 스케줄링함
        if (current_process == NULL) {
            current_process = scheduling();
            if (current_process == NULL) {
                int check = sleep_check();
                if (remain_process == 0 && check == 0) {
                    // 모든 프로세스가 종료된 것을 확인하면 반복문 종료
                    break;
                }
                // I/O를 기다리는 프로세스만 남은 경우, cpu가 그냥 대기함
                current_process_id = 0;
                current_queue = -1;
            } else {
                // 새로 스케줄링한 프로세스의 정보로 갱신함
                current_process_id = current_process->PID;
                current_queue = current_process->queue;
            }
        }
        // 실행중인 프로세스 있는 경우
        else {
            // 실행중인 프로세스가 SRTN 기법인 경우, preemption 확인함
            if (current_queue == 2) {
                Process* preemtion_process;
                int burst_time = get_burst_time(current_process);
                preemtion_process = preemtion(burst_time);
                // preemption 발생
                if (preemtion_process != NULL) {
                    // 실행중이던 프로세스는 Q3으로 진입
                    current_process->queue = 3;
                    push_queue(current_process);
                    current_process = preemtion_process;
                    current_process_id = current_process->PID;
                    current_queue = 2;
                }
            }
        }
        
        // cpu를 할당받은 프로세스가 변경되는 경우, 출력함
        if(prev_process_id != current_process_id) {
            if(current_process_id == 0) {
                // cpu가 실행중인 프로세스가 없는 경우
                printf("%5d %5c\n", global_time, ' ');
            } else {
                printf("%5d %5d\n", global_time, current_process_id);
            }
            prev_process_id = current_process_id;
        }

        // cpu의 time quantum이 1 지남
        // I/O 대기중이던 프로세스의 burst time도 1씩 감소시킴
        global_time += 1;
        time_quantum -= 1;
        for (int i = 0; i < process_num; i++) {
            if (sleep_queue[i] != NULL) {
                int index = sleep_queue[i]->cycle_index;
                sleep_queue[i]->seq_burst[index] -= 1;
            }
        }

        // I/O를 기다리는 프로세스만 남아서 실행중인 프로세스 없는 경우, 그냥 넘어감
        if( current_process == NULL) continue;

        // 실행중인 프로세스의 cpu burst time을 1 줄임
        // cpu burst time이 끝났으면 실행중인 프로세스에서 제거하고 ready_queue에 넣음
        // - I/O 요청했으면, 슬립큐에 넣기
        // - 프로세스가 종료되었으면 삭제함
        int check = burst_check(current_process, current_queue, current_process_id);
        if (check != 1) {
            current_process = NULL;
            current_process_id = 0;
            current_queue = -1;
        } else {
            // time quantum을 모두 소모하고 ready_queue로 돌아가는 경우, 우선순위를 1 낮춤
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

// 큐에 할당한 메모리를 모두 반환함
void delete_queue() {
    free(job_queue);
    free(ready_queue0);
    free(ready_queue1);
    free(ready_queue2);
    free(ready_queue3);
    free(sleep_queue);
}

int main() {
    set_simulation();   // 시뮬레이션 세팅
    start_simulation(); // 시뮬레이션 시작
    delete_queue();     // 자원 반납
}