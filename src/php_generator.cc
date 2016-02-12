/**
 * PHP Protocol Buffer Generator Plugin for protoc
 * By Andrew Brampton (c) 2010,2016
 */

#include "php_generator.h"

PHPFileGenerator::PHPFileGenerator(io::Printer& printer,
                                   const FileDescriptor& file,
                                   const string& parameter)
    : printer_(printer),
      file_(file),
      parameter_(parameter),

      // Parse the file options
      options_(file.options().GetExtension(php)) {
  (void)parameter_;  // Unused
}

PHPFileGenerator::~PHPFileGenerator() {}

string PHPFileGenerator::DefaultValueAsString(const FieldDescriptor &field) {
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
      return ClassName(*field.enum_type()) + "::" + field.default_value_enum()->name();

    case FieldDescriptor::CPPTYPE_MESSAGE:
      return "null";
  }

  assert(false);  // Every field has a default type, we are missing one
}

void PHPFileGenerator::FieldVariables(const OneofDescriptor &oneof,
                    map<string, string> &variables) {
  variables["name"] = VariableName(oneof.name());  // TODO Is this a bug?
  variables["oneof_name"] = VariableName(oneof.name());
  variables["oneof_case"] = "_" + VariableName(oneof.name()) + "case";
  variables["oneof_capitalized_name"] =
      UnderscoresToCapitalizedCamelCase(oneof.name());
  variables["oneof_definition"] = OneLineDefinition(oneof.DebugString());
  variables["oneof_default"] = "self::NONE";
  variables["oneof_field"] = oneof.name();

  if (TargetPHP() >= 70) {
    variables["oneof_case_type"] = "int ";
    variables["oneof_case_return_type"] = ": int";
    variables["bool_return_type"] = ": bool";

  } else {
    variables["oneof_case_type"] = "";
    variables["oneof_case_return_type"] = "";
    variables["bool_return_type"] = "";
  }
  
}

void PHPFileGenerator::TypeHintingFieldVariables(const FieldDescriptor &field,
                          map<string, string> &variables) {

  // Type hinting related to this field
  variables["type"] = "";
  variables["return_type"] = "";

  // Generic type hints
  variables["int_type"] = "";
  variables["array_return_type"] = "";
  variables["bool_return_type"] = "";
  variables["int_return_type"] = "";


  if (TargetPHP() >= 50) {
    // Only "class names supported"
    if (field.type() == FieldDescriptor::TYPE_MESSAGE || field.type() == FieldDescriptor::TYPE_GROUP) {
      variables["type"] = FullClassName(Deref(field.message_type())) + " ";
    }
  }

  if (TargetPHP() >= 70) {
    // PHP 7.0 supports many primiatives, and return types

    variables["int_type"] = "int ";
    variables["array_return_type"] = ": array";
    variables["bool_return_type"] = ": bool";
    variables["int_return_type"] = ": int";

    switch (field.type()) {
      //      TODO If PHP>7 enums should be type checked as a int
      case FieldDescriptor::TYPE_ENUM:
        variables["type"] = "int ";
        variables["return_type"] = ": int";
        break;

      case FieldDescriptor::TYPE_BOOL:
        variables["type"] = "bool ";
        variables["return_type"] = ": bool";

        break;

      case FieldDescriptor::TYPE_FLOAT:
      case FieldDescriptor::TYPE_DOUBLE:
        variables["type"] = "float ";
        variables["return_type"] = ": float";
        break;

      case FieldDescriptor::TYPE_STRING:  // UTF-8 text.
      case FieldDescriptor::TYPE_BYTES:   // Arbitrary byte array.
        variables["type"] = "string ";
        variables["return_type"] = ": string";

      case FieldDescriptor::TYPE_MESSAGE:
      case FieldDescriptor::TYPE_GROUP:
        variables["return_type"] = ": " + FullClassName(Deref(field.message_type()));
        break;

      case FieldDescriptor::TYPE_SINT32:
      case FieldDescriptor::TYPE_SINT64:
      case FieldDescriptor::TYPE_FIXED64:
      case FieldDescriptor::TYPE_SFIXED64:
      case FieldDescriptor::TYPE_FIXED32:
      case FieldDescriptor::TYPE_SFIXED32:
      case FieldDescriptor::TYPE_INT32:
      case FieldDescriptor::TYPE_INT64:
      case FieldDescriptor::TYPE_UINT32:
      case FieldDescriptor::TYPE_UINT64:
        // Do nothing because many ints could be int or float (due to PHP's limitations)
        break;
    }
  }

  if (TargetPHP() >= 51) {
    // Since 5.1 array type supported. We put this last, so it can replace any repeated types, with array
    variables["array_type"] = "array ";
    if(field.is_repeated()) {
      variables["type"] = "array ";
      if (TargetPHP() >= 70) {
        variables["return_type"] = ": array";
      }
    }
  }

}

// Returns a map of variables related to this field
void PHPFileGenerator::FieldVariables(const FieldDescriptor &field,
                    map<string, string> &variables) {
  variables.clear();

  variables["name"] = VariableName(field.name());
  variables["[]"] = field.is_repeated() ? "[]" : "";
  variables["definition"] = OneLineDefinition(field.DebugString());
  variables["default"] = DefaultValueAsString(field);
  variables["capitalized_name"] =
      UnderscoresToCapitalizedCamelCase(field.name());
  variables["field"] = field.name();

  if (field.containing_oneof()) {
    variables["oneof"] = OneOfConstant(field.name());
    FieldVariables(Deref(field.containing_oneof()), variables);
  }

  TypeHintingFieldVariables(field, variables);
}


bool PHPFileGenerator::Generate(string* error) {
  // TODO Check if the last-mod time of the php file is newer than the proto. If
  // so skip.
  string php_filename = FileDescriptorToPath(file_);

  try {
    printer_.Print(
        "<?php\n"
        "// Generated by https://github.com/bramp/protoc-gen-php"
        "// Please include protocolbuffers before this file, for example:\n"
        "//   require('protocolbuffers.inc.php');\n"
        "//   require('`filename`');\n",
        "filename", php_filename.c_str());

    printer_.Print("\n");

    if (UseNamespaces() && !file_.package().empty()) {
      // If we are using namespaces
      printer_.Print(
        "namespace `namespace` {\n\n"
        "  use Protobuf;\n"
        "  use ProtobufEnum;\n"
        "  use ProtobufMessage;\n\n",

        "namespace", NamespaceName(file_).c_str()); 
      printer_.Indent();
    }

    // TODO Wrap the following in option that asks about autoloading
    // TODO Move the following into a method
    for (int i = 0; i < file_.dependency_count(); i++) {
      const FileDescriptor& dep_file(Deref(file_.dependency(i)));

      printer_.Print("require('`filename`');\n",
        "filename", FileDescriptorToPath(dep_file).c_str());
    }
    if (file_.dependency_count() > 0) {
      printer_.Print("\n");
    }

    PrintEnums();
    PrintMessages();
    PrintServices();

    if (UseNamespaces() && !file_.package().empty()) {
      printer_.Outdent();
      printer_.Print("}");
    }

  } catch (const std::string& msg) {
    *error = msg;
    return false;

  } catch (const char* msg) {
    error->assign(msg);
    return false;
  }

  return true;
}

