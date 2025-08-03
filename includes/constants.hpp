#pragma once

#define SERVER_PORT		"4242"
#define MAX_CLIENTS		3
#define LOG_PATH		"/var/log/matt_daemon/matt_daemon.log"
#define LOG_REPORTER	"Matt_daemon"
#define LOCK_PATH		"/var/lock/matt_daemon.lock"

extern int g_stopRequested;
