CXX = cc
CXXFLAGS = -Wall -Wextra -Werror -std=c++98
OBJS_MAND = $(addprefix objs/, $(notdir $(SRCS_MAND:.c=.o)))
OBJS_BONUS = $(addprefix objs/, $(notdir $(SRCS_BONUS:.c=.o)))
OBJ_DIR = objs
NAME = webserv
SRCS_MAND = srcs1 \
	srcs2 \
	srcs3 \
SRCS_BONUS = srcs_bonus1 \
	srcs_bonus2 \
	srcs_bonus3

vpath %.c srcs
vpath %.h includes

all: $(NAME)

$(NAME): $(OBJ_DIR) mandatory

$(OBJ_DIR):
	@mkdir objs

mandatory: $(OBJS_MAND)
	rm -f mandatory bonus $(NAME)
	$(CXX) $(CXXFLAGS) -o $(NAME) $(OBJS_MAND)
	touch $@
	$(info  __     __     ______     ______     ______     ______     ______     __   __  )
	$(info /\ \  _ \ \   /\  ___\   /\  == \   /\  ___\   /\  ___\   /\  == \   /\ \ / /  )
	$(info \ \ \/ ".\ \  \ \  __\   \ \  __<   \ \___  \  \ \  __\   \ \  __<   \ \ \'/   )
	$(info  \ \__/".~\_\  \ \_____\  \ \_____\  \/\_____\  \ \_____\  \ \_\ \_\  \ \__|   )
	$(info   \/_/   \/_/   \/_____/   \/_____/   \/_____/   \/_____/   \/_/ /_/   \/_/    )

bonus: $(OBJ_DIR) $(OBJS_BONUS)
	@rm -f mandatory bonus $(NAME)
	@$(CXX) $(CXXFLAGS) -o $(NAME) $(OBJS_BONUS)
	@touch $@
	$(info  __     __     ______     ______     ______     ______     ______     __   __  )
	$(info /\ \  _ \ \   /\  ___\   /\  == \   /\  ___\   /\  ___\   /\  == \   /\ \ / /  )
	$(info \ \ \/ ".\ \  \ \  __\   \ \  __<   \ \___  \  \ \  __\   \ \  __<   \ \ \'/   )
	$(info  \ \__/".~\_\  \ \_____\  \ \_____\  \/\_____\  \ \_____\  \ \_\ \_\  \ \__|   )
	$(info   \/_/   \/_/   \/_____/   \/_____/   \/_____/   \/_____/   \/_/ /_/   \/_/    )

$(OBJ_DIR)/%.o: mandatory/%.c
	$(CXX) $(CXXFLAGS) -c $< -o $@ 

$(OBJ_DIR)/%.o: bonus/%.c
	$(CXX) $(CXXFLAGS) -c $< -o $@ 

fclean: clean
	@rm -f $(NAME)

clean:
	@rm -f $(OBJS_MAND)
	@rm -f $(OBJS_BONUS)
	@rm -f mandatory bonus
	@make -C libft fclean
	@make -C mlx clean
	@rm -rf $(OBJ_DIR)

re:
	make fclean
	make all

rebonus: fclean
	make bonus

.PHONY: all clean fclean re rebonus
