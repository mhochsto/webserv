NAME :=	webserv

SRC_DIR := src
OBJ_DIR := obj
DEP_DIR := $(OBJ_DIR)/dep
INC_DIR := inc

CXXFLAGS := -Wall -Werror -Wextra -std=c++98
CPPFLAGS = -I$(INC_DIR)
DEPFLAGS = -MT $@ -MMD -MP -MF $(DEP_DIR)/$*.Td
POSTCOMPILE = mv -f $(DEP_DIR)/$*.Td $(DEP_DIR)/$*.d && touch $@

CXX := c++

SRC :=	main.cpp		\
		Config.cpp		\
		utils.cpp		\
		Server.cpp		\
		Request.cpp		\
		Response.cpp	\
		CgiHandler.cpp	

SRCS := $(addprefix $(SRC_DIR),$(SRC))

OBJ := $(SRC:%.cpp=$(OBJ_DIR)/%.o)

DEPFILES = $(SRC:%.cpp=$(DEP_DIR)/%.d)


$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(DEP_DIR)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(DEPFLAGS) -c $< -o $@ 

	@$(POSTCOMPILE)


all: $(NAME)

$(DEP_DIR):
	@mkdir -p $(DEP_DIR)


$(NAME): $(OBJ)
	$(CXX) -o $(NAME) $(OBJ)
	@echo "Done making"

$(DEPFILES): 

clean:
	@rm -rf $(OBJ_DIR)
	@echo "cleaned the o_files and the d_files"

fclean: clean
	@rm -f $(NAME)
	@echo "cleaned the executable"

re:	fclean $(NAME)

.PHONY:
	all clean fclean re

include $(wildcard $(DEPFILES))