#ifndef __MAIN_H
# define __MAIN_H

# include <stdio.h>
# include <stdlib.h>

# define ERROR		-1

/*
**	struct define
*/
typedef struct		t_process
{
    int				PID;
    int				queue;
    int				arr_t;
    int				cycle_num;
    int				cycle_index;
    int				cycle_total;
    int				*seq_burst;
}					t_process;

typedef struct		t_node
{
    struct t_node	*next;
    t_process		*data;
}					t_node;

/*
**	global variables
*/
t_process			**job_queue;			// processes before arriving ready queue
t_node				*ready_queue0;			// Q0, RR(time quantum = 2)
t_node				*ready_queue1;			// Q1, RR(time quantum = 6)
t_process			**ready_queue2;			// Q2, SRTN
t_node				*ready_queue3;			// Q3, FCFS
t_process			**sleep_queue;			// processes requesting I/O system call
int					**process_table;		// result of the simulation
int					process_num;

/*
**	setter.c
*/
int 				set_simulation(void);

/*
**	simulator.c
*/
int					start_simulation(void);
void				delete_queue(void);

#endif
