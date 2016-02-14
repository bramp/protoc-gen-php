/**
 * PHP Protocol Buffer Generator Plugin for protoc
 * By Andrew Brampton (c) 2010,2016
 */
#include "php_generator.h"

#include <google/protobuf/wire_format.h>
#include <google/protobuf/wire_format_lite.h>
#include <google/protobuf/wire_format_lite_inl.h>

using google::protobuf::internal::WireFormat;
using google::protobuf::internal::WireFormatLite;  // TODO Should I be using
                                                   // internal?

void PHPFileGenerator::PrintSize(const Descriptor& message) {
  // Print the calc size method
  printer_.Print(
      "\n"
      "public function size() {\n"
      "  $size = 0;\n");
  printer_.Indent();

  map<string, string> variables;

  for (int i = 0; i < message.field_count(); ++i) {
    const FieldDescriptor& field(Deref(message.field(i)));

    FieldVariables(field, variables);
    string command;

    int len = WireFormat::TagSize(field.number(), field.type());

    switch (WireFormat::WireTypeForField(&field)) {
      case WireFormatLite::WIRETYPE_VARINT:
        if (field.type() == FieldDescriptor::TYPE_BOOL) {
          len++;  // A bool will always take 1 byte
          command = "$size += `len`;\n";
        } else {
          command = "$size += `len` + Protobuf::size_varint(`var`);\n";
        }
        break;

      case WireFormatLite::WIRETYPE_FIXED32:
        len += 4;
        command = "$size += `len`;\n";
        break;

      case WireFormatLite::WIRETYPE_FIXED64:
        len += 8;
        command = "$size += `len`;\n";
        break;

      case WireFormatLite::WIRETYPE_LENGTH_DELIMITED:
        if (field.type() == FieldDescriptor::TYPE_MESSAGE) {
          command = "$l = `var`->size();\n";
        } else {
          command = "$l = strlen(`var`);\n";
        }

        command += "$size += `len` + Protobuf::size_varint($l) + $l;\n";
        break;

      case WireFormatLite::WIRETYPE_START_GROUP:
      case WireFormatLite::WIRETYPE_END_GROUP:
        // WireFormat::TagSize returns the tag size * two when using groups, to
        // account for both the start and end tag
        command += "$size += `len` + `var`->size();\n";
        break;

      default:
        // TODO use the proper exception
        throw "Error " + field.full_name() + ": Unsupported wire type";
    }

    variables["len"] = SimpleItoa(len);

    if (field.is_repeated()) {
      // TODO Support packed size
      variables["var"] = "$v";
      printer_.Print(variables, "foreach($this->`name` as $v) {\n");

    } else {
      variables["var"] = "$this->" + VariableName(field.name());

      if (field.containing_oneof() != NULL) {
        const OneofDescriptor& oneof(Deref(field.containing_oneof()));

        variables["var"] = "$this->" + VariableName(oneof.name());
        printer_.Print(variables,
                       "if ($this->`oneof_case` === self::`oneof`) {\n");

      } else if (FieldHasHas(field)) {
        // TODO is_null needed. I think we can ensure the default value is null,
        // and remove this branch
        printer_.Print(variables, "if (!is_null(`var`)) {\n");
      } else {
        printer_.Print(variables, "if (`var` !== `default`) {\n");
      }
    }

    printer_.Indent();
    printer_.Print(variables, command.c_str());
    printer_.Outdent();
    printer_.Print("}\n");
  }

  printer_.Outdent();
  printer_.Print(
      "  return $size;\n"
      "}\n");
}
