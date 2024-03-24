# TODO: 디버그, 릴리스 룰이랑 헤더파일 의존성 부여 하기

#----------FLAG----------
CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98
DEBUGFLAGS = -g
DEPFLAGS = -MMD -MP

#----------OBJ----------
OBJS_MAND = $(addprefix objs/, $(notdir $(SRCS_MAND:.cpp=.o)))
OBJS_BONUS = $(addprefix objs/, $(notdir $(SRCS_BONUS:.cpp=.o)))
OBJ_DIR = objs

#----------DEP----------
DEPS = $(addprefix objs/, $(notdir $(SRCS_MAND:.cpp=.d)))

#----------BINARY----------
NAME = webserv

#----------SRCS----------
SRCS_MAND = main.cpp \
	WebServer.cpp \
	Http.cpp \
	parseUtils.cpp
SRCS_BONUS = srcs_bonus1 \
	srcs_bonus2 \
	srcs_bonus3

#----------VPATH----------
vpath %.cpp srcs
vpath %.hpp includes


all: $(NAME)

debug:
	CXXFLAGS += DEBUGFLAGS
	$(NAME)


$(NAME): $(OBJ_DIR) mandatory

$(OBJ_DIR):
	@mkdir objs

-include $(DEPS)

mandatory: $(OBJS_MAND)
	rm -f mandatory bonus $(NAME)
	$(CXX) $(CXXFLAGS) -o $(NAME) $(OBJS_MAND)
	touch $@
	$(info    __     __     ______     ______     ______     ______     ______     __   __  )
	$(info   /\ \  _ \ \   /\  ___\   /\  == \   /\  ___\   /\  ___\   /\  == \   /\ \ / /  )
	$(info   \ \ \/ ".\ \  \ \  __\   \ \  __<   \ \___  \  \ \  __\   \ \  __<   \ \ \'/   )
	$(info    \ \__/".~\_\  \ \_____\  \ \_____\  \/\_____\  \ \_____\  \ \_\ \_\  \ \__|   )
	$(info     \/_/   \/_/   \/_____/   \/_____/   \/_____/   \/_____/   \/_/ /_/   \/_/    )

bonus: $(OBJ_DIR) $(OBJS_BONUS)
	@rm -f mandatory bonus $(NAME)
	@$(CXX) $(CXXFLAGS) $(DEPFLAGS) -o $(NAME) $(OBJS_BONUS)
	@touch $@
	$(info    __     __     ______     ______     ______     ______     ______     __   __  )
	$(info   /\ \  _ \ \   /\  ___\   /\  == \   /\  ___\   /\  ___\   /\  == \   /\ \ / /  )
	$(info   \ \ \/ ".\ \  \ \  __\   \ \  __<   \ \___  \  \ \  __\   \ \  __<   \ \ \'/   )
	$(info    \ \__/".~\_\  \ \_____\  \ \_____\  \/\_____\  \ \_____\  \ \_\ \_\  \ \__|   )
	$(info     \/_/   \/_/   \/_____/   \/_____/   \/_____/   \/_____/   \/_/ /_/   \/_/    )

$(OBJ_DIR)/%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(DEPFLAGS) -c $< -o $@

fclean: clean
	@rm -f $(NAME)

clean:
	@rm -f $(OBJS_MAND)
	@rm -f $(OBJS_BONUS)
	@rm -f $(DEPS)
	@rm -f mandatory bonus
	@rm -rf $(OBJ_DIR)

re:
	make fclean
	make all

rebonus: fclean
	make bonus

.PHONY: all clean fclean re rebonus
