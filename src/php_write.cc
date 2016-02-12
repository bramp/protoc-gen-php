/**
 * PHP Protocol Buffer Generator Plugin for protoc
 * By Andrew Brampton (c) 2010,2016
 */

#include "php_generator.h"

/**
 * Prints the write() method for this Message
 * @param printer
 * @param message
 * @param parentField
 */
void PHPFileGenerator::PrintWrite(const Descriptor& message) {
  // Write
  printer_.Print(
      "\n"
      "public function write($fp) {\n");
  printer_.Indent();

  if (SupportsRequiredValue()) {
    printer_.Print(
        // TODO Add list of missing fields
        "if (!$this->validate())\n"
        "  throw new \\Exception('Required fields are missing');\n");
  }

  map<string, string> variables;

  for (int i = 0; i < message.field_count(); ++i) {
    const FieldDescriptor& field(Deref(message.field(i)));

    FieldVariables(field, variables);

    // TODO We don't support writing packed fields

    string commands;
    switch (field.type()) {
      case FieldDescriptor::TYPE_DOUBLE:  // double, exactly eight bytes on the
                                          // wire
        commands = "Protobuf::write_double($fp, `var`);\n";
        break;

      case FieldDescriptor::TYPE_FLOAT:  // float, exactly four bytes on the
                                         // wire.
        commands = "Protobuf::write_float($fp, `var`);\n";
        break;

      case FieldDescriptor::TYPE_INT64:   // int64, varint on the wire.
      case FieldDescriptor::TYPE_UINT64:  // uint64, varint on the wire.
      case FieldDescriptor::TYPE_INT32:   // int32, varint on the wire.
      case FieldDescriptor::TYPE_UINT32:  // uint32, varint on the wire
      case FieldDescriptor::TYPE_ENUM:    // Enum, varint on the wire
        commands = "Protobuf::write_varint($fp, `var`);\n";
        break;

      case FieldDescriptor::TYPE_FIXED64:  // uint64, exactly eight bytes on the
                                           // wire.
        commands = "Protobuf::write_uint64($fp, `var`);\n";
        break;

      case FieldDescriptor::TYPE_SFIXED64:  // int64, exactly eight bytes on the
                                            // wire
        commands = "Protobuf::write_int64($fp, `var`);\n";
        break;

      case FieldDescriptor::TYPE_FIXED32:  // uint32, exactly four bytes on the
                                           // wire.
        commands = "Protobuf::write_uint32($fp, `var`);\n";
        break;

      case FieldDescriptor::TYPE_SFIXED32:  // int32, exactly four bytes on the
                                            // wire
        commands = "Protobuf::write_int32($fp, `var`);\n";
        break;

      case FieldDescriptor::TYPE_BOOL:  // bool, varint on the wire.
        // TODO Change to be raw encoded values for 1 and 0
        commands = "Protobuf::write_varint($fp, `var` ? 1 : 0);\n";
        break;

      case FieldDescriptor::TYPE_STRING:  // UTF-8 text.
      case FieldDescriptor::TYPE_BYTES:   // Arbitrary byte array.
        commands =
            "Protobuf::write_varint($fp, strlen(`var`));\n"
            "fwrite($fp, `var`);\n";
        break;

      case FieldDescriptor::TYPE_GROUP: {  // Tag-delimited message. Deprecated.
        // The start tag has already been printed, but also print the end tag
        string endTag =
            MakeTag(field.number(), WireFormatLite::WIRETYPE_END_GROUP);
        commands =
            "`var`->write($fp); // group\n"
            "fwrite($fp, \"" + PHPEscape(endTag) + "\", " + SimpleItoa(endTag.size()) + ");\n";
        break;
      }
      case FieldDescriptor::TYPE_MESSAGE:  // Length-delimited message.
        commands =
            "Protobuf::write_varint($fp, `var`->size());\n"
            "`var`->write($fp);\n";
        break;

      case FieldDescriptor::TYPE_SINT32:  // int32, ZigZag-encoded varint on the
                                          // wire
        commands = "Protobuf::write_zint32($fp, `var`);\n";
        break;

      case FieldDescriptor::TYPE_SINT64:  // int64, ZigZag-encoded varint on the
                                          // wire
        commands = "Protobuf::write_zint64($fp, `var`);\n";
        break;

      default:
        // TODO use the proper exception
        throw "Error " + field.full_name() + ": Unsupported type";
    }

    if (field.is_repeated()) {
      // TODO If we store default values in the repeated field, we will seralise
      // them
      variables["var"] = "$v";
      printer_.Print(variables, "foreach($this->`name` as $v) {\n");

    } else {
      // variables["var"] = VariableName(field.name());
      variables["var"] = "$this->" + VariableName(field.name());

      if (field.containing_oneof() != nullptr) {
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

    string tag = MakeTag(field);
    variables["tag"] = PHPEscape(tag);
    variables["tagLen"] = SimpleItoa(tag.size());

    // TODO Change here for packed variables
    printer_.Indent();
    printer_.Print(variables, "fwrite($fp, \"`tag`\", `tagLen`);\n");
    printer_.Print(variables, commands.c_str());
    printer_.Outdent();
    printer_.Print("}\n");
  }

  printer_.Outdent();
  printer_.Print("}\n");
}
