#include <iostream>
#include <fstream>
#include <vector>

using namespace std;

class Process {
public:
    int PID;
    int queue;
    int arr_t;
    int cycle_num;
    int * seq_burst;

    Process() {

    }

    Process(int _PID, int _queue, int _arr, int _cycle) {
        PID = _PID;
        queue = _queue;
        arr_t = _arr;
        cycle_num = _cycle;        
    }

    void setBurst() {
        int size_arr = (cycle_num * 2) - 1 ;
        seq_burst = new int[size_arr];
    }

    ~Process() {
        delete []seq_burst;
    }
};

vector<Process> q0;
vector<Process> q1;
vector<Process> q2;
vector<Process> q3;

//input.txt ���Ϸ� �Է¹޾Ƽ� ���� queue�� ���μ��� �Ҵ�
void init_process() {
    ifstream input_file("input.txt");
    int s;
    string s1;

    if (input_file.is_open()) {
        input_file >> s;
        for(int i=0; i<s; i++) {
            Process new_process;
            cout << i <<"��° ���μ���\n";
            int pid, init_q, arr_t, cycle;
            int size_arr;
            input_file>>pid>>init_q>>arr_t>>cycle;
            cout<<pid<<" "<<init_q<<" "<<arr_t<<" "<<cycle<<"\n";
            new_process.PID = pid;
            new_process.queue = init_q;
            new_process.arr_t = arr_t;
            new_process.cycle_num = cycle;
            size_arr = (cycle * 2) - 1 ;
            new_process.setBurst();
            for(int j=0; j<size_arr; j++) {
                int tmp;
                input_file>>tmp;
                cout<<tmp<<" ";
                new_process.seq_burst[j] = tmp;
            }
            switch (init_q) {
            case 0:
                q0.push_back(new_process);
                break;
            case 1:
                q1.push_back(new_process);
                break;
            case 2:
                q2.push_back(new_process);
                break;
            default:
                q3.push_back(new_process);
                break;
            }
        }
    } else {
        cout << "������ ã�� �� �����ϴ�!" << endl;
    }
    return;
}

//q0 ~ q3���� ��� �� ��� �����Ѵ�.
void start_simulation() {
    bool is_empty = true;
    while(1) {
        //���࿡ q0�� ���μ����� �ִٸ�
        //������ �����Ѵ�
        //is_empty�� false�� �Ѵ�

        //���࿡ q1�� ���μ����� �ִٸ�
        //������ �����Ѵ�
        //is_empty�� false�� �Ѵ�

        //���࿡ q2�� ���μ����� �ִٸ�
        //������ �����Ѵ�
        //is_empty�� false�� �Ѵ�

        //���࿡ q3�� ���μ����� �ִٸ�
        //������ �����Ѵ�
        //is_empty�� false�� �Ѵ�

        //���࿡ is_empty�� true��� ������ Ż���Ѵ�
        if(is_empty == true) {
            break;
        }
    }
    return;
}

//��� ����
void print_result() {
    
}

int main() {
    init_process();
    start_simulation();
    print_result();
}