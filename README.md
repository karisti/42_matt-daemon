# 42_matt-daemon
[![Demo image]()]()

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
| Command     | Signal    | Number | Description                           |
| ----------- | --------- | ------ | ------------------------------------- |
| `kill -15`  | `SIGTERM` | 15     | Terminate the process gracefully      |
| `kill -9`   | `SIGKILL` | 9      | Kill the process immediately          |
| `kill -2`   | `SIGINT`  | 2      | Same as pressing `Ctrl+C`             |
| `kill -3`   | `SIGQUIT` | 3      | Same as pressing `Ctrl+\`            |
| `kill -19`  | `SIGSTOP` | 19     | Pause the process (like `Ctrl+Z`)     |
| `kill -20`  | `SIGTSTP` | 20     | Suspend with a signal handler (`Ctrl+Z`)|

## Resources
- [Introductory video about daemons](https://www.youtube.com/watch?v=65DarzNIFR0)
