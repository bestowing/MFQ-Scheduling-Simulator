#ifndef __MAIN_H
# define __MAIN_H

# include <stdio.h>
# include <stdlib.h>

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

#endif
