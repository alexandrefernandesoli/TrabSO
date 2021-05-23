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
#include "Utils.h"
#include "BuiltIns.h"

using namespace std;

// Headers
void initialize();
void handle_sigint();
void handle_ctrd(char *entry);
void write_history(string command);
void execute_command(string command);
void check_builtins();
void run_external();

// Variáveis Globais
enum State
{
  OUTSIDE,
  WORD,
  QUOTE
};
char cur_dir[256];
string command_string = "";
string prompt = "tecii$ ";
struct utsname uname_data;
vector<string> args;
map<int, pair<int, string>> job_list;

// Função chamada no inicio da execução, define variáveis de ambiente
void initialize()
{
  getcwd(cur_dir, 256);
  uname(&uname_data);

  setenv("MYPATH", getenv("PATH"), 1);
  setenv("MYPS1", "tecii$", 1);
}

// Função chamada no momento que um ctrl+C é detectado
void handle_sigint(int sig)
{
  if (sig)
  {
  } // ignore ctrl+C
}

// Função que encapsula leitura de entrada, usada para lidar com ctrl+D
void handle_ctrd(char *entry)
{
  if (!entry)
  {
    cout << "\n";
    exit(0);
  }
}

// Função que cria arquivo history.txt e salva comandos executados
void write_history(string command)
{
  ifstream in("history.txt");
  vector<string> v;
  int n = 0;

  string s;
  if (in.is_open())
  {
    while (!in.eof())
    {
      getline(in, s);

      v.push_back(s);
      n++;
    }

    in.close();
  }

  v.push_back(command);

  if (n >= 50) // Quantidade de comandos que serão salvos
  {
    ofstream out("temp.txt", std::fstream::out | std::fstream::app);

    if (out.is_open())
    {

      for (int i = 1; i < n; i++)
      {

        out << v[i];
        out << "\n";
      }

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

// Função para remover espaços que estejam no fim e no começo de cada comando e \n
string parse_command(char text[])
{
  regex space_remove_regex("^\\s+|\\s+$"); // remove leading and trailing spaces
  regex newlines_remove_regex("\n+");      // remove new line

  string parsed;

  parsed = regex_replace(regex_replace(text, space_remove_regex, ""), newlines_remove_regex, "\0");

  parsed.erase(std::remove(parsed.begin(), parsed.end(), '\n'), parsed.end());

  return parsed;
}

// Função que divide os argumentos de cada comando em um vector<string>
void split_args(string input)
{
  args.clear();
  command_string = "";

  // Verificação se o comando possui pipes (não implementado)
  if (input.find('|') != string::npos)
  {
    cout << "\nTem pipes (não implementado)\n";
    string tmp;
    stringstream string_stream(input);
    vector<string> commands;

    // Divide os comandos pelas pipes
    while (getline(string_stream, tmp, '|'))
    {
      commands.push_back(tmp);
    }

    for (string command : commands)
    {
      cout << "<" << trim(command) << ">";
    }

    return;
  }
  else
  {
    std::stringstream ss(input);
    State state = WORD;
    string arg;
    while (ss.peek() != EOF)
    {
      char c = ss.get();

      switch (state)
      {
      case OUTSIDE:
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
    if (!args.empty())
    {
      command_string = args.front();
      args.erase(args.begin());
    }
  }
}

//  Função chamada para executar o comando, também verifica se o comando não é só uma string vazia, se for só ignora
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

// Função que verifica se o comando executado é um comando implementado pela própria shell
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
    echo_command(args);
  }
  else if (command_string == "cd")
  {
    cd_command(args, cur_dir);
  }
  else if (command_string == "jobs")
  {
    jobs_command(job_list);
  }
  else if (command_string == "kill")
  {
    kill_command(args, job_list);
  }
  else
  {
    run_external();
  }
}

// Util que transforma string em char*
vector<char *> str_to_charptr(string cmd, vector<string> args)
{
  vector<char *> argv;
  argv.push_back(&cmd[0]);
  for (vector<string>::iterator it = args.begin(); it != args.end(); ++it)
  {
    argv.push_back(&(*it)[0]);
  }
  argv.push_back((char *)NULL);
  return argv;
}

// Função para rodar programa externo à shell
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
      job_list[pid] = pair<int, string>(0, command_string); // add to jobs list
    }
    else
    {
      wait(&status);
    }
  }
}

// Função main
int main()
{
  signal(SIGINT, &handle_sigint);

  initialize();

  while (1)
  {
    prompt = getenv("MYPS1");
    cout << prompt << " ";

    char text[100];

    handle_ctrd(fgets(text, sizeof(text), stdin));

    string command = parse_command(text);
    split_args(command);
    execute_command(command);
  }

  return 0;
}