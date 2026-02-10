# Redis Clone

A learning/experimental Redis implementation written in C.

## Project Overview

This is a simplified Redis clone that implements basic key-value operations over a custom binary protocol. The implementation focuses on demonstrating fundamental networking concepts, data structures, and command processing in C.

## Implemented Features

### Supported Commands
- `GET <key>` - Retrieves a value for the given key (returns hardcoded "value")
- `SET <key> <value>` - Sets a key-value pair (currently returns error)
- `DEL <key>` - Deletes a key (currently returns NX - not exists)

### Protocol
- Custom binary protocol with length-prefixed messages
- Request format: `[4 bytes command length][4 bytes argument count][4 bytes string length][string data]...`
- Response format: `[4 bytes response length][4 bytes response code][response data]`
- Response codes: RES_OK (0), RES_ERR (1), RES_NX (2)

### Networking
- TCP server listening on 127.0.0.1:8081
- Non-blocking I/O with `poll()` for event multiplexing
- Connection state machine (STATE_REQ, STATE_RES, STATE_END)
- Signal handling for graceful shutdown (SIGINT)

## Architecture

### Server Model
- **Single-threaded event loop** using `poll()` for I/O multiplexing
- **Non-blocking sockets** for all client connections
- **Connection state management** with explicit read/write states
- **Dynamic connection tracking** with expandable connection arrays

### Core Modules

#### Connection Management (`conn.c`, `conn.h`)
- `struct conn` - Per-connection state with read/write buffers
- `struct clist` - Dynamic array of connection pointers indexed by file descriptor
- Automatic capacity expansion when file descriptors exceed current array size

#### Protocol Processing (`server.c`)
- Binary protocol parsing with length validation
- Command extraction and routing
- Response generation and transmission
- Buffer management for partial reads/writes

#### Command Engine (`command.c`, `command.h`)
- Currently implements placeholder functions for GET, SET, DEL
- String-based key-value interface
- Fixed argument validation (MAX_ARGS_COUNT = 3)

#### Data Structures
- **Dynamic strings** (`estring.c`): Resizable string buffers with automatic capacity management
- **String lists** (`slist.c`): Dynamic arrays of string pointers for command arguments
- **Hash table** (`hashtable.c`): Framework exists but core functions are stub implementations
- **Poll list** (`plist.c`): Dynamic array management for `pollfd` structures

#### Utilities
- **Logging** (`log.c`): Colored console output and file logging capability
- **Memory management**: Dynamic arrays with exponential growth strategies
- **Error handling**: Consistent error codes and logging throughout

### Internal Data Structures

#### Connection Buffers
- Fixed-size buffers (4KB + headers) per connection
- Length-prefixed protocol prevents buffer overflows
- State machine handles partial message boundaries

#### String Implementation
- Header: `char *data`, `size_t len`, `size_t cap`
- Automatic capacity doubling when needed
- Null-terminated strings for C compatibility

#### Hash Table Framework
- Designed for incremental resizing with main/temp maps
- Power-of-two bucket allocation with bit masking
- Collision resolution via separate chaining

## Supported Commands

### GET
- **Usage**: `GET <key>`
- **Behavior**: Returns hardcoded string "value" for any key
- **Response Code**: RES_OK (0)
- **Current Limitation**: Does not perform actual key lookup

### SET
- **Usage**: `SET <key> <value>`
- **Behavior**: Logs the operation but returns error
- **Response Code**: RES_ERR (1)
- **Current Limitation**: Does not store data

### DEL
- **Usage**: `DEL <key>`
- **Behavior**: Logs the operation but returns NX (not exists)
- **Response Code**: RES_NX (2)
- **Current Limitation**: Does not delete data

## Build & Run Instructions

### Prerequisites
- GCC compiler with C11 support
- POSIX-compliant system (Linux/Unix)

### Building
```bash
make
```

### Running Server
```bash
./build/main server
```
Server will start listening on 127.0.0.1:8081

### Running Client
```bash
./build/main client set key value
./build/main client get key
./build/main client del key
```

### Cleaning Build
```bash
make clean
```

## Limitations

### Functional Limitations
- **No actual data persistence**: All commands return mock responses
- **Incomplete hash table**: Core storage operations are not implemented
- **No persistence**: No RDB or AOF functionality
- **Limited protocol**: Not compatible with standard Redis RESP protocol
- **Single-threaded**: No concurrency or background operations

### Architectural Constraints
- **Memory usage**: Fixed 4KB buffers per connection may limit large values
- **Connection limits**: Linear file descriptor array lookup
- **No authentication**: No security or access control
- **No configuration**: Hardcoded network settings

### Protocol Limitations
- **Custom binary format**: Incompatible with Redis clients
- **No pipelining**: One request/response cycle at a time
- **No multi-bulk operations**: Limited to simple string arguments

## Current State

This is an educational implementation demonstrating:
- Socket programming with non-blocking I/O
- Event-driven server architecture
- Custom binary protocol design
- Dynamic memory management in C
- Modular C project organization

The codebase provides a solid foundation for building a complete Redis-like server but requires significant implementation work to become functional.
