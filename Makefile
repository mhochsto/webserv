NAME :=	webserv

CFLAGS := -Wall -Werror -Wextra -std=c++98 -gdwarf-4

CC := c++

SRC :=	main.cpp		\
		Server.cpp		\
		Request.cpp		\
		Config.cpp		\
		utils.cpp		\
		
		# CgiHandler.cpp
		# Response.cpp

OBJ := $(SRC:%.cpp=%.o)

%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

all: $(NAME)

$(NAME): $(OBJ)
	@$(CC) -o $(NAME) $(OBJ) -I.
	@echo "Done making"

test: $(NAME)
	python3 client.py

clean:
	@rm -f $(OBJ)

fclean: clean
	@rm -f $(NAME)

re:	fclean $(NAME)

.PHONY:
	all clean fclean re