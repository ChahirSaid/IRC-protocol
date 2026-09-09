*This project has been created as part of the 42 curriculum by schahir, lserghin, kel-bahr.*

# IRC Protocol

## Description

This project is an Internet Relay Chat (IRC) server written in C++98, developed according to the 42 `ft_irc` subject requirements. The primary goal is to create a fully functional, non-blocking IRC server capable of handling multiple client connections simultaneously using a single `poll()` loop without forking. 

The server provides user authentication via password, nickname and user registration, channel lifecycle management, operator moderation privileges (`KICK`, `INVITE`, `TOPIC`, `MODE`), and message delivery for both private messaging and channel multicasting. It also includes an interactive bonus ASCII-art bot (`ircbot`).

## Features

### Server

- **Networking:** Non-blocking IPv4 TCP sockets multiplexed with a single `poll()` event loop.
- **Packet Handling:** Transparent packet aggregation supporting commands fragmented across TCP chunks or multiple commands per packet.
- **Registration & Authentication:** Client verification via `PASS`, `NICK`, and `USER` with standard welcome numerics (`001`–`004`).
- **Channel Operations:** Creation and dynamic destruction of channels with `JOIN` and `PART`.
- **Messaging:** Private one-to-one messaging and multi-client channel broadcasts via `PRIVMSG`.
- **Channel Operator Moderation:**
  - `KICK` to eject clients from channels.
  - `INVITE` to invite users to invite-only channels.
  - `TOPIC` to view or set channel topics.
  - `MODE` supporting channel modes `+i`/`-i` (invite-only), `+t`/`-t` (topic lock), `+k`/`-k` (channel key), `+o`/`-o` (operator status), and `+l`/`-l` (user limit).
- **Error Handling:** Standard IRC numeric error replies (`401`, `403`, `412`, `431`, `433`, `441`, `442`, `443`, `451`, `461`, `462`, `464`, `471`, `472`, `473`, `475`, `482`).

### Bonus Bot (`ircbot`)

An interactive ASCII-animal chatbot that authenticates with the server and responds to private messages with ASCII art:
- `help`: Explains available commands.
- `1` through `9`: Renders a specific ASCII animal.
- `0`: Renders a random animal.

## Instructions

### Requirements

- POSIX-compliant operating system (Linux or macOS)
- Standard C++ compiler (`c++` / `clang++` / `g++`) supporting the C++98 standard
- GNU `make`
- An IRC client (such as `irssi`, `hexchat`, `weechat`, or netcat `nc`)

### Compilation

Compile the mandatory IRC server:

```bash
make
```

Compile the bonus IRC bot:

```bash
make bonus
```

Clean object files:

```bash
make clean
```

Remove all compiled binaries and object files:

```bash
make fclean
```

Recompile the entire project:

```bash
make re
```

### Execution

#### 1. Starting the Server

The server requires two arguments: a port number and a connection password.

```bash
./ircserv <port> <password>
```

Example:

```bash
./ircserv 6667 secret
```

#### 2. Starting the Bonus Bot

In a separate terminal, launch the bot with the server's listening port and password:

```bash
./ircbot <port> <password>
```

Example:

```bash
./ircbot 6667 secret
```

#### 3. Connecting an IRC Client

Connect via raw TCP using netcat:

```bash
nc -C 127.0.0.1 6667
```

Register with the server:

```text
PASS secret
NICK alice
USER alice 0 * :Alice Smith
```

Join channels and interact:

```text
JOIN #general
PRIVMSG #general :Hello everyone!
MODE #general +t
TOPIC #general :Official discussion channel
```

## Testing

The implementation was validated using an automated test harness covering:
- Strict C++98 compilation flags (`-Wall -Wextra -Werror -std=c++98`).
- Zero memory leaks confirmed under Valgrind Memcheck.
- Packet fragmentation across multiple TCP packets and concatenated commands.
- Non-blocking multiplexing with concurrent client connections.
- Channel modes (`i`, `t`, `k`, `o`, `l`), operator permissions, and error responses.
- Bot authentication and response processing.

## Resources

### References

- [RFC 1459](https://datatracker.ietf.org/doc/html/rfc1459): Internet Relay Chat Protocol
- [RFC 2812](https://datatracker.ietf.org/doc/html/rfc2812): Internet Relay Chat: Client Protocol
- Linux Programmer's Manual: `socket(2)`, `bind(2)`, `listen(2)`, `accept(2)`, `poll(2)`, `send(2)`, `recv(2)`, `fcntl(2)`, `sigaction(2)`
- 42 `ft_irc` Project Subject (Version 10.0)

### AI Usage

In accordance with 42 curriculum guidelines regarding AI assistance:
- **Protocol Analysis & RFC Alignment:** AI was utilized to analyze RFC 1459 and RFC 2812 numeric specifications to identify expected reply structures and client behaviors.
- **Test Automation:** AI assisted in drafting the automated Python-based integration test harness ([test_irc.py](scratch/test_irc.py)) to test partial TCP packets, rapid disconnections, channel permission matrices, and concurrency edge cases.
- **Code Review & Auditing:** AI was used to inspect socket handling, audit memory safety, and cross-reference error codes with the evaluation criteria.
- **Documentation:** AI aided in refining the structure of the project documentation to ensure compliance with subject formatting requirements.

## Team

| Login | Role |
| --- | --- |
| `schahir` | Server architecture, networking, and integration |
| `lserghin` | IRC command handling and channel protocol |
| `kel-bahr` | Testing, documentation, and bonus bot |

## Current Limitations

- `QUIT` broadcast to channel members upon exit is pending full integration.
- `NOTICE` is not yet implemented.
- Nickname change broadcasting to shared channels remains to be finalized.

## License

This repository is an educational project created for the 42 curriculum.

