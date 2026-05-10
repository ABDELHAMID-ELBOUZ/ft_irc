NAME = ircserver
C=c++
FLAGS= -Wall -Wextra -Werror  -g -std=c++98
SRSC= main.cpp Server.cpp Client.cpp Channel.cpp
HEADERS = Server.hpp Client.hpp Channel.hpp
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