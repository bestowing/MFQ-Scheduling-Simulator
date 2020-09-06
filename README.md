### OS-project-1

# MFQ 스케줄링 시뮬레이터
*Multi-level Feedback Queue Scheduling Simulator*

본 문서는 '운영체제' 과목 수강중 배운 내용을 바탕으로, MFQ 스케줄링 기법을 구현한 시뮬레이터 구현 프로젝트를 설명합니다.

## 개요
본 프로그램은 텍스트 파일 *input.txt*에서 프로세스에 대한 정보를 받아, 정해진 스케줄링 기법으로 시뮬레이션합니다. 이후 시뮬레이션 결과를 콘솔창에 다음과 같은 순서로 출력합니다.

* **Gantt Chart**
* 프로세스별 **Turnaround Time**, **Wating Time**
* (전체 프로세스의) **평균 Turnaround Time, 평균 Wating Time**

## 입출력 규칙
### 입력 규칙
본 프로그램은 정해진 format에 맞춘 텍스트 파일 *input.txt*의 데이터를 읽습니다. 그 format의 예시는 아래와 같습니다.

| | | | | | | | | | | | |
---|---|---|---|---|---|---|---|----|---|---|---|
**4** |       |       |        |   |   |    |    | | | |
**1** | **0** | **0** | **1** | 8 |   |    |    | | | |
**2** | **0** | **1** | **4** | 3 | 5 | 9  | 11 | 3  | 12 | 10
**3** | **2** | **3** | **3** | 9 | 4 | 15 | 12 | 10 |    |    
**4** | **3** | **6** | **2** | 5 | 10 | 6 |    |    |    |    

* 첫째 행은 **프로세스의 갯수**를 나타냅니다.
* 둘째 행부터, 각 프로세스 정보가 주어집니다.
    * 첫번째 열은 해당 프로세스의 **ID**를 의미합니다.
    * 두번째 열은 해당 프로세스의 **최초 진입 ready queue**를 의미합니다.
    * 세번째 열은 해당 프로세스의 **arrival time**을 의미합니다.
    * 네번째 열은 해당 프로세스의 **Cycle 수**를 의미합니다.
        * 다섯번째 열부터, Cycle 수에 맞춰 **CPU burst time**, **I/O burst time**이 번갈아 제시됩니다.
        * 항상 CPU burst로 종료되어야 합니다.

### 출력 규칙
본 프로그램의 출력 예시는 아래와 같습니다.

![출력 예시](https://bestowing.github.io/portfolio/img/os_project1.348aa72a.png)

* **[Gantt chart]** 프로세스가 실행되는 흐름을 한 눈에 확인할 수 있습니다.
* **[Process table]** 프로세스별 TT와 WT를 확인할 수 있습니다.
* 프로세스 전체 **평균 TT와 WT**을 확인할 수 있습니다.

## 세부사항
* 이 MFQ는 4개의 ready queue로 구성되어 있습니다: {Q<sub>0</sub>, Q<sub>1</sub>, Q<sub>2</sub>, Q<sub>3</sub>}
* 각 ready queue의 스케줄링 기법은 아래와 같습니다:

| ready queue | scheduling method |
|---|---|
| Q<sub>0</sub> | RR(Round-Robin), time quatum = 2 |
| Q<sub>1</sub> | RR(Round-Robin), time quatum = 6 |
| Q<sub>2</sub> | SRTN(Shortest-Remaining-Time-Next) |
| Q<sub>3</sub> | FCFS(First-Come-First-Serve) |

* 우선순위는 Q<sub>0</sub> > Q<sub>1</sub> > Q<sub>2</sub> > Q<sub>3</sub> 순서입니다.
* Q<sub>i</sub>에서 스케줄 받아 실행된 프로세스가 주어진 time quantum을 모두 소모한 경우, Q<sub>i+1</sub>로 진입합니다.
* Q<sub>i</sub>에서 스케줄 받아 실행된 프로세스가 I/O burst에 진입한 경우, Wake up 할때 Q<sub>i-1</sub>로 진입합니다.
* Q<sub>2</sub>의 경우, preemption이 발생할 수 있습니다. 단, 오직 Q<sub>2</sub>로 진입하는 프로세스에 한정하여 발생하며, 다른 ready queue의 프로세스는 고려하지 않습니다.
    * 예시: Q<sub>2</sub>에 있던 P<sub>1</sub>이 스케줄링되어 실행중이라고 가정함.
    * 만약 P<sub>2</sub>이 Wake up 하여 Q<sub>2</sub>로 진입하는 경우, preemption 여부를 확인하기 위해 P<sub>1</sub>과 P<sub>2</sub>의 burst time을 비교해야 한다.
    * 그러나, 같은 시간에 Wake up한 P<sub>3</sub>가 Q<sub>1</sub>으로 진입하는 경우에는 preemption 여부를 확인하지 않는다.
* Q<sub>2</sub>를 제외한 어떠한 ready queue에서도 preemption을 허용하지 않습니다.
* Burst time estimation이 없습니다.
    * 사용자는 프로그램을 실행할때 프로세스별 Burst time을 주어진 format에 맞추어 제공해야 합니다.
    * 사용자는 주어진 프로세스별 최초 진입 Ready queue를 주어진 format에 맞추어 제공해야 합니다.
    * 자세한 사항은 [입력 규칙](#입력-규칙)을 참고하세요.

## 프로그램 설계

프로그램 구조에 대해 설명합니다. 파일이 기능별로 분할되어 있으며, 헤더파일을 포함해 총 4개 파일이 있습니다:
``main.h, main.c, setter.c, simulator.c``

### 개요

| 소스 파일 | 함수 | 설명 |
|:---:|:---:|:---|
| main.h      | | 필요한 정적 라이브러리를 포함하고, 구조체를 정의하는 헤더파일입니다. |
| main.c      | **main**         | 프로그램 실행 시작지점인 main 함수입니다.                               |
| setter.c    | **set_simulation** | 시뮬레이션을 위해 파일을 읽어오고 필요한 자원을 세팅하는 함수입니다.    |
|             | init_queue       | 시뮬레이션을 위해 필요한 queue에 메모리를 동적할당하는 함수입니다.      |
|             | set_processes    | 파일에서 읽어온 정보를 프로세스에 넣어주는 함수입니다.                 |
|             | init_process     | 프로세스에 메모리를 동적할당하는 함수입니다.                            |
| simulator.c | **start_simulation** | 시뮬레이션의 핵심 과정을 실행하는 함수입니다.                           |
|             | cpu_running      | 프로세스 P<sub>i</sub>를 실행하여 burst time을 1 감소시키는 함수입니다.         |
|             | push_queue       | 프로세스 P<sub>i</sub>를 적절한 ready queue에 푸시하는 함수입니다.              |
|             | io_check         | I/O 요청이 종료된 프로세스를 wake up하는 함수입니다.                    |
|             | arrival_check    | 프로세스 P<sub>i</sub>의 arrival 여부를 확인하는 함수입니다.                    |
|             | burst_check      | 프로세스 P<sub>i</sub>의 burst time을 1 감소된 후의 상태를 확인하는 함수입니다. |
|             | fcfs             | ready queue Q<sub>0</sub>, Q<sub>1</sub>, Q<sub>3</sub>에서 프로세스 P<sub>i</sub>를 스케줄링하는 함수입니다. |
|             | srtn             | ready queue Q<sub>2</sub>에서 프로세스 P<sub>i</sub>를 스케줄링하는 함수입니다. |
|             | preemtion        | ready queue Q<sub>2</sub>에서 스케줄링한 프로세스가 실행중일때, preemtion 발생 여부를 확인하는 함수입니다. |
|             | sleep_check      | sleep queue가 비어있는지 확인하는 함수입니다. |
|             | sleep            | I/O 요청 프로세스를 sleep 시키는 함수입니다. |
|             | scheduling       | 우선 순위를 고려하여 ready queue에서 프로세스 P<sub>i</sub>를 스케줄링하는 함수입니다. |
|             | get_burst_time   | 프로세스 P<sub>i</sub>의 남은 burst time을 반환하는 함수입니다. |
|             | delete_process   | 프로세스 P<sub>i</sub>에 동적할당받은 메모리 bytes를 반환하는 함수입니다. |
|             | delete_queue     | 시뮬레이션을 위해 필요한 queue에 동적할당받은 메모리 bytes를 반환하는 함수입니다. |

### main.h

메인 헤더에 시뮬레이션에 필요한 정적 라이브러리와 사용자 정의 구조체가 명시되어있습니다.

~~~ c
#ifndef __MAIN_H
# define __MAIN_H

// 표준 출력과 파일 입력
#include <stdio.h>  
// 메모리 동적 할당
#include <stdlib.h> 

// 프로세스의 정보를 담는 구조체
typedef struct Process {          
    int PID; 
    int queue;  
    int arr_t;   
    int cycle_num;
    int cycle_index;
    int cycle_total;
    int* seq_burst;    
} Process;

// ready queue를 구현하는 연결 리스트 구조체
typedef struct Node {
    struct Node* next;  
    Process* data;    
} Node;

#endif
~~~

### main.c

본 프로그램은 main 함수에서 다음과 같은 절차를 거쳐 실행됩니다.

1. 파일에서 프로세스 정보를 가져온다.
1. 시뮬레이션을 실시한다.
1. 결과를 출력한다.
1. 사용한 메모리 자원을 반납하고 종료한다.

~~~ c
int main(int argc, char *argv[]) {
    if (set_simulation() == -1)   // 파일에서 프로세스 정보를 가져온다.
        return (0); // 예외 처리
    if (start_simulation() == -1) // 시뮬레이션을 실시한다.
        return (0); // 예외 처리
    print_table();                // 결과를 출력하고 종료한다.
    delete_queue();               // 사용한 메모리 자원을 반납한다.
    return (0);
}
~~~

### setter.c

set_simulation() 함수에서 파일을 읽고, 프로세스 구조체에 메모리를 동적 할당하여 정보를 입력합니다.
파일 입력과 메모리 동적 할당 과정에서 생길 수 있는 예외가 각 code number와 함께 처리되어 있습니다.

~~~ c
int set_simulation(void) {
    FILE *file;

    file = fopen("input.txt", "r");
    if (file == NULL)
    {
        printf("Error code 01: failed to find \"input.txt\" file.\n");
        return (-1);
    }
    if (fscanf(file, "%d", &process_num) == -1)
    {
        printf("Error code 02: failed to read \"input.txt\" file.\n");
        return (-1);
    }
    if (init_queue() == -1)
    {
        printf("Error code 03: failed to allocate memory bytes.\n");
        return (-1);
    }
    if (set_processes(file) == -1)
    {
        printf("Error code 04: failed to read \"input.txt\" file or allocate memory bytes.\n");
        return (-1);
    }
    return (0);
}
~~~

자세한 코드는 [레포지토리](https://github.com/bestowing/OS-project1/blob/master/setter.c)를 참고하세요.

### simulator.c

start_simulation() 함수에서 시뮬레이션 핵심 과정이 진행됩니다.

1. job_queue에 있는 프로세스가 도착할 시간이 되었는지 확인하고 ready queue로 푸시합니다.
1. I/O를 요청한 프로세스가 있다면, I/O burst time을 확인하고 ready queue로 푸시합니다.
1. 현재 실행중인 프로세스가 있는지 확인합니다.
    * 실행중인 프로세스가 없다면, 새로운 프로세스를 스케줄링합니다.
        * 새로운 프로세스를 스케줄링하는데 실패했고, 남아있는 프로세스가 없다면, 시뮬레이션을 종료합니다.
        * 새로운 프로세스를 스케줄링하는데 실패했고, I/O 요청을 대기중인 프로세스가 있다면, 대기합니다.
    * 실행중인 프로세스가 있다면, Q<sub>2</sub>에서 스케줄링받아 실행중인 프로세스인지 확인합니다.
        * preemption 발생 여부를 확인하고 프로세스를 교체합니다.
1. 글로벌 시간을 1 증가시킵니다.
1. 만약 I/O 요청을 대기중인 프로세스만 남았다면, 대기합니다.
1. 실행중인 프로세스가 할당받은 시간을 1 소모합니다.
    * 만약 할당받은 시간을 모두 소모했음에도 burst time이 남았다면, 적절한 ready queue로 푸시합니다.
    * I/O를 요청했다면, sleep queue로 푸시합니다.
    * 프로세스가 할당받은 시간을 모두 소모했고, burst time이 남아있지 않다면, 해당 프로세스를 종료합니다.
    * 이외의 경우, 계속 실행합니다.

## 테스트

다양한 입력을 통해 프로그램 실행 결과를 테스트합니다.

### 1. 모든 프로세스가 I/O 요청을 하지 않는 경우

입력은 아래와 같습니다: input.txt

| | | | | | | | || | |
---|---|---|---|---|---|---|---|---|---|---
| 4 | | | | | | | || | |
| 1 | 0 | 0 |1 |8 | | | || | |
| 2 | 0 | 1 |1 |4 | | | || | |
| 3 | 2 | 3 |1 |9 | | || | | |
| 4 | 3 | 6 |1 |5 | | | || | |

프로세스는 총 4개로, 모든 프로세스가 I/O 요청없이 cycle 1회만에 종료됩니다.

출력은 아래와 같습니다:

![result1](https://user-images.githubusercontent.com/59321616/92317186-2f902780-f039-11ea-91b5-cadd29e2b1ed.png)

### 2. I/O요청을 하는 프로세스와 그렇지 않은 프로세스가 모두 있는 경우

입력은 아래와 같습니다: input.txt

| | | | | | | | || | |
---|---|---|---|---|---|---|---|---|---|---
| 4 |   |   |   |   |   |   |  |  |  |  |
| 1 | 0 | 0 | 1 | 8 |   |   |  |  |  |  |
| 2 | 0 | 1 | 1 | 4 | 5 | 9 |11| 3|12|10|
| 3 | 2 | 3 | 1 | 9 | 4 |15 |12|10|  |  |
| 4 | 3 | 6 | 1 | 5 | 10| 6 |  |  |  |  |

프로세스는 총 4개로, 프로세스 P<sub>1</sub>은 I/O 요청없이 cycle 1회만에 종료됩니다. 나머지 프로세스는 모두 I/O 시스템 호출을 한 번 이상 요청합니다.

출력은 아래와 같습니다:

![result2](https://user-images.githubusercontent.com/59321616/92317187-30c15480-f039-11ea-817e-03f5c8cdfdf1.png)
![result2_2](https://user-images.githubusercontent.com/59321616/92317188-3159eb00-f039-11ea-8bdd-1eebf16459b4.png)

* time 53 ~ 57의 *wating*은 ready_queue에 남은 프로세스가 없어 cpu가 sleep 상태에 있는 프로세스의 wake up을 기다리고 있음을 의미합니다.

### 3. ready Q<sub>2</sub>에서 preemption이 발생하는 경우

입력은 아래와 같습니다: input.txt

| | | | | | | | || | |
---|---|---|---|---|---|---|---|---|---|---
| 2 | | | | | |||||
|1| 2| 0| 1| 4| |||||
|2| 2| 1| 1| 1| |||||

프로세스는 총 2개로, ready Q<sub>2</sub>에 먼저 도착한 P<sub>1</sub>이 실행중일 때, burst time이 더 짧은 P<sub>2</sub>가 Q<sub>2</sub>에 도착하여 preemtion이 발생하는 경우를 시뮬레이션합니다.

출력은 아래와 같습니다:

![result3](https://user-images.githubusercontent.com/59321616/92317189-3159eb00-f039-11ea-95af-1ba4895d5b4a.png)

* 주의: preemtion은 오직 ready Q<sub>2</sub>에 들어온 프로세스에 한정하여 발생 가능합니다. P<sub>2</sub>가 Q<sub>1</sub>으로 도착한다면, 시뮬레이션 결과는 아래와 같이 preemption이 발생하지 않는 결과가 나올 것입니다. [세부 사항](#세부사항)을 참고하세요.

![result3_2](https://user-images.githubusercontent.com/59321616/92317190-31f28180-f039-11ea-96f0-8b5a4448abc1.png)

## 에러 코드별 해결 방법

* **Error code 01**: input.txt 파일을 open하는데 실패했을때 발생합니다. 파일을 프로그램 실행파일과 같은 위치에 두었는지 확인하세요.

* **Error code 02**: input.txt 파일에서 값을 읽는데 실패했을때 발생합니다. [입력 규칙](#입력-규칙)에 맞게 파일을 작성하세요.

* **Error code 03**: 시뮬레이션에 필요한 queue 포인터 동적 메모리 할당에 실패했을때 발생합니다. 실행 환경의 메모리 공간이 부족하지 않은지 확인하세요.

* **Error code 04**: input.txt 파일에서 값을 읽는데 실패하거나, 프로세스 포인터 동적 메모리 할당에 실패했을때 발생합니다. [입력 규칙](#입력-규칙)에 맞게 파일을 작성하거나, 실행 환경의 메모리 공간이 부족하지 않은지 확인하세요.

* **Error code 05**: 시뮬레이션에 필요한 Node 포인터 동적 메모리 할당에 실패했을때 발생합니다. 실행 환경의 메모리 공간이 부족하지 않은지 확인하세요.