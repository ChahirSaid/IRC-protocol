NAME = ircserv
BONUS_NAME = ircbot

CXX = c++
CXXSTD = -std=c++98
CXXFLAGS = -Wall -Wextra -Werror
INCLUDE = -I./include -I./include/core -I./include/components -I./include/managers -I./include/systems
BONUS_INCLUDE = -I./bonus

SRCS = source/main.cpp \
       source/core/Server.cpp \
       source/managers/LogManager.cpp \
       source/managers/SignalManager.cpp \
       source/managers/EntityManager.cpp \
       source/systems/NetworkIOSystem.cpp \
       source/systems/ParserSystem.cpp \
       source/systems/CommandSystem.cpp \
       source/systems/handlers/RegistrationHandlers.cpp \
       source/systems/handlers/MessagingHandlers.cpp \
       source/systems/handlers/ChannelHandlers.cpp \
       source/systems/handlers/ModeHandlers.cpp

BONUS_SRCS = bonus/main.cpp \
           bonus/IRCBOT.cpp

OBJS = $(patsubst source/%.cpp, object/%.o, $(SRCS))
BONUS_OBJS = $(patsubst bonus/%.cpp, object/bonus/%.o, $(BONUS_SRCS))

all: $(NAME)

$(NAME): $(OBJS)
	$(CXX) $(CXXSTD) $(CXXFLAGS) $(INCLUDE) $(OBJS) -o $(NAME)

bonus: $(BONUS_NAME)

$(BONUS_NAME): $(BONUS_OBJS)
	$(CXX) $(CXXSTD) $(CXXFLAGS) $(BONUS_INCLUDE) $(BONUS_OBJS) -o $(BONUS_NAME)

object/%.o: source/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXSTD) $(CXXFLAGS) $(INCLUDE) -c $< -o $@

object/bonus/%.o: bonus/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXSTD) $(CXXFLAGS) $(BONUS_INCLUDE) -c $< -o $@

clean:
	rm -rf object

fclean: clean
	rm -f $(NAME) $(BONUS_NAME)

re: fclean all

.PHONY: all clean fclean re bonus