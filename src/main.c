#include "main.h"

static void	print_table(void)
{
    double	ATT;
    double	AWT;

	ATT = 0;
	AWT = 0;
    printf("process table:\n");
    printf("+-------+-------------------+----------------+\n");
    printf("|  PID  |  Turnaround Time  |  Waiting Time  |\n");
    printf("+-------+-------------------+----------------+\n");
    for(int i=0; i<process_num; i++) {
        ATT += process_table[i][0];
        AWT += process_table[i][1];
        printf("| %5d | %17d | %14d |\n", i+1, process_table[i][0], process_table[i][1]);
    }
    printf("+-------+-------------------+----------------+\n");
    printf("\n");
    ATT /= process_num;
    AWT /= process_num;
    printf("Average Turnaround Time : %.2lf\nAverage Wating Time     : %.2lf\n", ATT, AWT);
}

int			main()
{
    if (set_simulation() == ERROR)
        return (0);
    if (start_simulation() == ERROR)
        return (0);
    print_table();
    delete_queue();
    return (0);
}
