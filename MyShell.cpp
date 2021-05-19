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
// #include <sys/stat.h>
// #include <sys/types.h>

using namespace std;

enum State
{
  OUTSIDE,
  WORD,
  QUOTE
};

// headers
void initialize();
void handle_sigint();
void handle_ctrd(char *entry);
void write_history(string command);
void execute_command(string command);
void set_command();
void echo_command();
void cd_command();
void check_builtins();
void run_external();
// string read_line(char *prompt);

// global variables
int status = 1;    // 1 - OK, 0 - EXIT OR ERROR
char cur_dir[256]; // current directory
int cmd_counter = 0;
string command_string = "";
string prompt = "tecii$ ";
struct utsname uname_data;
extern char **environ;
vector<string> args;

void initialize()
{
  getcwd(cur_dir, 256);
  uname(&uname_data);

  setenv("MYPATH", getenv("PATH"), 1);
  setenv("MYPS1", "tecii$", 1);
}

void handle_sigint(int sig)
{
  if (sig)
  {
  } // ignore ctrl+C
}

void handle_ctrd(char *entry)
{
  if (!entry)
  {
    cout << "\n";
    exit(0);
  }
}

void write_history(string command)
{
  //Function to write the current command into the history file.
  //This function checks for 2000 entries, replaces the first entry if no of lines = 2000 else appends

  ifstream in("history.txt");
  vector<string> v;
  int n = 0;

  string s;
  if (in.is_open())
  {
    while (!in.eof())
    {
      getline(in, s);
      //cout<<"\nRead: "<<s;

      v.push_back(s);
      n++;
    }

    in.close();
  }

  //printf("n is : %d",n);
  v.push_back(command);

  if (n >= 50) //THRESHOLD VALUE
  {
    //n is equal to threshold
    //cout<<"Printing";
    ofstream out("temp.txt", std::fstream::out | std::fstream::app);

    if (out.is_open())
    {

      for (int i = 1; i < n; i++)
      {

        out << v[i];
        out << "\n";
      }

      //Push Final Element
      out << v[n];

      out.close();
    }
    else
    {
      perror("\nFile not opened");
    }

    remove("history.txt");
    rename("temp.txt", "history.txt");
  }
  else
  {
    //printf("\nAppending..");
    ofstream out("history.txt", std::fstream::out | std::fstream::app);

    if (out.is_open())
    {
      if (n != 0)
      {
        out << "\n";
      }

      out << v[n];

      out.close();
    }
  }
}

void show_history()
{
  //Function to print the history
  // printf("in history");
  ifstream in("history.txt");
  int n = 1;

  string s;
  if (in.is_open())
  {
    while (!in.eof())
    {
      getline(in, s);
      cout << n << ". " << s << "\n";
      n++;
    }

    in.close();
  }
}

string parse_command(char text[])
{
  regex space_remove_regex("^\\s+|\\s+$"); // remove leading and trailing spaces
  regex newlines_remove_regex("\n+");      // remove new line

  string parsed;

  parsed = regex_replace(regex_replace(text, space_remove_regex, ""), newlines_remove_regex, "\0");

  parsed.erase(std::remove(parsed.begin(), parsed.end(), '\n'), parsed.end());

  return parsed;
}

void split_args(string input)
{
  // clear args & command
  args.clear();
  command_string = "";

  std::stringstream ss(input);
  State state = WORD;
  string arg;
  while (ss.peek() != EOF)
  {
    char c = ss.get();

    switch (state)
    {
    case OUTSIDE:
      // move to next, dont add to word
      if (ss.peek() == '"')
      {
        state = QUOTE;
        args.push_back(arg);
        arg.clear();
      }
      else if (ss.peek() != ' ')
      {
        state = WORD;
        args.push_back(arg);
        arg.clear();
      }
      break;

    case WORD:
      // move to next & add unless a space
      if (c == '\\')
      {
        c = ss.get();
      }
      if (ss.peek() == ' ')
      {
        state = OUTSIDE;
      }
      arg.push_back(c);
      break;

    case QUOTE:
      // move to next & add unless end quote
      if (ss.peek() == '"')
      {
        state = OUTSIDE;
      }
      if (c != '"')
      {
        arg.push_back(c);
      }
      break;
    }
  }
  if (arg.size() != 0)
  {
    args.push_back(arg);
  }

  // remove the command from the args list
  if (!args.empty())
  {
    command_string = args.front();
    args.erase(args.begin());
  }
}

void execute_command(string command)
{
  if (command_string == "")
  {
    return;
  }
  else
  {
    write_history(command);
    check_builtins();
    return;
  }
}

void check_builtins()
{
  if (command_string == "exit")
  {
    cout << "Bye bye...\n";
    exit(0);
  }
  else if (command_string == "history")
  {
    show_history();
  }
  else if (command_string == "set")
  {
    set_command();
  }
  else if (command_string == "echo")
  {
    echo_command();
  }
  else if (command_string == "cd")
  {
    cd_command();
  }
  else
  {
    run_external();
  }
}

vector<char *> str_to_charptr(string cmd, vector<string> args)
{
  vector<char *> argv;
  argv.push_back(&cmd[0]);
  for (vector<string>::iterator it = args.begin(); it != args.end(); ++it)
  {
    argv.push_back(&(*it)[0]);
  }
  argv.push_back((char *)NULL); // needs to be null-terminated
  return argv;
}

void run_external()
{
  int pid = fork();
  int status = 0;
  bool background = false;
  if (args.empty())
  {
    background = false;
  }
  else if (args.back() == "&")
  {
    background = true;
    args.pop_back(); // remove '&' from args
  }

  if (pid == 0)
  { // new process
    vector<char *> argv = str_to_charptr(command_string, args);
    status = execvp(command_string.c_str(), &argv[0]);
    if (status == -1)
    {
      perror("An error occurred");
    }
  }
  else
  {
    if (background)
    {
      cout << pid << " " << command_string << endl;
      // job_list[pid] = pair<int, string>(0, command_string); // add to jobs list
      // return status;
    }
    else
    {
      wait(&status);
    }
  }
}

void set_command()
{
  char **s = environ;

  for (; *s; s++)
  {
    printf("%s\n", *s);
  }
}

void echo_command()
{
  if (args.size() == 0)
  {
    return;
  }
  else if (args[0].substr(0, 1) == "$")
  {
    char string_char[args[0].length() + 1];

    strcpy(string_char, args[0].substr(1).c_str());

    if (getenv(string_char))
    {
      string env_value = getenv(string_char);
      cout << env_value << "\n";
    }
    else
    {
      cout << "\n";
    }
  }
  else
  {
    cout << args[0] << "\n";
  }
}

void cd_command()
{
  int status = chdir(args.front().c_str());
  if (status == -1)
  {
    perror("An error occurred");
  }
  else
  {
    getcwd(cur_dir, 256); // update the prompt cwd
    setenv("PWD", cur_dir, 1);
    // cout << cur_dir;
  }
}

int main()
{
  signal(SIGINT, &handle_sigint);

  initialize();

  while (status)
  {
    prompt = getenv("MYPS1");
    cout << prompt << " ";

    char text[100];

    handle_ctrd(fgets(text, sizeof(text), stdin));

    string command = parse_command(text);
    split_args(command);

    // cout << args[0];

    execute_command(command);
  }

  return 0;
}