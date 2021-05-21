#include "Utils.h"

const char *ws = " \t\n\r\f\v";

// trim from end of string (right)
string &rtrim(string &s)
{
  s.erase(s.find_last_not_of(ws) + 1);
  return s;
}

// trim from beginning of string (left)
string &ltrim(string &s)
{
  s.erase(0, s.find_first_not_of(ws));
  return s;
}

// trim from both ends of string (right then left)
string &trim(string &s)
{
  return ltrim(rtrim(s));
}