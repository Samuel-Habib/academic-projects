# Network-Centric Programming in C
Samuel Habib · Rutgers University · ECE 14:332:456

Course projects in socket programming, concurrency, interprocess communication, and low-level memory analysis.

| Project | Implementation | Technical focus |
| --- | --- | --- |
| [Concurrent Web Proxy](concurrent-proxy/) | HTTP forwarding with process and thread modes, CONNECT tunneling, and request logging | TCP sockets, `fork`, pthreads, bidirectional I/O with `select` |
| [Multicast Heartbeat Controller](multicast-heartbeat/) | Controller-issued heartbeat intervals, client timers, controller-loss detection, and interactive takeover | UDP multicast, `poll`, timing, message structures, logging |
| [Sequential Web Proxy](sequential-proxy/) | HTTP GET parsing, destination connections, response forwarding, and request-size logging | Client/server sockets, buffered I/O, URL parsing |
| [Multiple Processes](processes/) | Partitioned summation with child processes returning partial results through pipes | Process creation, IPC, synchronization with `wait` |
| [Multiple Threads](threads/) | Partitioned sum-of-squares calculation with shared result aggregation | pthreads, mutexes, shared memory |
| [Buffer-Overflow Lab](buffer-overflow/) | Report on disassembly, stack layout, return-address manipulation, and payload construction | x86 assembly, GDB, memory representation |
| [File Search](file-search/) | Command-line file-search warmup with recorded timings | C file processing and runtime measurement |

## Build and run

Run `make` inside a project directory. Example commands:

| Project | Command |
| --- | --- |
| Concurrent proxy — processes | `./proxy -p 8080` |
| Concurrent proxy — threads | `./proxy -t 8080` |
| Sequential proxy | `./proxy 8080` |
| Heartbeat controller | `./heartbeat_app controller -c 5` |
| Heartbeat client | `./heartbeat_app client` |
| Multiple processes | `./h1 200 10` |
| Multiple threads | `./main 200 10` |

Each directory includes the assignment specification and original submission archive. C source and build files are available directly alongside the archives. The buffer-overflow directory contains the submitted report, lab specification, and supplied binary handout. CS:APP helper code and supplied examples retain their original attribution.

## Validation and scope

All six C submissions build with their supplied Makefiles on the review environment. Network integration behavior has not been revalidated.

- The concurrent proxy includes process and thread modes, but synchronization remains incomplete: the GET path bypasses the DNS mutex, and GET logging does not use the locking applied to tunnel logging. Process mode also lacks child reaping.
- The heartbeat application uses fixed multicast address and port values; the `-m` and `-p` options described in its original README are not implemented. Controller takeover requires user input, and controller conflict handling is incomplete.
- The buffer-overflow report documents levels 0–3 and an attempted level 4 that remained unreliable.
