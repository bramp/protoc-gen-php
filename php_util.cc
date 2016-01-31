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
using google::protobuf::CEscape; 
using google::protobuf::SimpleItoa;
using google::protobuf::SimpleFtoa;
using google::protobuf::SimpleDtoa;
using google::protobuf::scoped_array;
using google::protobuf::internal::WireFormat; // TODO Should I be using internal?

/**
 * Returns the path to the generated file for this FileDescriptor
 */
string FileDescriptorToPath(const FileDescriptor & file) {
	return file.name() + ".php";
}

bool FieldHasHas(const FieldDescriptor & field) {
  return (Deref(field.file()).syntax() == FileDescriptor::SYNTAX_PROTO2) || field.containing_oneof();
}

string OneOfConstant(const string & s) {
  return UpperString(s);
}

string VariableName(const string & s) {
	return UnderscoresToCamelCase(s) + '_';
}


string DefaultValueAsString(const FieldDescriptor & field, bool quote_string_type) { // TODO check on the need for quote_string_type
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
      if (quote_string_type)
        //return "'" + CEscape(field.default_value_string()) + "'";
        return "\"" + PHPEscape(field.default_value_string()) + "\"";

      if (field.type() == FieldDescriptor::TYPE_BYTES)
        //return CEscape(field.default_value_string()); // TODO Does this need to be sounded by quotes?
        return "\"" + PHPEscape(field.default_value_string()) + "\"";

      return field.default_value_string(); // TODO Check this works

    case FieldDescriptor::CPPTYPE_ENUM:
      return ClassName(*field.enum_type()) + "::" + field.default_value_enum()->name();

    case FieldDescriptor::CPPTYPE_MESSAGE:
      return "null";
  }

  assert(false); // Every field has a default type, we are missing one
  //return ""; // TODO Should this assert(false) ?
}

string UnderscoresToCamelCase(const string & s) {
  return UnderscoresToCamelCaseImpl(s, false);
}

string UnderscoresToCapitalizedCamelCase(const string & s) {
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

  const int dest_length = len * 4 + 1; // Maximum possible expansion
  scoped_array<char> dest(new char[dest_length]);

  char *p = dest.get();

  while(len > 0) {
    uint8 c = *a++;
    if ((c >= 0 && c <= 31) || c >= 127 ) {
      p += sprintf(p, "\\x%02x", c);
    } else if (c == '"'){
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
string PHPEscape(const string & a) {

  string dest;
  dest.reserve(a.size() * 4); // Maximum possible expansion

  for (size_t i = 0; i < a.size(); i++) {
    char c = a[i];

    if ((c >= 0 && c <= 31) || c >= 127 ) {
      char buf[16];
      snprintf(buf, sizeof(buf), "\\x%02x", c);
      dest += buf;
    } else if (c == '"' || c == '$' || c == '\\'){
      dest += '\\';
      dest += c;
    } else {
      dest += c;
    }
  }

  return dest;
}

string OneLineDefinition(const string & definition) {
  size_t p = definition.find ('{');
  if (p != string::npos)
    return definition.substr(0, p - 1);

  return TrimString(definition);
}

void FieldVariables(const OneofDescriptor & oneof, map<string, string> & variables) {
  variables["name"]                   = VariableName(oneof.name()); // TODO Is this a bug?
  variables["oneof_name"]             = VariableName(oneof.name());
  variables["oneof_case"]             = "_" + VariableName(oneof.name()) + "case";
  variables["oneof_capitalized_name"] = UnderscoresToCapitalizedCamelCase(oneof.name());
  variables["oneof_definition"]       = OneLineDefinition(oneof.DebugString());
  variables["oneof_default"]          = "self::NONE";
  variables["oneof_field"]            = oneof.name();
}

// Returns a map of variables related to this field
void FieldVariables(const FieldDescriptor & field, map<string, string> & variables) {
  variables.clear();

  variables["name"]             = VariableName(field.name());
  variables["[]"]               = field.is_repeated() ? "[]" : ""; 
  variables["definition"]       = OneLineDefinition(field.DebugString());
  variables["default"]          = DefaultValueAsString(field, true);
  variables["capitalized_name"] = UnderscoresToCapitalizedCamelCase(field.name());
  variables["field"]            = field.name();

  if (field.containing_oneof()) {
    variables["oneof"]          = OneOfConstant(field.name());
    FieldVariables(Deref(field.containing_oneof()), variables);
  }

  switch (field.type()) {
//      TODO If PHP>7 enums should be type checked as a int
//      case FieldDescriptor::TYPE_ENUM:
//        variables["type"] = "int ";
//        break;

    case FieldDescriptor::TYPE_MESSAGE:
    case FieldDescriptor::TYPE_GROUP:
      variables["type"] = ClassName(Deref(field.message_type())) + " ";
      break;

    default:
      variables["type"] = "";
  }
}

string MakeTag(const FieldDescriptor & field) {
  return MakeTag(field.number(), WireFormat::WireTypeForFieldType(field.type()));
}

string MakeTag(int field_number, WireFormatLite::WireType type) {
  uint8 tag[5];
  uint8 *tmp = WireFormatLite::WriteTagToArray(field_number, type, tag);
  int tagLen = tmp - tag;
  return string((const char *)tag, tagLen);
}

