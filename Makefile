# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: kkoujan <kkoujan@student.42.fr>            +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2026/06/26 09:10:30 by kkoujan           #+#    #+#              #
#    Updated: 2026/06/26 11:12:40 by kkoujan          ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME = ircserv
OBJ = ./server/Client.o ./server/Server.o ./command/Channel.o main.o ./command/Parser.o
HEADERS = ./server/Client.hpp ./server/Server.hpp ./command/Channel.hpp ./command/Command.hpp ./command/Parser.hpp
FLAGS = -Wall -Wextra -Werror -std=c++98
CC = c++


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
