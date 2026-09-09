NAME = ircserv
BOT_NAME = ircbot

CXX = c++
CXXSTD = -std=c++98
CXXFLAGS = -Wall -Wextra -Werror
INCLUDE = -I./include -I./include/core -I./include/components -I./include/managers -I./include/systems
BOT_INCLUDE = -I./bot

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

BOT_SRCS = bot/main.cpp \
           bot/IRCBOT.cpp

OBJS = $(patsubst source/%.cpp, object/%.o, $(SRCS))
BOT_OBJS = $(patsubst bot/%.cpp, object/bot/%.o, $(BOT_SRCS))

all: $(NAME)

$(NAME): $(OBJS)
	$(CXX) $(CXXSTD) $(CXXFLAGS) $(INCLUDE) $(OBJS) -o $(NAME)

bonus: $(BOT_NAME)

$(BOT_NAME): $(BOT_OBJS)
	$(CXX) $(CXXSTD) $(CXXFLAGS) $(BOT_INCLUDE) $(BOT_OBJS) -o $(BOT_NAME)

object/%.o: source/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXSTD) $(CXXFLAGS) $(INCLUDE) -c $< -o $@

object/bot/%.o: bot/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXSTD) $(CXXFLAGS) $(BOT_INCLUDE) -c $< -o $@

clean:
	rm -rf object

fclean: clean
	rm -f $(NAME) $(BOT_NAME)

re: fclean all

.PHONY: all clean fclean re bonus