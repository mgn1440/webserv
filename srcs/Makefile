#----------FLAG----------
CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98 #-fsanitize=address -g
DEBUGFLAGS = -g
DEPFLAGS = -MMD -MP
ADDRESS = -fsanitize=address

#----------OBJ----------
OBJS_MAND =  $(notdir $(SRCS_MAND:.cpp=.o))
OBJS_BONUS = $(notdir $(SRCS_BONUS:.cpp=.o))

#----------DEP----------
DEPS = $(notdir $(SRCS_MAND:.cpp=.d))

#----------BINARY----------
NAME = webserv

#----------SRCS----------
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
	main_t.cpp 
all: $(NAME)

$(NAME): mandatory

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

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(DEPFLAGS) -c $< -o $@

fclean: clean
	@rm -f $(NAME)

clean:
	@rm -f $(OBJS_MAND)
	@rm -f $(OBJS_BONUS)
	@rm -f $(DEPS)
	@rm -f mandatory bonus

re:
	@make fclean
	@make all

rebonus: fclean
	@make bonus

.PHONY: all clean fclean re rebonus debug

debug:
	@make fclean
	@make all CXXFLAGS="$(CXXFLAGS) $(DEBUGFLAGS) $(ADDRESS)"
	