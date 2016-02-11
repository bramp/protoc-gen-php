/**
 * PHP Protocol Buffer Generator Plugin for protoc
 * By Andrew Brampton (c) 2010,2016
 */

#include "php_generator.h"

void PHPFileGenerator::PrintRead(const Descriptor& message,
                                 const FieldDescriptor* parentField) {
  // Read
  printer_.Print(
      "\n"
      "public function read($fp, &$limit = PHP_INT_MAX) {\n");
  printer_.Indent();

  printer_.Print(
    "$fp = ProtobufIO::toStream($fp, $limit);\n"
    "while(!feof($fp) && $limit > 0) {\n");
  printer_.Indent();

  printer_.Print(
      "$tag = Protobuf::read_varint($fp, $limit);\n"
      "if ($tag === false) break;\n"
      "$wire  = $tag & 0x07;\n"
      "$field = $tag >> 3;\n"
      //"var_dump(\"`name`: Found field: '$field' type: '\" .
      //Protobuf::get_wiretype($wire) . \"' $limit bytes left\");\n"
      "switch($field) {\n",
      "name", ClassName(message));
  printer_.Indent();

  // If we are a group message, we need to add a end group case
  if (parentField && parentField->type() == FieldDescriptor::TYPE_GROUP) {
    printer_.Print(
        "case `index`: //`name`\n",  // TODO BUG `name` is used but not set
        "index", SimpleItoa(parentField->number()));
    printer_.Print(
        "  ASSERT('$wire === 4');\n"
        "  break 2;\n");
  }

  for (int i = 0; i < message.field_count(); ++i) {
    const FieldDescriptor& field = Deref(message.field(i));

    int wire = -1;
    string commands;

    switch (field.type()) {
      case FieldDescriptor::TYPE_DOUBLE:  // double, exactly eight bytes on the
                                          // wire
        wire = 1;
        commands =
            "$tmp = Protobuf::read_double($fp, `$limit`);\n"
            "if ($tmp === false) throw new \\Exception('Protobuf::read_double "
            "returned false');\n"
            "$this->`name``[]` = $tmp;\n";
        break;

      case FieldDescriptor::TYPE_FLOAT:  // float, exactly four bytes on the
                                         // wire.
        wire = 5;
        commands =
            "$tmp = Protobuf::read_float($fp, `$limit`);\n"
            "if ($tmp === false) throw new \\Exception('Protobuf::read_float returned false');\n"
            "$this->`name``[]` = $tmp;\n";
        break;

      // TODO Check the range of all returned values
      case FieldDescriptor::TYPE_INT32:  // int32, varint on the wire.
        wire = 0;
        commands =
            "$tmp = Protobuf::read_signed_varint($fp, `$limit`);\n"
            "if ($tmp === false) throw new \\Exception('Protobuf::read_varint returned false');\n"
            // TODO The below test doesn't work right on 32bit PHP
            "if ($tmp < Protobuf::MIN_INT32 || $tmp > Protobuf::MAX_INT32) throw new \\Exception('int32 out of range');"
            "$this->`name``[]` = $tmp;\n";
        break;

      case FieldDescriptor::TYPE_INT64:  // int64, varint on the wire.
        wire = 0;
        commands =
            "$tmp = Protobuf::read_signed_varint($fp, `$limit`);\n"
            "if ($tmp === false) throw new \\Exception('Protobuf::read_varint returned false');\n"
            "if ($tmp < Protobuf::MIN_INT64 || $tmp > Protobuf::MAX_INT64) throw new \\Exception('int64 out of range');"
            "$this->`name``[]` = $tmp;\n";
        break;

      case FieldDescriptor::TYPE_UINT32:  // uint32, varint on the wire
        wire = 0;
        commands =
            "$tmp = Protobuf::read_varint($fp, `$limit`);\n"
            "if ($tmp === false) throw new \\Exception('Protobuf::read_varint returned false');\n"
            "if ($tmp < Protobuf::MIN_UINT32 || $tmp > Protobuf::MAX_UINT32) throw new \\Exception('uint32 out of range');"
            "$this->`name``[]` = $tmp;\n";
        break;

      case FieldDescriptor::TYPE_UINT64:  // uint64, varint on the wire.
        wire = 0;
        commands =
            "$tmp = Protobuf::read_varint($fp, `$limit`);\n"
            "if ($tmp === false) throw new \\Exception('Protobuf::read_varint returned false');\n"
            "if ($tmp < Protobuf::MIN_UINT64 || $tmp > Protobuf::MAX_UINT64) throw new \\Exception('uint64 out of range');"
            "$this->`name``[]` = $tmp;\n";
        break;

      case FieldDescriptor::TYPE_ENUM:  // Enum, varint on the wire
        wire = 0;
        commands =
            "$tmp = Protobuf::read_varint($fp, `$limit`);\n"
            "if ($tmp === false) throw new \\Exception('Protobuf::read_varint returned false');\n"
            // TODO Check $tmp is within the enum range
            "$this->`name``[]` = $tmp;\n";
        break;

      case FieldDescriptor::TYPE_FIXED64:  // uint64, exactly eight bytes on the
                                           // wire.
        wire = 1;
        commands =
            "$tmp = Protobuf::read_uint64($fp, `$limit`);\n"
            "if ($tmp === false) throw new \\Exception('Protobuf::read_unint64 returned false');\n"
            "$this->`name``[]` = $tmp;\n";
        break;

      case FieldDescriptor::TYPE_SFIXED64:  // int64, exactly eight bytes on the
                                            // wire
        wire = 1;
        commands =
            "$tmp = Protobuf::read_int64($fp, `$limit`);\n"
            "if ($tmp === false) throw new \\Exception('Protobuf::read_int64 returned false');\n"
            "$this->`name``[]` = $tmp;\n";
        break;

      case FieldDescriptor::TYPE_FIXED32:  // uint32, exactly four bytes on the
                                           // wire.
        wire = 5;
        commands =
            "$tmp = Protobuf::read_uint32($fp, `$limit`);\n"
            "if ($tmp === false) throw new \\Exception('Protobuf::read_uint32 returned false');\n"
            "$this->`name``[]` = $tmp;\n";
        break;

      case FieldDescriptor::TYPE_SFIXED32:  // int32, exactly four bytes on the
                                            // wire
        wire = 5;
        commands =
            "$tmp = Protobuf::read_int32($fp, `$limit`);\n"
            "if ($tmp === false) throw new \\Exception('Protobuf::read_int32 returned false');\n"
            "$this->`name``[]` = $tmp;\n";
        break;

      case FieldDescriptor::TYPE_BOOL:  // bool, varint on the wire.
        wire = 0;
        commands =
            "$tmp = Protobuf::read_varint($fp, `$limit`);\n"
            "if ($tmp === false) throw new \\Exception('Protobuf::read_varint returned false');\n"
            "$this->`name``[]` = ($tmp > 0) ? true : false;\n";
        break;

      case FieldDescriptor::TYPE_STRING:  // UTF-8 text.
      case FieldDescriptor::TYPE_BYTES:   // Arbitrary byte array.
        wire = 2;
        commands =
            "$len = Protobuf::read_varint($fp, `$limit`);\n"
            "if ($len === false) throw new \\Exception('Protobuf::read_varint returned false');\n"
            "$tmp = Protobuf::read_bytes($fp, $len, `$limit`);\n"
            "if ($tmp === false) throw new \\Exception(\"read_bytes($len) returned false\");\n"
            "$this->`name``[]` = $tmp;\n";
        break;

      case FieldDescriptor::TYPE_GROUP: {  // Tag-delimited message. Deprecated.
        const Descriptor& d(Deref(field.message_type()));
        wire = 3;
        commands =
            "$this->`name``[]` = new " + FullClassName(d) + "($fp, `$limit`);\n";
        break;
      }

      case FieldDescriptor::TYPE_MESSAGE: {  // Length-delimited message.
        const Descriptor& d(Deref(field.message_type()));
        wire = 2;
        commands =
            "$len = Protobuf::read_varint($fp, `$limit`);\n"
            "if ($len === false) throw new \\Exception('Protobuf::read_varint returned false');\n"
            "`$limit` -= $len;\n"
            "$this->`name``[]` = new " + FullClassName(d) + "($fp, $len);\n"
            "if ($len !== 0) throw new \\Exception('new " + FullClassName(d) + " did not read the full length');\n";
        break;
      }

      case FieldDescriptor::TYPE_SINT32:  // int32, ZigZag-encoded varint on the
                                          // wire
        wire = 5;
        commands =
            "$tmp = Protobuf::read_zint32($fp, `$limit`);\n"
            "if ($tmp === false) throw new \\Exception('Protobuf::read_zint32 returned false');\n"
            "$this->`name``[]` = $tmp;\n";
        break;

      case FieldDescriptor::TYPE_SINT64:  // int64, ZigZag-encoded varint on the
                                          // wire
        wire = 1;
        commands =
            "$tmp = Protobuf::read_zint64($fp, `$limit`);\n"
            "if ($tmp === false) throw new \\Exception('Protobuf::read_zint64 returned false');\n"
            "$this->`name``[]` = $tmp;\n";
        break;

      default:
        // TODO use the proper exception
        throw "Error " + field.full_name() + ": Unsupported type";
    }

    map<string, string> variables;
    FieldVariables(field, variables);
    variables["$limit"] = "$limit";

    printer_.Print("case `index`: // `definition`\n", "index",
                   SimpleItoa(field.number()), "definition",
                   TrimString(field.DebugString()));
    printer_.Indent();

    if (field.is_packable()) {  // is_repeated() && IsTypePackable(type())

      // the packable value can only be a primitive type
      assert(wire == 0 || wire == 1 || wire == 5);

      printer_.Print(
          "if($wire !== 2 && $wire !== `wire`) {\n"
          "  throw new \\Exception(\"Incorrect wire format for field $field, "
          "expected: 2 or `wire` got: $wire\");\n"
          "}\n"
          "if ($wire === `wire`) {\n",
          "wire", SimpleItoa(wire));
      printer_.Indent();
      printer_.Print(variables, commands.c_str());
      printer_.Outdent();
      printer_.Print(
          "} elseif ($wire === 2) {\n"
          "  $len = Protobuf::read_varint($fp, $limit);\n"
          //"  $limit -= $len;\n"
          "  while ($len > 0) {\n");

      printer_.Indent();
      printer_.Indent();
      variables["$limit"] = "$len";  // Switch this to len as we are in a loop,
                                     // and no longer using limit
      printer_.Print(variables, commands.c_str());
      printer_.Outdent();
      printer_.Outdent();

      printer_.Print(
          "  }\n"
          "}\n");
    } else {
      printer_.Print(
          "if($wire !== `wire`) {\n"
          "  throw new \\Exception(\"Incorrect wire format for field $field, "
          "expected: `wire` got: $wire\");\n"
          "}\n",
          "wire", SimpleItoa(wire));

      printer_.Print(variables, commands.c_str());

      if (field.containing_oneof()) {
        printer_.Print(variables, "$this->`oneof_case` = self::`oneof`;\n");
      }
    }

    printer_.Print("\nbreak;\n");
    printer_.Outdent();
  }

  if (options_.handle_unknown()) {
    printer_.Print(
        "default:\n"
        "  $field_idx = $field . '-' . Protobuf::get_wiretype($wire);\n"
        "  $this->_unknown[$field_idx][] = Protobuf::read_field($fp, $wire, $limit);\n",
        "name", ClassName(message));
  } else {
    printer_.Print(
        "default:\n"
        "  $limit -= Protobuf::skip_field($fp, $wire);\n",
        "name", ClassName(message));
  }

  printer_.Outdent();
  printer_.Outdent();
  printer_.Print(
      "  }\n"  // switch
      "}\n"    // while
      );

  if (SupportsRequiredValue()) {
    printer_.Print(
        "if (!$this->validate()) throw new \\Exception('Required fields are missing');\n");
  }

  printer_.Outdent();
  printer_.Print("}\n");
}
