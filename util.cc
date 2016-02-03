#include "util.h"

// TODO strutil.h is from the offical protobuf source, but it is not installed.
// Replace it
//      We use LowerString, UpperString, SimpleItoa from it.
#include "strutil.h"

#include <cstdio>  // for sprintf
#include <map>
#include <string>
#include <algorithm>
#include <iostream>

using google::protobuf::UpperString;
using google::protobuf::LowerString;

string UnderscoresToCamelCaseImpl(const string& input, bool cap_next_letter) {
  string result;
  // Note:  I distrust ctype.h due to locales.
  for (size_t i = 0; i < input.size(); i++) {
    if ('a' <= input[i] && input[i] <= 'z') {
      if (cap_next_letter) {
        result += input[i] + ('A' - 'a');
      } else {
        result += input[i];
      }
      cap_next_letter = false;
    } else if ('A' <= input[i] && input[i] <= 'Z') {
      if (i == 0 && !cap_next_letter) {
        // Force first letter to lower-case unless explicitly told to
        // capitalize it.
        result += input[i] + ('a' - 'A');
      } else {
        // Capital letters after the first are left as-is.
        result += input[i];
      }
      cap_next_letter = false;
    } else if ('0' <= input[i] && input[i] <= '9') {
      result += input[i];
      cap_next_letter = true;
    } else {
      cap_next_letter = true;
    }
  }
  return result;
}

string LowerString(const string& s) {
  string newS(s);
  LowerString(&newS);
  return newS;
}

string UpperString(const string& s) {
  string newS(s);
  UpperString(&newS);
  return newS;
}

string TrimString(const string& s) {
  string whitespace = " \t\r\n";
  size_t startpos = s.find_first_not_of(whitespace);
  size_t endpos = s.find_last_not_of(whitespace);
  if (startpos == string::npos) startpos = 0;
  if (endpos == string::npos) endpos = s.length();
  return s.substr(startpos, endpos - startpos);
}
