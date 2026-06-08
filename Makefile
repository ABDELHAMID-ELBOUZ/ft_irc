NAME = ircserver
C=c++
FLAGS= -Wall -Wextra -Werror  -g -std=c++98
SRSC= ACommand.cpp Channel.cpp Client.cpp JoinCommand.cpp NickCommand.cpp Server.cpp main.cpp PassCommand.cpp PrivmsgCommand.cpp UserCommand.cpp
HEADERS = ACommand.hpp Channel.hpp Client.hpp JoinCommand.hpp NickCommand.hpp Server.hpp PassCommand.hpp PrivmsgCommand.hpp UserCommand.hpp
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