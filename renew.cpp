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
    int cycle_num;        //cycle number
    int cycle_index = 0;
    int* seq_burst;

    //������
    Process(int _PID, int _queue, int _arr, int _cycle_num) {
        PID = _PID;
        queue = _queue;
        arr_t = _arr;
        cycle_num = _cycle_num;
        int size_arr = (cycle_num * 2) - 1;
        seq_burst = new int[size_arr];
    }

    //�Ҹ���
    ~Process() {
        delete[] this->seq_burst;
    }

    int get_burst_time() {
        return this->seq_burst[cycle_index];
    }

    // �׳� time_q�� �� �Ҹ��ϸ� 0��, ������� ��û�ϸ� 1��, ���μ����� ����Ǹ� 2�� ��ȯ��
    int consume_bt(int time_q) {
        int remain_t = this->get_burst_time();
        if (remain_t <= time_q) {
            seq_burst[cycle_index] = 0;
            cycle_index += 1;
            int size_arr = (cycle_num * 2) - 1;
            if (cycle_index == size_arr) {
                return 2;
            }
            //cout << PID << "�� I/O�� ��û�߽��ϴ� "<<"cycle index: "<<cycle_index<<"size_arr:"<<size_arr<<"\n";
            return 1;
        } else {
            cout<<"���� �ð� "<<remain_t<<"���� "<<time_q<<"�� �� ��: ";
            seq_burst[cycle_index] = remain_t - time_q;
            cout<<seq_burst[cycle_index]<<"\n";
            return 0;
        }
    }

    // �ȱ���� 0��, ����� 1�� ��ȯ��
    int consume_bt_sleep(int time_q) {
        int remain_t = this->get_burst_time();
        if (remain_t <= time_q) {
            seq_burst[cycle_index] = 0;
            this->cycle_index += 1;
            //����� ���μ����� �켱������ �ϳ� ������
            queue = queue > 0 ? queue - 1 : 0;
            return 1;
        } else {
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

//input.txt ���Ϸ� �Է¹޾Ƽ� ���μ��� ����, ���� queue�� ���μ��� �Ҵ�
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
        cout << "������ ã�� �� �����ϴ�!" << endl;
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

//�����ٸ��� ���μ����� ������ cpu ������ ��ٸ���
//� ���μ��� �ϳ��� ��������� ��� time quantum�� �Ҹ��ϸ鼭 ��ٸ���
void wating_IO() {
    while(1) {
        bool is_stoped = false;
        chart.push_back(make_pair(-1, global_time));
        global_time += 1;
        //cout<<"cpu Ÿ��: "<<global_time<<"\n";
        
        //��, time quantum�� 1 �����Ҷ����� preemtion�� �˻��Ѵ�.
        vector< Process* >::iterator ptr;
        for (ptr = sleep.begin(); ptr != sleep.end(); ) {
            int result = (*ptr)->consume_bt_sleep(1);
            if (result == 1) {                      //��������
                push_queue(*ptr);
                is_stoped = true;
                ptr = sleep.erase(ptr);
            } else {
                ++ptr;
            }
        }

        //I/O ��û�� ������ ���� ���μ����� ����� Ż��
        if( is_stoped == true ) {
            return;
        }
    }
}

//q2���� �����ٸ����� ���μ����� preemption�Ǵ°� ���� q2������ ���μ����� burst time�� �����������̴�.
//
Process* scheduling_q2(void) {
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
    cout<<process->PID<<" �� �����մϴ�: ";
    //2��° ť���� �� ���μ������
    if( process->queue == 2) {
        //�����ų� I/O ��û�Ҷ����� �����Ѵ�
        while(1) {
            bool is_stoped = false;
            chart.push_back(make_pair(process->PID, global_time));
            global_time += 1;
            //cout<<"cpu Ÿ��: "<<global_time<<"\n";
            
            //��, time quantum�� 1 �����Ҷ����� preemtion�� �˻��Ѵ�.
            vector< Process* >::iterator ptr;
            for (ptr = sleep.begin(); ptr != sleep.end(); ) {
                int result = (*ptr)->consume_bt_sleep(1);
                if (result == 1) {                      //��������
                    push_queue(*ptr);
                    if ( (*ptr)->queue == 2 ) is_stoped = true;
                    ptr = sleep.erase(ptr);
                } else {
                    ++ptr;
                }
            }
            
            int consume_result = process->consume_bt(1);
            if (consume_result == 1) {                // ������� ��û�� -> sleep
                sleep.push_back(process);
                break;
            } else if (consume_result == 2) {         // ���μ����� �����
                //cout << process->PID<<" �� ���� Ȯ��.\n";
                break;
            }

            //I/O ��û�� ������ ���� ���μ����� �ְ�, �� ���μ����� burst time�� ���� ���μ������� ª�ٸ� �ݺ��� Ż��
            if( is_stoped == true ) {
                //cout<<"preemtion �߻�\n";
                //preemption�� �߻��ϸ� ���� �����ϴ� ���μ����� q3���� ����
                process->queue += 1;
                push_queue(process);
                //���ο� ���μ����� �̾�޾Ƽ� ����
                process = scheduling_q2();
            }
        }
        return;
    }
     
    //Gantt Chart �׸��� ���� ���
    chart.push_back(make_pair(process->PID, global_time));
    //�ð� ����
    //cout<<"����ð� "<<global_time<<"���� "<<time_q<<"��ŭ �����߽��ϴ�: ";
    global_time += time_q;

    vector< Process* >::iterator ptr;
    for (ptr = sleep.begin(); ptr != sleep.end();) {
        int result = (*ptr)->consume_bt_sleep(time_q);
        if (result == 1) {                      //��������
            push_queue(*ptr);
            ptr = sleep.erase(ptr);
        } else {
            ++ptr;
        }
    }

    int consume_result = process->consume_bt(time_q);
    if (consume_result == 0) {                // queue�� time_quantum�� ��� �Ҹ��� -> �켱���� 1 �÷��� �ٽ� ����
        //cout<<process->PID<<" �� �ð���"<<process->get_burst_time()<<" ���ҽ��ϴ�. �켱������ �� �ܰ� ����ϴ�.\n";
        process->queue += 1;
        push_queue(process);
    } else if (consume_result == 1) {         // ������� ��û�� -> sleep
        sleep.push_back(process);
    } else {                                   // ���μ����� �����
        cout << process->PID<<" �� ���� Ȯ��.\n";
    }
}

//��� ť(����ť0,1,2,3 & ����ť)���� ���μ����� ����� ���� Ȯ���ϸ� �ùķ��̼� ����.
void start_simulation() {
    cout<<"�ùķ��̼��� �����մϴ�.\n";
    while (true) {
        //q0���� ���μ��� �߰�
        if (q0.empty() == false && q0.front()->arr_t <= global_time ) {
            Process* running = scheduling_FCFS(0);
            cpu_working(running, 2);
            //cpu�� release�ϸ� �ٽ� q0���� ���μ��� Ž�� -> ����ť ������ �켱����
            continue;
        }

        //q1���� ���μ��� �߰�
        if (q1.empty() == false && q1.front()->arr_t <= global_time) {
            Process* running = scheduling_FCFS(1);
            //cout << running->PID << "�� cpu�� �����ϴ�, Ÿ�������� 6\n";
            cpu_working(running, 6);
            continue;
        }

        //q2���� ���μ��� �߰�
        if (q2.empty() == false && q2.front()->arr_t <= global_time) {
            //cout<<"2��° ť���� �߰�: ";
            //������ �����Ѵ�
            Process* running = scheduling_q2();
            cpu_working(running, 1);
            continue;
        }

        //q3���� ���μ��� �߰�
        if (q3.empty() == false && q3.front()->arr_t <= global_time) {
            Process* running = scheduling_FCFS(3);
            cpu_working(running, running->get_burst_time()); //���μ����� ���������� ���ؾ��� cpu �Ҵ�
            continue;
        }

        //����ť���� ���μ��� �߰�
        if ( sleep.empty() == false ) {
            // I/O burst time ��ٸ�
            wating_IO();
            continue;
        }

        //��� ���μ����� ����� ���� Ȯ���Ͽ� �ùķ��̼� ����.
        break;
    }
    return;
}

//��� ����
void print_result() {
    vector< pair<int, int> >::iterator ptr;
    int prev = -3;
    printf(" �ð� | ���μ���1 | ���μ���2 | ���μ���3 | ���μ���4 | ���μ���5\n");
    int i = 0;
    int pid = chart.front().first;
    for (ptr = chart.begin()+1; ptr != chart.end(); ++ptr) {
        for( ; i<(*ptr).second; i++ ) {
            int p1 = 32, p2 =32, p3 = 32, p4 = 32, p5 = 32;
            switch (pid) {
                case 1:
                    p1 = 64;
                    break;
                case 2:
                    p2 = 64;
                    break;
                case 3:
                    p3 = 64;
                    break;
                case 4:
                    p4 = 64;
                    break;
                default:
                    p5 = 64;
                    break;
            }
            printf("%4d %8c   %8c   %10c   %10c   %12c\n", i, p1, p2, p3, p4, p5);
        }
        pid = (*ptr).first;
    }
}

int main() {
    init_process();
    start_simulation();
    print_result();
}