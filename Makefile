# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: pamatya <pamatya@student.42heilbronn.de    +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: Invalid date        by                   #+#    #+#              #
#    Updated: 2025/04/08 12:16:11 by pamatya          ###   ########.fr        #
#                                                                              #
# **************************************************************************** #


# ----------------- Compiler Flags ----------------- #

CC			= 	gcc -g
# CC			= 	gcc -g -fsanitize=address
CFLAGS		= 	-Wall -Wextra -Werror
DEPFLAGS	= 	-MMD

# ----------------- Commands ----------------- #

RM			= 	rm -rf

# ----------------- Default Directories ----------------- #

# D_BIN		=	bin
D_INC		=	include
D_LIB		=	lib
D_OBJ		=	obj
D_PARSE		=	src_parse
D_EXEC		=	src_exe
D_BUILTIN	=	$(D_EXEC)/built-ins

# ----------------- Docker files ----------------- #

DK_FILE		=	Dockerfile
DK_COMP		=	docker-compose.yml

# Define the script name
DOCK_BUILD_SCRIPT = dock_build.sh

# Define the Docker image name
IMAGE_NAME = valgrind

# ----------------- Source, Object and Dependency files ----------------- #

SRC_MAIN	=	main.c
SRC_PARSE	= 	parser_1.c parser_2.c \
				tokenizer.c \
				secondary_prompt.c \
				syntax_validation.c \
				heredoc.c \
				expansions.c \
				tilde_expansion.c \
				remove_quotes.c \
				pipes.c \
				cmds_lst_utils.c \
				signal.c \
				utils_1.c utils_2.c utils_3.c utils_4.c
SRC_EXE		=	initiate.c \
				initializations.c \
				indexing.c \
				binaries.c \
				executions.c child_fns.c \
				redirections.c \
				lst_str_fns.c lst_str_fns_2.c \
				bi_cd.c \
				bi_echo.c \
				bi_env.c \
				bi_exit.c \
				bi_export.c \
				bi_pwd.c \
				bi_unset.c \
				environment.c \
				variables.c variables_local.c \
				env_utils.c \
				string_utils.c string_utils_2.c \
				error_handlers.c \
				tests.c

VPATH		+=	$(D_PARSE) $(D_EXEC) $(D_BUILTIN)

SRC			=	$(SRC_MAIN) $(SRC_PARSE) $(SRC_EXE)
OBJ 		+= 	$(addprefix $(D_OBJ)/, $(SRC:.c=.o))
DEP			= 	$(OBJ:.o=.d)

# ----------------- Target Binary ----------------- #

NAME		=	mini-bash

# ----------------- Headers Flag ----------------- #

HEADS_FLG	=	-I$(D_INC) -I$(LIBFT_HED) -I$(READLINE_HED)

# ----------------- Libraries ----------------- #

LIBS_FLG		=	$(LIBFT_FLG) $(READLINE_FLG)

# LIBFT
LIBFT			=	libft.a
LIBFT_DIR		=	$(D_LIB)															# library file directory
LIBFT_HED		=	$(D_LIB)/includes													# header directory
LIBFT_FLG		=	-L$(LIBFT_DIR) -l$(basename $(subst lib,,$(LIBFT)))					# library flag

# Readline for Linux
READLINE		=	libreadline.a
ifeq ($(USER), pranujamatya)
	READLINE_DIR	=	/opt/homebrew/opt/readline/lib
	READLINE_HED	=	/opt/homebrew/opt/readline/include
else
	READLINE_DIR	=	/usr/local/lib
	READLINE_HED	=	/usr/local/include
endif
READLINE_FLG	=	-L$(READLINE_DIR) -l$(basename $(subst lib,,$(READLINE)))

VALGRIND_OPTS	= 	--suppressions=ignore_readline.supp \
					--leak-check=full \
					--show-leak-kinds=all \
					--trace-children=yes \
					--track-fds=yes \
					-s \
# --child-silent-after-fork=yes
# Leak Sanitizer for leak check on Mac (run: make LEAK=1)
LEAKSAN			=	liblsan.dylib
LEAKSAN_DIR		=	/Users/dstinghe/LeakSanitizer
LEAKSAN_HED		= 	/Users/dstinghe/LeakSanitizer/include
LEAKSAN_FLG		=	-L$(LEAKSAN_DIR) -l$(basename $(subst lib,,$(LEAKSAN)))
		
ifeq ($(LEAK), 1)
	LIBS_FLG	+=	$(LEAKSAN_FLG)
endif


# ----------------- Color Codes ----------------- #

RESET 		= 	\033[0m
RED 		= 	\033[31m
GREEN 		= 	\033[32m
YELLOW 		= 	\033[33m
BLUE 		= 	\033[34m

# ----------------- Rules ----------------- #

all: $(NAME)
	@echo "$(GREEN)Compilation finished$(RESET)"

$(NAME): $(OBJ)
	@echo "Compiling mini-bash..."
	@make -sC $(D_LIB)
	@$(CC) $(CFLAGS) $(HEADS_FLG) $^ $(LIBS_FLG) -o $@ 

$(D_OBJ)/%.o: %.c
	@$(CC) $(CFLAGS) $(HEADS_FLG) $(DEPFLAGS) -c $< -o $@

-include $(DEP)

clean:
	@$(RM) $(OBJ) $(DEP)
	@make clean -sC $(D_LIB)
	@echo "$(YELLOW)Cleaned Objects & Dependencies$(RESET)"

fclean:
	@$(RM) $(OBJ) $(DEP) $(NAME)
	@make fclean -sC $(D_LIB)
	@echo "$(YELLOW)Cleaned All$(RESET)"

re: fclean all

valgrind: $(NAME)
	@valgrind $(VALGRIND_OPTS) ./$(NAME)

valgrind_n: $(NAME)
	@env -i valgrind $(VALGRIND_OPTS) ./$(NAME)

norm:
	@/bin/bash -c 'NORM="$$(norminette main.c src_parse/*.c src_exe/*.c src_exe/built-ins/*.c include/*.h)"; \
	IFS=$$'\''\n'\''; \
	for i in $$NORM; do \
		if [[ "$$j" =~ ^Error:.*$$ && ! "$$i" =~ ^Error:.*$$ ]]; then \
			read INPUT; \
		fi; \
		if [[ ! "$$i" =~ .*OK! ]]; then \
			if [[ "$$i" =~ .*Error!$$ ]]; then \
				echo -e "$(RED)$$i$(RESET)"; \
			else \
				echo "$$i"; \
			fi; \
		else \
			echo -e "$(GREEN)$$i$(RESET)"; \
		fi; \
		j="$$i"; \
	done'

format:
	@c_formatter_42 main.c $(D_PARSE)/*.c $(D_EXEC)/*.c $(D_BUILTIN)/*.c $(D_INC)/*.h

dock:
	@docker build -t $(IMAGE_NAME) .
	@docker-compose run --rm $(IMAGE_NAME)

.PHONY: all clean fclean re valgrind valgrind_n norm format dock

