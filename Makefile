NAME = ircserv

CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98 -Iinclude

SRC = \
	src/main.cpp \
	src/Server.cpp \
	src/Client.cpp \
	src/Channel.cpp \
	src/CommandParser.cpp \
	src/utils.cpp \
	src/commands/HandleNick.cpp \
	src/commands/HandleUser.cpp \
	src/commands/HandlePass.cpp \
	src/commands/HandleJoin.cpp \
	src/commands/HandlePrivmsg.cpp \
	src/commands/HandleKick.cpp \
	src/commands/HandleInvite.cpp \
	src/commands/HandleTopic.cpp \
	src/commands/HandleMode.cpp

OBJ = $(SRC:.cpp=.o)

all: $(NAME)

$(NAME): $(OBJ)
	$(CXX) $(CXXFLAGS) $(OBJ) -o $(NAME)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re