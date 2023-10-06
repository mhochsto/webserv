NAME :=	webserv

CPPFLAGS := -Wall -Werror -Wextra -std=c++98
DEPFLAGS = -MT $@ -MMD -MP -MF $*.Td
POSTCOMPILE = mv -f $*.Td $*.d && touch $@

CXX := c++

SRC :=	main.cpp		\
		Config.cpp		\
		utils.cpp		\
		Server.cpp		\
		Request.cpp		\
		Response.cpp	\
		CgiHandler.cpp

OBJ := $(SRC:%.cpp=%.o)

DEPFILES = $(SRC:%.cpp=%.d)

%.o: %.cpp
	$(CXX) $(CPPFLAGS) $(DEPFLAGS) -c $< -o $@

	@$(POSTCOMPILE)

all: $(NAME)

$(NAME): $(OBJ)
	@$(CXX) -o $(NAME) $(OBJ) -I.
	@echo "Done making"

$(DEPFILES): 

test: $(NAME)
	python3 client.py

clean:
	@rm -f $(OBJ)
	@rm -f $(DEPFILES)

fclean: clean
	@rm -f $(NAME)

re:	fclean $(NAME)

.PHONY:
	all clean fclean re

include $(wildcard $(DEPFILES))