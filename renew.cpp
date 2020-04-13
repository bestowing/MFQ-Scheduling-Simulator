#include <iostream>
#include <fstream>
#include <vector>
#include <list>

using namespace std;

class Process {
public:
    int PID;              //Identification number of the process
    int queue;            //Initial queue
    int arr_t;            //Arrival time
    //int close_t;          
    int cycle_num;        //cycle number
    int cycle_index = 0;
    int* seq_burst;

    //생성자
    Process(int _PID, int _queue, int _arr, int _cycle_num) {
        PID = _PID;
        queue = _queue;
        arr_t = _arr;
        cycle_num = _cycle_num;
        int size_arr = (cycle_num * 2) - 1;
        seq_burst = new int[size_arr];
    }

    //소멸자
    ~Process() {
        delete[] this->seq_burst;
    }

    /*
    //깊은 복사
    Process(const Process* process) {
        cout << "생성";
        PID = process->PID;
        queue = process->queue;
        arr_t = process->arr_t;
        cycle_num = process->cycle_num;
        int size_arr = (cycle_num * 2) - 1;
        seq_burst = new int[size_arr];
        for (int i = 0; i < size_arr; i++) {
            seq_burst[i] = process->seq_burst[i];
        }
    }
    */

    int get_burst_time() {
        return this->seq_burst[cycle_index];
    }

    // 그냥 time_q를 다 소모하면 0을, 입출력을 요청하면 1을, 프로세스가 종료되면 2를 반환함
    int consume_bt(int time_q) {
        int remain_t = this->get_burst_time();
        if (remain_t <= time_q) {
            seq_burst[cycle_index] = 0;
            cycle_index += 1;
            int size_arr = (cycle_num * 2) - 1;
            if (cycle_index == size_arr) {
                cout << PID << "가 종료되었습니다\n";
                return 2;
            }
            cout << PID << "가 I/O를 요청했습니다 "<<"cycle index: "<<cycle_index<<"size_arr:"<<size_arr<<"\n";
            return 1;
        } else {
            cout<<"남은 시간 "<<remain_t<<"에서 "<<time_q<<"를 뺀 값: ";
            seq_burst[cycle_index] = remain_t - time_q;
            cout<<seq_burst[cycle_index]<<"\n";
            return 0;
        }
    }

    // 안깨어나면 0을, 깨어나면 1을 반환함
    int consume_bt_sleep(int time_q) {
        int remain_t = this->get_burst_time();
        if (remain_t <= time_q) {
            seq_burst[cycle_index] = 0;
            this->cycle_index += 1;
            queue = queue > 0 ? queue - 1 : 0;
            cout << PID << "가 깨어났습니다\n";
            return 1;
        }
        else {
            seq_burst[cycle_index] = remain_t - time_q;
            return 0;
        }
    }
};

vector < Process* >  sleep;
list   < Process* >  q0;
list   < Process* >  q1;
vector < Process* >  q2;
list   < Process* >  q3;
vector < pair < int, int > > chart;
int global_time = 0;

void push_queue(Process* process) {
    int position = process->queue;
    switch (position) {
    case 0:
        q0.push_back(process);
        break;
    case 1:
        q1.push_back(process);
        break;
    case 2:
        q2.push_back(process);
        break;
    default:
        q3.push_back(process);
        break;
    }
    return;
}

//input.txt 파일로 입력받아서 최초 queue에 프로세스 할당
void init_process() {
    ifstream input_file("input.txt");
    int process_num;

    if (input_file.is_open()) {
        input_file >> process_num;
        for (int i = 0; i < process_num; i++) {
            int pid, init_q, arr_t, cycle;
            int size_arr;
            input_file >> pid >> init_q >> arr_t >> cycle;
            size_arr = (cycle * 2) - 1;
            Process* new_process = new Process(pid, init_q, arr_t, cycle);
            for (int j = 0; j < size_arr; j++) {
                int tmp;
                input_file >> tmp;
                new_process->seq_burst[j] = tmp;
            }
            push_queue(new_process);
        }
    }
    else {
        cout << "파일을 찾을 수 없습니다!" << endl;
    }
    input_file.close();
    return;
}

Process* scheduling_FCFS(int q_num) {
    Process* process;
    switch (q_num) {
    case 0:
        process = q0.front();
        q0.pop_front();
        break;
    case 1:
        process = q1.front();
        q1.pop_front();
        break;
    default:
        process = q3.front();
        q3.pop_front();
        break;
    }
    return process;
}

//q2에서 스케줄링받은 프로세스가 preemption되는건 같은 q2에서의 프로세스와 burst time을 비교했을때만이다.
//
Process* scheduling_q2(void) {
    cout<<"2번째 큐에 남은 모든 프로세스의 burst time을 비교합니다\n";
    vector< Process* >::iterator ptr;
    Process * result = q2.front();
    int min_t = result->get_burst_time();
    int delete_num = 0;
    int cnt = 1;
    for (ptr = q2.begin()+1; ptr != q2.end(); ++ptr) {
        if ((*ptr)->get_burst_time() < min_t) {
            result = *ptr;
            delete_num = cnt;
        }
        ++cnt;
    }
    q2.erase(q2.begin() + delete_num);
    return result;
}

void cpu_working(Process* process, int time_q) {
    cout<<process->PID<<" 를 실행합니다: ";
    //2번째 큐에서 온 프로세스라면
    if( process->queue == 2) {
        //끝나거나 I/O 요청할때까지 실행한다
        while(1) {
            bool is_stoped = false;
            chart.push_back(make_pair(process->PID, global_time));
            global_time += 1;
            //cout<<"cpu 타임: "<<global_time<<"\n";
            
            //단, time quantum이 1 증가할때마다 preemtion을 검사한다.
            vector< Process* >::iterator ptr;
            for (ptr = sleep.begin(); ptr != sleep.end(); ) {
                int result = (*ptr)->consume_bt_sleep(1);
                if (result == 1) {                      //깨어났을경우
                    push_queue(*ptr);
                    if ( (*ptr)->queue == 2 ) is_stoped = true;
                    ptr = sleep.erase(ptr);
                } else {
                    ++ptr;
                }
            }
            
            int consume_result = process->consume_bt(1);
            if (consume_result == 1) {                // 입출력을 요청함 -> sleep
                cout << process->PID << "를 재웠습니다\n";
                sleep.push_back(process);
                break;
            } else if (consume_result == 2) {         // 프로세스가 종료됨
                cout << process->PID<<" 의 종료 확인.\n";
                break;
            }

            //I/O 요청이 끝나고 들어온 프로세스가 있고, 그 프로세스의 burst time이 기존 프로세스보다 짧다면 반복문 탈출
            if( is_stoped == true ) {
                cout<<"preemtion 발생?\n";
                process = scheduling_q2();
            }
        }
        return;
    }
     
    //Gantt Chart 그리는 정보 담기
    chart.push_back(make_pair(process->PID, global_time));
    //시간 지남
    cout<<"현재시간 "<<global_time<<"에서 "<<time_q<<"만큼 실행했습니다: ";
    global_time += time_q;

    vector< Process* >::iterator ptr;
    for (ptr = sleep.begin(); ptr != sleep.end();) {
        int result = (*ptr)->consume_bt_sleep(time_q);
        if (result == 1) {                      //깨어났을경우
            push_queue(*ptr);
            ptr = sleep.erase(ptr);
        } else {
            ++ptr;
        }
    }

    int consume_result = process->consume_bt(time_q);
    if (consume_result == 0) {                // queue의 time_quantum을 모두 소모함 -> 우선순위 1 늘려서 다시 진입
        cout<<process->PID<<" 의 시간이"<<process->get_burst_time()<<" 남았습니다. 우선순위를 한 단계 낮춥니다.\n";
        process->queue += 1;
        push_queue(process);
    } else if (consume_result == 1) {         // 입출력을 요청함 -> sleep
        cout << process->PID << "를 재웠습니다\n";
        sleep.push_back(process);
    } else {                                   // 프로세스가 종료됨
        cout << process->PID<<" 의 종료 확인.\n";
    }
}

void start_simulation() {
    cout<<"시뮬레이션을 시작합니다.\n";
    while (true) {
        cout<<"running할 프로세스를 탐색합니다.\n";
        bool is_empty = true;
        //만약에 q0에 프로세스가 있다면
        if (q0.empty() == false && q0.front()->arr_t <= global_time ) {
            cout<<"0번째 큐에서 발견: ";
            //꺼내서 실행한다
            Process* running = scheduling_FCFS(0);
            cout << running->PID << "을 cpu로 보냅니다, 타임퀀텀은 2\n";
            cpu_working(running, 2);
            continue;
        }

        //만약에 q1에 프로세스가 있다면
        if (q1.empty() == false && q1.front()->arr_t <= global_time) {
            cout<<"1번째 큐에서 발견: ";
            //꺼내서 실행한다
            Process* running = scheduling_FCFS(1);
            cout << running->PID << "을 cpu로 보냅니다, 타임퀀텀은 6\n";
            cpu_working(running, 6);
            continue;
        }

        //만약에 q2에 프로세스가 있다면
        if (q2.empty() == false && q2.front()->arr_t <= global_time) {
            cout<<"2번째 큐에서 발견: ";
            //꺼내서 실행한다
            Process* running = scheduling_q2();
            cout << running->PID << "을 cpu로 보냅니다, 타임퀀텀은 무제한\n";
            cpu_working(running, 1);
            continue;
        }

        //만약에 q3에 프로세스가 있다면
        if (q3.empty() == false && q3.front()->arr_t <= global_time) {
            //꺼내서 실행한다
            Process* running = scheduling_FCFS(3);
            cout << running->PID << "을 cpu로 보냅니다, 타임퀀텀은 무제한\n";
            cpu_working(running, running->get_burst_time());
            continue;
        }

        //I/O 상태인 프로세스 기다리기
        if ( sleep.empty() == false ) {

        }

        //모든 프로세스가 종료되면 종료함
        break;
    }
    return;
}

//출력 몰빵
void print_result() {
    cout<<"끝났지롱~";
    /*
    vector< pair<int, int> >::iterator ptr;
    for (ptr = chart.begin(); ptr != chart.end(); ++ptr) {
        cout << (*ptr).first << " ";
    }
    */
}

int main() {
    init_process();
    start_simulation();
    print_result();
}