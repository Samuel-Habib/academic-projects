# Systems Programming in C
Rutgers University · CS 214

Developed by **Samuel Habib (smh389)** and **Victor Peng (vmp134)**.

| Project | Implementation | Technical focus |
| --- | --- | --- |
| [Memory Allocator](Assignment1/) | Custom allocation and deallocation with block splitting, coalescing, pointer validation, and workload tests | Heap organization, alignment, fragmentation, error detection |
| [File Comparison](Assignment2/) | Recursive file discovery and word-frequency comparison using Jensen–Shannon distance | Dynamic memory, directory traversal, tokenization, sorting |
| [Unix Shell](Assignment3/) | Interactive and batch command execution, built-in commands, pipes, redirection, and wildcard expansion | Processes, file descriptors, interprocess communication |
| [TCP Chat Server](Assignment4/) | Multiple client connections, framed messages, broadcast and direct messaging, and status queries | TCP sockets, `poll`, protocol parsing, connection lifecycle |

Each directory contains the C source, makefile, author information, and assignment-specific design and test documentation. The chat server includes the `raw` client used for manual protocol testing.

## Build and test
Run commands from the corresponding assignment directory.

| Project | Command | Validation |
| --- | --- | --- |
| Memory Allocator | `make run_all` | Existing tests pass; compiler warnings remain |
| File Comparison | `make` | The separate `make test` target currently has duplicate `main` definitions |
| Unix Shell | `make mysh`; `make test` | Existing unit tests pass |
| TCP Chat Server | `make` | Builds successfully; manual integration scenarios are documented in its README |

Shared project repositories: [Assignment 1](https://github.com/vmp134/Assignment1), [Assignment 2](https://github.com/vmp134/Assignment2), [Assignment 3](https://github.com/vmp134/Assignment3), [Assignment 4](https://github.com/vmp134/Assignment4).
