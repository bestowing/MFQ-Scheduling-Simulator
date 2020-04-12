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
    int * seq_burst;

    Process() {}

    /*
    Process(int _PID, int _queue, int _arr, int _cycle_num) {
        PID = _PID;
        queue = _queue;
        arr_t = _arr;
        cycle_num = _cycle_num;        
    }
    */

    void setBurst() {
        int size_arr = (cycle_num * 2) - 1 ;
        seq_burst = new int[size_arr];
    }

    /*
    int get_burst_time() {
        return seq_burst[cycle_index];
    }
    
    // 그냥 time_q를 다 소모하면 0을, 입출력을 요청하면 1을, 프로세스가 종료되면 2를 반환함
    int consume_bt(int time_q) {
        int remain_t = get_burst_time();
        if( remain_t <= time_q ) {
            seq_burst[cycle_index] = 0;
            cycle_index++;
            
            int size_arr = (cycle_num * 2) - 1;
            if( cycle_index == size_arr ) {
                cout<<PID<<"가 종료되었습니다\n";
                return 2;
            }
            cout<<PID<<"가 I/O를 요청했습니다\n";
            return 1;
        } else {
            seq_burst[cycle_index] = remain_t - time_q;
            return 0;
        }
    }

    // 안깨어나면 0을, 깨어나면 1을 반환함
    int consume_bt_sleep(int time_q) {
        int remain_t = get_burst_time();
        if( remain_t <= time_q ) {
            seq_burst[cycle_index] = 0;
            cycle_index++;
            queue = queue > 0 ? queue-1 : 0;
            cout<<PID<<"가 깨어났습니다\n";
            return 1;
        } else {
            seq_burst[cycle_index] = remain_t - time_q;
            return 0;
        }
    }*/

    ~Process() {
        delete []seq_burst;
    }
};

vector<Process> sleep;
list<Process *> q0;
list<Process> q1;
vector<Process> q2;
list<Process> q3;
vector< pair<int, int> > chart;
int global_time = 0;

void push_queue(Process process) {
    int position = process.queue;
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
    cout<<1.5<<" ";
    return;
}

//input.txt 파일로 입력받아서 최초 queue에 프로세스 할당
void init_process() {
    ifstream input_file("input.txt");
    int process_num;

    if (input_file.is_open()) {
        input_file >> process_num;
        for(int i=0; i<process_num; i++) {
            cout<<"1 ";
            Process new_process;
            int pid, init_q, arr_t, cycle;
            int size_arr;
            input_file>>pid>>init_q>>arr_t>>cycle;
            //cout<<pid<<" "<<init_q<<" "<<arr_t<<" "<<cycle<<"\n";
            new_process.PID = pid;
            new_process.queue = init_q;
            new_process.arr_t = arr_t;
            new_process.cycle_num = cycle;
            size_arr = (cycle * 2) - 1 ;
            new_process.setBurst();
            for(int j=0; j<size_arr; j++) {
                int tmp;
                input_file>>tmp;
                //cout<<tmp<<" ";
                new_process.seq_burst[j] = tmp;
            }
            push_queue(new_process);
            cout<<"2 ";
        }
    } else {
        cout << "파일을 찾을 수 없습니다!" << endl;
    }
    input_file.close();
    return;
}

/*
Process scheduling_q0(void) {
    Process process = q0.front();
    q0.pop_front();
    return process;
}

Process scheduling_q1(void) {
    Process process = q1.front();
    q1.pop_front();
    return process;
}


Process scheduling_q2(void) {
    vector< Process >::iterator ptr;
    Process result;
    int i = 0;
    int delete_num = 0;
    for (ptr = q2.begin(); ptr != q2.end(); ++ptr) {
        int min_t = 1000;
        if( (*ptr).get_burst_time() < min_t ) {
            result = *ptr;
            delete_num = i;
        }
        ++i;
    }
    q2.erase(q2.begin() + delete_num);
    return result;
}

Process scheduling_q3(void) {
    Process process = q3.front();
    q3.pop_front();
    return process;
}

void cpu_working(Process process, int time_q) {
    //Gantt Chart 그리는 정보 담기
    chart.push_back( make_pair(process.PID, global_time) );
    //시간 지남
    global_time += time_q;

    vector< Process >::iterator ptr;
    for (ptr = sleep.begin(); ptr != sleep.end(); ++ptr) {
        int result = (*ptr).consume_bt_sleep(time_q);
        if(result == 1) {                      //깨어났을경우
            push_queue(*ptr);
        }
    }

    //
    int consume_result = process.consume_bt(time_q);
    if( consume_result == 0 ) {                // queue의 time_quantum을 모두 소모함 -> 우선순위 1 늘려서 다시 진입
        process.queue += 1;
        push_queue(process);
    } else if( consume_result == 1 ) {         // 입출력을 요청함 -> sleep
        process.cycle_num += 1;
        cout<<process.PID<<"를 재웠습니다\n";
        sleep.push_back(process);
    } else {                                   // 프로세스가 종료됨
        cout<<"종료 확인.\n";
    }
}

void start_simulation() {
    while(true) {
        bool is_empty = true;
        //만약에 q0에 프로세스가 있다면
        if(q0.empty() == false) {
            //꺼내서 실행한다
            Process running = scheduling_q0();
            cout<<running.PID<<"을 실행합니다\n";
            cpu_working(running, 2);
            continue;
        }
        
        //만약에 q1에 프로세스가 있다면
        if(q1.empty() == false) {
            //꺼내서 실행한다
            Process running = scheduling_q1();
            cout<<running.PID<<"을 실행합니다\n";
            cpu_working(running, 6);
            continue;
        }

        //만약에 q2에 프로세스가 있다면
        if(q2.empty() == false) {
            //꺼내서 실행한다
            Process running = scheduling_q2();
            cout<<running.PID<<"을 실행합니다\n";
            cpu_working(running, 1);
            continue;
        }

        //만약에 q3에 프로세스가 있다면
        if(q3.empty() == false) {
            //꺼내서 실행한다
            Process running = scheduling_q3();
            cout<<running.PID<<"을 실행합니다\n";
            cpu_working(running, 500);
            continue;
        }

        //모든 프로세스가 종료되면 종료함
        break;
    }
    return;
}

//출력 몰빵
void print_result() {
    vector< pair<int, int> >::iterator ptr;
    for (ptr = chart.begin(); ptr != chart.end(); ++ptr) {
        cout << (*ptr).first << " ";
    }
}*/

int main() {
    init_process(); 
    cout<<"야";
    //start_simulation();
    //print_result();
}