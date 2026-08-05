*This project has been created as part of the 42 curriculum by kkoujan and aelbouz and ynini.*

# ft_irc — Internet Relay Chat Server

## Description

**ft_irc** is a custom IRC (Internet Relay Chat) server built from scratch in **C++98**. It handles multiple simultaneous client connections using a single non-blocking I/O loop driven by `poll()`. The server implements a subset of the IRC protocol as defined in [RFC 1459](https://datatracker.ietf.org/doc/html/rfc1459), allowing users to authenticate, set nicknames, join channels, exchange private and channel messages, and manage channel privileges.

This project was developed to deepen our understanding of low-level network programming, TCP/IP socket communication, and event-driven server architecture without relying on threads or forking.

## Features

- **Multi-client support**: Handles numerous simultaneous connections via a single `poll()` loop.
- **Non-blocking I/O**: All sockets (listening and client) operate in non-blocking mode (`O_NONBLOCK`).
- **Authentication**: Password-protected server access via `PASS`.
- **User registration**: `NICK` and `USER` handshake with proper welcome replies (`001`–`004`).
- **Channels**: Create and join channels with `JOIN`. Channels support multiple users.
- **Messaging**: `PRIVMSG` and `NOTICE` for both channel broadcasts and direct user-to-user messages.
- **Channel operators**: The channel creator automatically receives operator (`+o`) status.
- **Operator commands**:
  - `KICK` — Eject a client from a channel.
  - `INVITE` — Invite a client to an invite-only channel.
  - `TOPIC` — View or change the channel topic (restricted by `+t`).
  - `MODE` — Toggle channel modes: `i` (invite-only), `t` (topic restriction), `k` (key/password), `o` (operator), `l` (user limit).
- **Graceful disconnects**: `QUIT` broadcasts departure messages to shared channels.
- **Partial command handling**: Aggregates fragmented TCP packets before parsing commands (tested with `nc` partial sends).

## Instructions

### Compilation

A `Makefile` is provided at the root of the repository. It compiles the project with the following flags:

```bash
make
```

This produces the `ircserv` binary. Available Makefile rules:
- `make` or `make all` — Compile the server.
- `make clean` — Remove object files.
- `make fclean` — Remove object files and the binary.
- `make re` — Rebuild from scratch.

### Execution

Run the server with a port number and a connection password:

```bash
./ircserv <port> <password>
```

**Example:**
```bash
./ircserv 6667 mysecretpass
```

- `<port>`: The TCP port on which the server will listen for incoming IRC connections (must be between 1024 and 65535).
- `<password>`: The password required by any IRC client attempting to connect.

### Connecting with a Client

You can test the server using any standard IRC client (e.g., **irssi**, **weechat**, **hexchat**) or via netcat:

```bash
nc -C 127.0.0.1 6667
```

Then register manually:
```irc
PASS mysecretpass
NICK mynickname
USER myusername 0 * :My Real Name
```

### Supported Commands

| Command | Description |
|---------|-------------|
| `PASS <password>` | Authenticate with the server password. |
| `NICK <nickname>` | Set or change your nickname. |
| `USER <user> 0 * :<realname>` | Set username and real name. |
| `JOIN <#channel>[,<#channel2>] [<key>]` | Join one or more channels. |
| `PRIVMSG <target> :<message>` | Send a message to a channel or user. |
| `NOTICE <target> :<message>` | Send a notice (no error replies generated). |
| `KICK <#channel> <nick> [:reason]` | Remove a user from a channel (operator only). |
| `INVITE <nick> <#channel>` | Invite a user to a channel. |
| `TOPIC <#channel> [:new topic]` | View or set the channel topic. |
| `MODE <#channel> [+/-modes] [params]` | View or change channel modes. |
| `QUIT [:reason]` | Disconnect from the server. |

## Architecture Overview

The server is built around an event loop using `poll()`:

1. **Socket Setup**: A listening socket is created, bound to the specified port, and set to non-blocking mode.
2. **Event Loop**: `poll()` monitors all file descriptors (listening socket + client sockets) for read/write readiness.
3. **New Connections**: `accept()` creates new client sockets, which are added to the `pollfd` array.
4. **Command Parsing**: Incoming data is buffered per-client. Complete lines (delimited by `\r\n`) are parsed into `Command` structures.
5. **Execution**: Commands are dispatched to dedicated handlers (`handleNick`, `handleJoin`, etc.).
6. **Replies**: Responses are queued in per-client output buffers. `POLLOUT` is enabled so `send()` flushes data when the kernel is ready.

Key design choices:
- **Single-threaded, non-blocking**: No `fork()`, no threads. All I/O multiplexed through one `poll()` call.
- **RAII containers**: `std::map` and `std::vector` manage clients, channels, and file descriptors safely.
- **Buffered I/O**: Input and output buffers handle partial `recv()`/`send()` operations robustly.

## Resources

- [RFC 1459 — Internet Relay Chat Protocol](https://datatracker.ietf.org/doc/html/rfc1459)
- [IRC Numerics List](https://defs.ircdocs.horse/defs/numerics.html)
- [Beej's Guide to Network Programming](https://beej.us/guide/bgnet/)
- [Linux `poll()` manual](https://man7.org/linux/man-pages/man2/poll.2.html)
- [Modern IRC Client Protocol](https://modern.ircdocs.horse/)

### AI Usage

AI tools were used during this project for **debugging and testing** purposes. Specifically, AI assisted with:

- Analyzing test output from automated testers to identify failing cases and root causes.
- Debugging protocol formatting issues (e.g., IRC reply syntax, trailing spaces, missing `\r\n` delimiters).
- Testing edge cases such as partial command delivery, nick length validation, and mode handling.

All AI-generated suggestions were manually reviewed, understood, and verified before being integrated into the codebase. The core architecture, design decisions, and main implementations were developed by the team.

## Authors

- kkoujan
- aelbouz
- ynini
