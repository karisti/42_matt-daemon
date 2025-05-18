#pragma once

#include <string>
#include <vector>
#include <iostream>
#include <csignal>
#include <signal.h>


extern bool socketKiller;
extern bool g_stopRequested;


std::vector<std::string>	splitString(std::string str, std::string delim, int count = -1);
void						catchSignal(void);
void						sig_handle(int sig);
std::string					pingGenerator(const int &size);
bool						isNumber(std::string str);
void						printStrVector(std::string name, std::vector<std::string> strs);
