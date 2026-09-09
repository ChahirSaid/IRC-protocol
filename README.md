*This project has been created as part of the 42 curriculum by schahir, lserghin, kel-bahr.*

# IRC Protocol

An IRC server written in C++98 for the 42 `ft_irc` project. The server accepts multiple TCP clients, authenticates them with a password, manages channels and operators, and routes private and channel messages using a nonblocking `poll()` loop.

## Features

### Server

- Password-based client registration with `PASS`, `NICK`, and `USER`
- Nonblocking IPv4 TCP networking with `poll()`
- IRC command parsing with support for commands sent in separate or combined packets
- Channel creation and membership management with `JOIN` and `PART`
- Private and channel messaging with `PRIVMSG`
- Channel operators and moderation with `KICK`, `INVITE`, and `MODE`
- Channel topics with `TOPIC`
- Channel modes for invite-only access, topic restriction, keys, user limits, and operator privileges
- IRC numeric replies for the main registration, messaging, channel, and permission errors

### Bonus bot

The bonus target builds an ASCII-animal bot named `AsciiBot`. It responds to private messages containing:

- `help`
- A number from `1` to `9` for a specific animal
- `0` for a random animal

## Requirements

- Linux or another POSIX-compatible operating system
- A C++ compiler with C++98 support
- `make`
- An IRC client, such as `irssi`, `nc`, or another client that supports raw IRC connections

## Build

Build the server:

```bash
make
```

Build the bonus bot:

```bash
make bonus
```

Remove object files and binaries:

```bash
make fclean
```

## Run

Start the server with a port and connection password:

```bash
./ircserv <port> <password>
```

Example:

```bash
./ircserv 6667 secret
```

Start the bonus bot in another terminal:

```bash
./ircbot <port> <password>
```

Example:

```bash
./ircbot 6667 secret
```

Connect an IRC client to `127.0.0.1:6667`, then use the same server password. A minimal raw IRC registration sequence is:

```text
PASS secret
NICK client
USER client 0 * :Client Name
```

After registration, try:

```text
JOIN #general
PRIVMSG #general :hello
MODE #general +t
TOPIC #general :A channel topic
```

## Testing

The project was tested with real local TCP clients. The tested areas include:

- Strict C++98 compilation with `-Wall -Wextra -Werror`
- Registration and incorrect-password handling
- Nickname collisions and nickname changes
- Multiple commands in one network packet
- Channel creation, duplicate JOIN prevention, PART, and cleanup
- Private and channel `PRIVMSG`
- Invite-only channels, channel keys, and user limits
- Topic and operator permissions
- KICK and INVITE permissions
- Disconnect and reconnect handling
- Bonus bot authentication and command responses

## Resources

- RFC 1459: Internet Relay Chat Protocol
- RFC 2812: Internet Relay Chat: Client Protocol
- `man 2 socket`, `man 2 bind`, `man 2 listen`, `man 2 accept`, `man 2 poll`, `man 2 send`, and `man 2 recv`
- The 42 `ft_irc` project subject and evaluation requirements

## Team

| Login | Role |
| --- | --- |
| `schahir` | Server architecture, networking, and integration |
| `lserghin` | IRC command handling and channel protocol |
| `kel-bahr` | Testing, documentation, and bonus bot |

## Current Limitations

- `QUIT` is not implemented yet.
- `NOTICE` is not implemented yet.
- Some advanced `MODE` argument validation remains to be completed.
- Network partial-write and nonblocking error handling can be hardened further.

## License

This repository is an educational project created for the 42 curriculum.
