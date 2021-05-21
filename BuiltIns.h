#include <stdio.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <signal.h>
#include <unistd.h>
#include <sys/utsname.h>
#include <string.h>
#include <regex>
#include <ctime>
#include <sys/wait.h>

using namespace std;

void check_builtins();

void show_history();
void set_command();
void echo_command(vector<string> args);
void cd_command(vector<string> args, char cur_dir[]);
void jobs_command(std::map<int, std::pair<int, std::string>> job_list);
void kill_command(vector<string> args, std::map<int, std::pair<int, std::string>> job_list);