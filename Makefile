#----------FLAG----------
CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98 #-fsanitize=address -g
DEBUGFLAGS = -g
DEPFLAGS = -MMD -MP
ADDRESS = -fsanitize=address

#----------OBJ----------
OBJS_MAND = $(addprefix objs/, $(notdir $(SRCS_MAND:.cpp=.o)))
OBJS_BONUS = $(addprefix objs/, $(notdir $(SRCS_BONUS:.cpp=.o)))
OBJ_DIR = objs

#----------DEP----------
DEPS = $(addprefix objs/, $(notdir $(SRCS_MAND:.cpp=.d)))

#----------BINARY----------
NAME = webserv

#----------SRCS----------
# SRCS_MAND = request_test.cpp \
# 	convertUtils.cpp \
# 	parseUtils.cpp \
# 	Request.cpp \
# 	autoIndex.cpp \
# 	HttpRequest.cpp
SRCS_MAND = ConfigHandler.cpp \
	AConfParser.cpp \
	Server.cpp \
	Location.cpp \
	parseUtils.cpp \
	Response.cpp \
	Request.cpp \
	HttpHandler.cpp \
	convertUtils.cpp \
	autoIndex.cpp \
	StatusPage.cpp \
	WebServ.cpp \
	main.cpp 

SRCS_BONUS = srcs_bonus1 \
	srcs_bonus2 \
	srcs_bonus3
#----------VPATH----------
vpath %.cpp srcs
vpath %.hpp includes

all: $(NAME)

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
	@rm -f */*/youpla.bla

re:
	@make fclean
	@make all

rebonus: fclean
	@make bonus

.PHONY: all clean fclean re rebonus debug

debug:
	@make fclean
	@make all CXXFLAGS="$(CXXFLAGS) $(DEBUGFLAGS) $(ADDRESS)"
	