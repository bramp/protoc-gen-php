//TODO Rename strutil.h

#include <string>

using std::string;

string UnderscoresToCamelCaseImpl(const string & input, bool cap_next_letter);

string LowerString(const string & s);
string UpperString(const string & s);
string TrimString(const string & s);
