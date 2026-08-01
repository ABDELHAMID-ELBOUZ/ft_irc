NAME = ircserv

CC = c++
FLAGS = -Wall -Wextra -Werror -std=c++98

OBJ = ./server/Client.o ./server/Server.o main.o ./command/Parser.o
HEADERS = ./server/Client.hpp ./server/Server.hpp ./command/Command.hpp ./command/Parser.hpp

all : $(NAME)
	
$(NAME) : $(OBJ)
	$(CC) $(FLAGS) $(OBJ) -o $(NAME)

%.o: %.cpp $(HEADERS)
	$(CC) $(FLAGS) -c $< -o $@

clean :
	rm -f $(OBJ)

fclean : clean
	rm -f $(NAME)

re : fclean all
