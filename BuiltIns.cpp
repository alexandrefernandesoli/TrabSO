#include "BuiltIns.h"

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

void set_command()
{
  extern char **environ;
  char **s = environ;

  for (; *s; s++)
  {
    printf("%s\n", *s);
  }
}

void echo_command(vector<string> args)
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

void cd_command(vector<string> args, char cur_dir[])
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

void jobs_command(map<int, pair<int, string>> job_list)
{
  for (map<int, pair<int, string>>::iterator it = job_list.begin(); it != job_list.end(); ++it)
  {
    cout << it->first << " " << it->second.second << endl;
  }
}

void kill_command(vector<string> args, map<int, pair<int, string>> job_list)
{
  int pid = atoi(args.front().c_str());
  int status = kill(pid, SIGKILL);
  cout << pid;

  if (status < 0)
  {
    perror("An error occurred");
  }
  else
  {
    // set the int value to 1

    cout << "processo morto?";

    map<int, pair<int, string>>::iterator job = job_list.find(pid);

    if (job != job_list.end())
    {
      job_list[pid] = pair<int, string>(1, (job->second).second);
    }
  }
}
