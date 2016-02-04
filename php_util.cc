#include "php_util.h"

#include "util.h"
#include "strutil.h"

#include <cstdint>
#include <map>
#include <string>
#include <algorithm>
#include <iostream>

#include <google/protobuf/wire_format.h>
#include <google/protobuf/wire_format_lite.h>
#include <google/protobuf/wire_format_lite_inl.h>

// TODO Replace the following:
using google::protobuf::SimpleItoa;
using google::protobuf::SimpleFtoa;
using google::protobuf::SimpleDtoa;
using google::protobuf::scoped_array;
using google::protobuf::internal::WireFormat;  // TODO Should I be using
                                               // internal?

string FileDescriptorToPath(const FileDescriptor &file) {
  return file.name() + ".php";
}

string DefaultValueAsString( const FieldDescriptor &field) {
  switch (field.cpp_type()) {
    case FieldDescriptor::CPPTYPE_INT32:
      return SimpleItoa(field.default_value_int32());

    case FieldDescriptor::CPPTYPE_INT64:
      return SimpleItoa(field.default_value_int64());

    case FieldDescriptor::CPPTYPE_UINT32:
      return SimpleItoa(field.default_value_uint32());

    case FieldDescriptor::CPPTYPE_UINT64:
      return SimpleItoa(field.default_value_uint64());

    case FieldDescriptor::CPPTYPE_FLOAT:
      return SimpleFtoa(field.default_value_float());

    case FieldDescriptor::CPPTYPE_DOUBLE:
      return SimpleDtoa(field.default_value_double());

    case FieldDescriptor::CPPTYPE_BOOL:
      return field.default_value_bool() ? "true" : "false";

    case FieldDescriptor::CPPTYPE_STRING:
      return "\"" + PHPEscape(field.default_value_string()) + "\"";

    case FieldDescriptor::CPPTYPE_ENUM:
      return ClassName(*field.enum_type()) + "::" +
             field.default_value_enum()->name();

    case FieldDescriptor::CPPTYPE_MESSAGE:
      return "null";
  }

  assert(false);  // Every field has a default type, we are missing one
}

string UnderscoresToCamelCase(const string &s) {
  return UnderscoresToCamelCaseImpl(s, false);
}

string UnderscoresToCapitalizedCamelCase(const string &s) {
  return UnderscoresToCamelCaseImpl(s, true);
}

/**
 * Turns a 32 bit number into a string suitable for PHP to print out.
 * For example, 0x12345678 would turn into "\x12\x34\x56\78".
 * @param tag
 * @return
 */
string ArrayToPHPString(const uint8 *a, size_t len) {
  assert(a != nullptr);
  assert(len > 0);

  const int dest_length = len * 4 + 1;  // Maximum possible expansion
  scoped_array<char> dest(new char[dest_length]);

  char *p = dest.get();

  while (len > 0) {
    uint8 c = *a++;
    if ((c >= 0 && c <= 31) || c >= 127) {
      p += sprintf(p, "\\x%02x", c);
    } else if (c == '"') {
      *p++ = '\\';
      *p++ = c;
    } else {
      *p++ = c;
    }

    len--;
  }

  // TODO Remove: *p = '\0'; // Null terminate us

  return string(dest.get(), p - dest.get());
}

// Returns a string escaped to be safe within a PHP string.
string PHPEscape(const string &a) {
  string dest;
  dest.reserve(a.size() * 4);  // Maximum possible expansion

  for (size_t i = 0; i < a.size(); i++) {
    char c = a[i];

    if ((c >= 0 && c <= 31) || c >= 127) {
      char buf[16];
      snprintf(buf, sizeof(buf), "\\x%02x", c);
      dest += buf;
    } else if (c == '"' || c == '$' || c == '\\') {
      dest += '\\';
      dest += c;
    } else {
      dest += c;
    }
  }

  return dest;
}

string OneLineDefinition(const string &definition) {
  size_t p = definition.find('{');
  if (p != string::npos) return definition.substr(0, p - 1);

  return TrimString(definition);
}


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

string MakeTag(const FieldDescriptor &field) {
  return MakeTag(field.number(), WireFormat::WireTypeForFieldType(field.type()));
}

string MakeTag(int field_number, WireFormatLite::WireType type) {
  uint8 tag[5];
  uint8 *tmp = WireFormatLite::WriteTagToArray(field_number, type, tag);
  int tagLen = tmp - tag;
  return string((const char *)tag, tagLen);
}

