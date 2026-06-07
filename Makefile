NAME = ircserver
C=c++
FLAGS= -Wall -Wextra -Werror  -g -std=c++98
SRSC= ACommand..cpp Channel.cpp Client.cpp JoinCommand.cpp NickCommand.cpp Server.cpp main.cpp
HEADERS = ACommand.hpp Channel.hpp Client.hpp JoinCommand.hpp NickCommand.hpp Server.hpp
OBJS=$(SRSC:.cpp=.o)
all:$(NAME)
$(NAME):$(OBJS)
	$(C) $(FLAGS) $^ -o $(NAME)
%.o:%.cpp $(HEADERS)
	$(C) $(FLAGS) -c $< -o $@
clean:
	rm -f $(OBJS)
fclean: clean
	rm -f $(NAME)
re: fclean all