NAME :=	webserv

CPPFLAGS := -Wall -Werror -Wextra -std=c++98 -g
DEPFLAGS = -MT $@ -MMD -MP -MF $*.Td
POSTCOMPILE = mv -f $*.Td $*.d && touch $@

CXX := c++

SRC_DIR := /src
OBJ_DIR := /obj
DEP_DIR := $(OBJ)/dep
INC_DIR := /inc

SRC :=	main.cpp		\
		Config.cpp		\
		utils.cpp		\
		Server.cpp		\
		Request.cpp		\
		Response.cpp	\
		CgiHandler.cpp	

SRCS := $(addprefix $(SRC_DIR),$(SRC))

OBJ := $(SRC:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)

DEPFILES = $(SRC:%.cpp=/DEP_DIR/%.d)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CPPFLAGS) $(DEPFLAGS) -c $< -o $@

	@$(POSTCOMPILE)

all: $(NAME)

$(NAME): $(OBJ)
	@mkdir -p $(DEP_DIR)
	$(CXX) -o $(NAME) $(OBJ) -I$(INC_DIR)
	@echo "Done making"

$(DEPFILES): 

clean:
	@rm -f $(OBJ)
	@rm -f $(DEPFILES)
	@echo "cleaned the o_files and the d_files"

fclean: clean
	@rm -f $(NAME)
	@echo "cleaned the executable"

re:	fclean $(NAME)

.PHONY:
	all clean fclean re

include $(wildcard $(DEPFILES))