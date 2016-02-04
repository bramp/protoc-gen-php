/**
 * PHP Protocol Buffer Generator Plugin for protoc
 * By Andrew Brampton (c) 2010,2016
 */

#include "php_generator.h"

void PHPFileGenerator::PrintToString(const Descriptor& message) {
  printer_.Print(
      "\n"
      "public function __toString() {\n"
      "  return ''");
  printer_.Indent();

  if (options_.handle_unknown()) {
    printer_.Print(
        "\n     . Protobuf::toString('unknown', $this->_unknown, array())");
  }

  map<string, string> variables;

  for (int i = 0; i < message.oneof_decl_count(); ++i) {
    const OneofDescriptor& oneof(Deref(message.oneof_decl(i)));

    variables.clear();
    FieldVariables(oneof, variables);

    printer_.Print(variables,
                   "\n     . Protobuf::toString('`oneof_field`_case', "
                   "$this->`oneof_case`, `oneof_default`)"
                   "\n     . Protobuf::toString('`oneof_field`', "
                   "$this->`oneof_name`, null)");
  }

  for (int i = 0; i < message.field_count(); ++i) {
    const FieldDescriptor& field(Deref(message.field(i)));

    if (field.containing_oneof()) {
      continue;
    }

    FieldVariables(field, variables);

    printer_.Print(
        variables,
        "\n     . Protobuf::toString('`field`', $this->`name`, `default`)");
    /*
                    if (field.type() == FieldDescriptor::TYPE_ENUM) {
                            variables["enum"] =
       ClassName(Deref(field.enum_type()));
                            printer_.Print(variables,
                                    "\n     . Protobuf::toString('`field`',
       `enum`::toString($this->`name`))"
                            );
                    } else {
                            printer_.Print(variables,
                                    "\n     . Protobuf::toString('`field`',
       $this->`name`)"
                            );
                    }
    */
  }
  printer_.Print(";\n");
  printer_.Outdent();
  printer_.Print("}\n");
}
