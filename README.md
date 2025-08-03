# 42_matt-daemon
[![Demo image](https://github.com/karisti/42_matt-daemon/blob/main/demo.gif)](https://github.com/karisti/42_matt-daemon/blob/main/demo.gif)

## Description
Example server implemented in C++ that runs as a **daemon** in the background and handles TCP connections using `epoll`.

## Lessons
- Daemonization
- Epoll loop
- Signal handling
- Logging system
- Docker usage

## Prerequisites
- Linux with `epoll` support
- Root privileges to start the daemon
- `clang++` or `g++` to compile
- Optional: [Docker](https://docs.docker.com/get-docker/) and [Docker Compose](https://docs.docker.com/compose/) to run in containers

## Usage
### 1. Build
```bash
make
```

### 2. Run the daemon
```bash
sudo ./bin/MattDaemon
```
It listens on port **4242** and writes its log to `/var/log/matt_daemon.log` by default.

### 3. Connect clients
Use any TCP client such as `telnet` or `netcat`:
```bash
telnet localhost 4242
# or
nc localhost 4242
```
On Windows you can test the connection with:
```bash
Test-NetConnection -Computername localhost -Port 4242
```

### 4. Check the log
```bash
tail -f /var/log/matt_daemon.log
```

## Theory
### What is a daemon?
A process running in the background waiting for events. It is used for system tasks such as network management or scheduled jobs.

### Architecture
- **Daemonization** through a double `fork()` and a lock file at `/var/lock/matt_daemon.lock`
- `epoll` event loop to handle multiple clients without threads
- Activity and error logging with `Tintin_reporter`
- Signal handling to stop the server gracefully

### Available commands
- `quit`: closes the connection and stops the daemon if sent by any client
- Any other text is written to the log with no response

### Useful signals

| Command        | Signal                              | Number | Description                                                            | Implemented |
| -------------- | ----------------------------------- | ------ | ---------------------------------------------------------------------- | ----------- |
| `kill -1`      | `SIGHUP` (Hangup)                   | 1      | Terminal closed; often used to reload config or restart daemons        | Yes         |
| `kill -2`      | `SIGINT` (Interrupt)                | 2      | Sent by `Ctrl+C`; interrupts and gracefully terminates the process     | Yes         |
| `kill -3`      | `SIGQUIT` (Quit)                    | 3      | Like `SIGINT` but also creates a core dump for debugging               | Yes         |
| `kill -4`      | `SIGILL` (Illegal Instruction)      | 4      | Illegal instruction executed; indicates a serious execution problem    | No          |
| `kill -5`      | `SIGTRAP` (Trap)                    | 5      | Used by debuggers for breakpoints and tracing                          | No          |
| `kill -6`      | `SIGABRT` (Abort)                   | 6      | Sent by `abort()` on critical error; requests program termination      | No          |
| `kill -7`      | `SIGBUS` (Bus Error)                | 7      | Faulty memory access (misaligned address); hardware or programming bug | No          |
| `kill -8`      | `SIGFPE` (Floating Point Exception) | 8      | Illegal floating-point operation (e.g., division by zero)              | No          |
| `kill -9`      | `SIGKILL` (Kill)                    | 9      | Forcefully and immediately kills the process                           | No          |
| `kill -10`     | `SIGUSR1` (User-defined Signal 1)   | 10     | User-defined signal 1; customizable for application needs              | No          |
| `kill -11`     | `SIGSEGV` (Segmentation Fault)      | 11     | Invalid memory access (segmentation fault); serious programming error  | No          |
| `kill -12`     | `SIGUSR2` (User-defined Signal 2)   | 12     | User-defined signal 2; customizable for application needs              | No          |
| `kill -13`     | `SIGPIPE` (Pipe Error)              | 13     | Write to a pipe with no readers; indicates a broken pipe               | No          |
| `kill -14`     | `SIGALRM` (Alarm)                   | 14     | Timer set by `alarm()` expired; used for timeouts and periodic tasks   | No          |
| `kill -15`     | `SIGTERM` (Termination)             | 15     | Requests graceful process termination (cleanup allowed)                | Yes         |
| `kill -16`     | `SIGSTKFLT` (Stack Fault)           | 16     | Stack fault (overflow/underflow); indicates stack-related bug          | No          |
| `kill -17`     | `SIGCHLD` (Child Process Status)    | 17     | Child process status changed; informs parent of child events           | No          |
| `kill -18`     | `SIGCONT` (Continue)                | 18     | Resumes a stopped process                                              | Yes         |
| `kill -19`     | `SIGSTOP` (Stop)                    | 19     | Stops (pauses) a process; cannot be caught or ignored                  | No          |
| `kill -20`     | `SIGTSTP` (Terminal Stop)           | 20     | Sent by `Ctrl+Z`; suspends process, can be resumed                     | Yes         |
| `kill -21`     | `SIGTTIN` (Background Read)         | 21     | Background process tried to read from terminal                         | No          |
| `kill -22`     | `SIGTTOU` (Background Write)        | 22     | Background process tried to write to terminal                          | No          |
| `kill -23`     | `SIGURG` (Urgent Data)              | 23     | Urgent data available on socket                                        | No          |
| `kill -24`     | `SIGXCPU` (CPU Time Limit Exceeded) | 24     | CPU time limit exceeded                                                | No          |
| `kill -25`     | `SIGXFSZ` (File Size Limit Exceeded)| 25     | File size limit exceeded                                               | No          |
| `kill -26`     | `SIGVTALRM` (Virtual Timer Expired) | 26     | Virtual timer expired (set by `setitimer()`)                           | No          |
| `kill -27`     | `SIGPROF` (Profiling Timer Expired) | 27     | Profiling timer expired (set by `setitimer()`)                         | No          |
| `kill -28`     | `SIGWINCH` (Window Size Change)     | 28     | Terminal window size changed                                           | No          |
| `kill -29`     | `SIGIO` (I/O is Possible)           | 29     | Asynchronous I/O event occurred                                        | No          |
| `kill -30`     | `SIGPWR` (Power Failure)            | 30     | Power failure or critical power event                                  | No          |
| `kill -31`     | `SIGSYS` (Bad System Call)          | 31     | Bad system call (not implemented)                                      | No          |


## Resources
- [Introductory video about daemons](https://www.youtube.com/watch?v=65DarzNIFR0)
- [Understanding Linux Signals](https://prateeksrivastav598.medium.com/understanding-linux-signals-a-comprehensive-guide-339ecc2d16d4)
