NAME		=	mfq-simulator
CC			=	gcc
CFLAGS		=	-Wall -Wextra -Werror

SRC_NAME	=	main.c \
				setter.c \
				simulator.c
SRC_PATH	=	./src
SRCS		=	$(addprefix $(SRC_PATH)/, $(SRC_NAME))

OBJ_NAME	=	$(SRC_NAME:.c=.o)
OBJ_PATH	=	./obj
OBJS		=	$(addprefix $(OBJ_PATH)/, $(OBJ_NAME))

INCLUDES	=	-I./include

all : $(NAME)

$(NAME) : $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) $(INCLUDES) -o $(NAME)

$(OBJ_PATH)/%.o: $(SRC_PATH)/%.c
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ -c $<

clean :
	rm -f $(OBJS)

fclean : clean
	rm -f $(NAME)

re : fclean all

.PHONY : all clean fclean re