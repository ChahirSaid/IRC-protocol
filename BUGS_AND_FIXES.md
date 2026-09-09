# IRC Bugs and Fixes

This document records the bugs found during manual testing and the fixes applied so far.

## Fixed Bugs

### Duplicate channel membership

**Bug:** A client could send `JOIN #channel` more than once. The server added the same entity to the channel roster repeatedly and emitted duplicate JOIN messages.

**Fix:** `JOIN` now checks whether the client is already in the channel roster before adding it.

**File:** `source/systems/handlers/ChannelHandlers.cpp`

### Stale nicknames after a nickname change

**Bug:** Changing a nickname updated the client's profile and added the new map entry, but left the old nickname in `_clientsByName`. The old nickname could incorrectly remain addressable.

**Fix:** `EntityManager::setClientName()` removes the old mapping when it still belongs to the same client before inserting the new nickname.

**File:** `source/managers/EntityManager.cpp`

### Registration failures were invisible to IRC clients

**Bug:** Invalid PASS, NICK, and USER commands were logged by the server but did not return IRC numeric replies. A real IRC client could wait indefinitely or assume a command succeeded.

**Fixes:** Added client-visible replies for:

- `461` - missing command parameters
- `462` - attempted re-registration
- `464` - incorrect password
- `431` - missing nickname
- `433` - nickname already in use
- `451` - registration required

**File:** `source/systems/handlers/RegistrationHandlers.cpp`

### JOIN and PART errors were invisible to clients

**Bug:** Channel access failures were logged but not sent to the client.

**Fixes:** Added replies for:

- `403` - no such channel
- `442` - client is not on the channel
- `461` - missing parameters
- `471` - channel is full
- `473` - invite-only channel
- `475` - incorrect channel key

**File:** `source/systems/handlers/ChannelHandlers.cpp`

### PRIVMSG errors were invisible to clients

**Bug:** Invalid message targets, missing text, and channel membership failures were only logged server-side.

**Fixes:** Added replies for:

- `401` - no such nickname
- `403` - no such channel
- `412` - no text to send
- `442` - client is not on the channel
- `461` - missing parameters
- `451` - registration required

**File:** `source/systems/handlers/MessagingHandlers.cpp`

### KICK and INVITE errors were invisible to clients

**Bug:** Permission failures, unknown users, missing parameters, and invalid membership states were only logged.

**Fixes:** Added replies for:

- `401` - no such nickname
- `403` - no such channel
- `441` - target is not on the channel
- `443` - target is already on the channel
- `461` - missing parameters
- `482` - client is not a channel operator

**File:** `source/systems/handlers/ChannelHandlers.cpp`

### MODE and TOPIC errors were invisible to clients

**Bug:** Invalid targets, missing parameters, membership failures, and permission failures were only logged.

**Fixes:** Added replies for:

- `403` - no such channel
- `442` - client is not on the channel
- `461` - missing parameters
- `472` - unknown mode character
- `482` - client is not a channel operator

**Files:** `source/systems/handlers/ChannelHandlers.cpp`, `source/systems/handlers/ModeHandlers.cpp`

## Verified Behavior

The following behavior was tested with real local TCP clients:

- C++98 build with `-Wall -Wextra -Werror`
- PASS/NICK/USER registration
- Incorrect password handling
- Nickname collision and nickname replacement
- Multiple IRC commands in one packet
- Nonblocking `poll()` network loop
- JOIN and duplicate JOIN handling
- PART and channel cleanup
- Channel PRIVMSG delivery
- Private PRIVMSG delivery
- Invite-only channels
- Channel keys
- Channel user limits
- TOPIC permissions and updates
- MODE permissions and updates
- KICK and INVITE permissions
- Disconnect cleanup and reconnecting clients

## Remaining Known Issues

These items were identified but have not been implemented yet:

### QUIT is not implemented

The command dispatcher does not handle `QUIT`. A proper implementation should broadcast the quit message to shared channels, remove the client from all rosters, close the socket, and destroy the entity cleanly.

### NOTICE is not implemented

`NOTICE` is a likely bonus feature. It should deliver messages to users and channels without generating automatic error replies, according to IRC behavior.

### Some MODE argument validation is incomplete

Missing or malformed arguments for modes such as `+k`, `+l`, and `+o` can be silently ignored. These paths should return appropriate IRC numerics instead of partially applying a command.

### Error handling is not fully centralized

Each handler currently has its own small reply helper and numeric formatting. A shared reply utility would reduce duplication and make future protocol corrections easier.

### Network write edge cases need hardening

`send()` failures and partial writes are not fully handled. The output buffer should preserve unsent data on errors and handle `EAGAIN`/`EWOULDBLOCK` explicitly.

### Network read edge cases need hardening

The server treats all `recv()` results less than or equal to zero as disconnects. Nonblocking `EAGAIN`/`EWOULDBLOCK` should be handled separately from an actual peer disconnect.

### IRC validation is incomplete

Nickname, username, channel-name, message-length, and parameter validation is basic. A production-compatible server should enforce IRC syntax and maximum line lengths more strictly.

## Current Status

The mandatory command paths tested so far are functional and provide client-visible errors for the main rejection cases. The IRC bot bonus now builds and responds to private messages with help text, ASCII animals, and unknown-command feedback. The next recommended work is to finish MODE argument validation and implement `QUIT`, then consider `NOTICE` as an additional feature.
