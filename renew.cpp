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
    
    // �׳� time_q�� �� �Ҹ��ϸ� 0��, ������� ��û�ϸ� 1��, ���μ����� ����Ǹ� 2�� ��ȯ��
    int consume_bt(int time_q) {
        int remain_t = get_burst_time();
        if( remain_t <= time_q ) {
            seq_burst[cycle_index] = 0;
            cycle_index++;
            
            int size_arr = (cycle_num * 2) - 1;
            if( cycle_index == size_arr ) {
                cout<<PID<<"�� ����Ǿ����ϴ�\n";
                return 2;
            }
            cout<<PID<<"�� I/O�� ��û�߽��ϴ�\n";
            return 1;
        } else {
            seq_burst[cycle_index] = remain_t - time_q;
            return 0;
        }
    }

    // �ȱ���� 0��, ����� 1�� ��ȯ��
    int consume_bt_sleep(int time_q) {
        int remain_t = get_burst_time();
        if( remain_t <= time_q ) {
            seq_burst[cycle_index] = 0;
            cycle_index++;
            queue = queue > 0 ? queue-1 : 0;
            cout<<PID<<"�� ������ϴ�\n";
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

//input.txt ���Ϸ� �Է¹޾Ƽ� ���� queue�� ���μ��� �Ҵ�
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
        cout << "������ ã�� �� �����ϴ�!" << endl;
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
    //Gantt Chart �׸��� ���� ���
    chart.push_back( make_pair(process.PID, global_time) );
    //�ð� ����
    global_time += time_q;

    vector< Process >::iterator ptr;
    for (ptr = sleep.begin(); ptr != sleep.end(); ++ptr) {
        int result = (*ptr).consume_bt_sleep(time_q);
        if(result == 1) {                      //��������
            push_queue(*ptr);
        }
    }

    //
    int consume_result = process.consume_bt(time_q);
    if( consume_result == 0 ) {                // queue�� time_quantum�� ��� �Ҹ��� -> �켱���� 1 �÷��� �ٽ� ����
        process.queue += 1;
        push_queue(process);
    } else if( consume_result == 1 ) {         // ������� ��û�� -> sleep
        process.cycle_num += 1;
        cout<<process.PID<<"�� ������ϴ�\n";
        sleep.push_back(process);
    } else {                                   // ���μ����� �����
        cout<<"���� Ȯ��.\n";
    }
}

void start_simulation() {
    while(true) {
        bool is_empty = true;
        //���࿡ q0�� ���μ����� �ִٸ�
        if(q0.empty() == false) {
            //������ �����Ѵ�
            Process running = scheduling_q0();
            cout<<running.PID<<"�� �����մϴ�\n";
            cpu_working(running, 2);
            continue;
        }
        
        //���࿡ q1�� ���μ����� �ִٸ�
        if(q1.empty() == false) {
            //������ �����Ѵ�
            Process running = scheduling_q1();
            cout<<running.PID<<"�� �����մϴ�\n";
            cpu_working(running, 6);
            continue;
        }

        //���࿡ q2�� ���μ����� �ִٸ�
        if(q2.empty() == false) {
            //������ �����Ѵ�
            Process running = scheduling_q2();
            cout<<running.PID<<"�� �����մϴ�\n";
            cpu_working(running, 1);
            continue;
        }

        //���࿡ q3�� ���μ����� �ִٸ�
        if(q3.empty() == false) {
            //������ �����Ѵ�
            Process running = scheduling_q3();
            cout<<running.PID<<"�� �����մϴ�\n";
            cpu_working(running, 500);
            continue;
        }

        //��� ���μ����� ����Ǹ� ������
        break;
    }
    return;
}

//��� ����
void print_result() {
    vector< pair<int, int> >::iterator ptr;
    for (ptr = chart.begin(); ptr != chart.end(); ++ptr) {
        cout << (*ptr).first << " ";
    }
}*/

int main() {
    init_process(); 
    cout<<"��";
    //start_simulation();
    //print_result();
}