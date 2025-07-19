# Project compilation files and directories

NAME					=	./bin/MattDaemon

SRCS					=	main.cpp			\
							Server.cpp			\
							Daemon.cpp			\
							Tintin_reporter.cpp	

# 							utils.cpp			\
# 							Client.cpp

SRCSFD					=	srcs/
OBJSFD					=	objs/
HDR_INC					=	-I./includes
# # # # # # # # # # # # # # # # # # # # # # # # # # # # #

# Compilation
CFLAGS					=	-Wall -Wextra -Werror -std=c++17
# # # # # # # # # # # # # # # # # # # # # # # # # # # # #

# Colors
RED						=	\033[0;31m
GREEN					=	\033[0;32m
NONE					=	\033[0m
# # # # # # # # # # # # # # # # # # # # # # # # # # # # # 

all: project $(NAME)
	@echo "... project ready"

project:
	@echo "Checking project ..."

$(OBJSFD):
	@mkdir -p $@
	@echo "\t[ $(GREEN)✔$(NONE) ] $@ directory"

$(NAME): $(OBJSFD) $(SRCS:%.cpp=$(OBJSFD)%.o)
	@mkdir -p bin
	@clang++ $(CFLAGS) $(SRCS:%.cpp=$(OBJSFD)%.o) -o $@
	@echo "\t[ $(GREEN)✔$(NONE) ] $(NAME) executable"

$(OBJSFD)%.o: $(SRCSFD)%.cpp
	@clang++ $(CFLAGS) $(HDR_INC) -c $< -o $@
	@echo "\t[ $(GREEN)✔$(NONE) ] $@ object"

clean:
	@echo "Cleaning project ..."
	@/bin/rm -rf $(OBJSFD)
	@echo "\t[ $(RED)✗$(NONE) ] Objects directory"

fclean: clean
	@/bin/rm -rf bin
	@echo "\t[ $(RED)✗$(NONE) ] $(NAME) executable deleted"

re: fclean all

.PHONY: project all clean fclean re
