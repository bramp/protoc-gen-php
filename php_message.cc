/**
 * PHP Protocol Buffer Generator Plugin for protoc
 * By Andrew Brampton (c) 2010,2016
 */
#include "php_generator.h"

void PHPFileGenerator::PrintMessages() {
  for (int i = 0; i < file_.message_type_count(); ++i) {
    PrintMessage(Deref(file_.message_type(i)));
  }
}

void PHPFileGenerator::PrintMessage(const Descriptor& message) {
  // Print nested messages
  for (int i = 0; i < message.nested_type_count(); ++i) {
    printer_.Print("\n");
    PrintMessage(Deref(message.nested_type(i)));
  }

  // Print nested enum
  for (int i = 0; i < message.enum_type_count(); ++i) {
    PrintEnum(Deref(message.enum_type(i)));
  }

  // Find out if we are a nested type, if so what kind
  const FieldDescriptor* parentField = NULL;
  const char* type = "message";
  if (message.containing_type() != NULL) {
    const Descriptor& parent(Deref(message.containing_type()));

    // Find which field we are
    for (int i = 0; i < parent.field_count(); ++i) {
      if (Deref(parent.field(i)).message_type() == &message) {
        parentField = parent.field(i);
        break;
      }
    }
    if (parentField && (parentField->type() == FieldDescriptor::TYPE_GROUP))
      type = "group";
  }

  // Start printing the message
  printer_.Print(
      "// `type` `full_name`\n"
      "final class `name` extends ProtobufMessage {\n",
      "name", ClassName(message), "type", type, "full_name",
      message.full_name());
  printer_.Indent();

  // Print fields map
  /*
  printer_.Print(
          "// Array maps field indexes to members\n"
          "private static $_map = array (\n"
  );
  printer_.Indent();
  for (int i = 0; i < message.field_count(); ++i) {
          const FieldDescriptor &field ( *message.field(i) );

          printer_.Print("`index` => '`value`',\n",
                  "index", SimpleItoa(field.number()),
                  "value", VariableName(field)
          );
  }
  printer_.Outdent();
  printer_.Print(");\n\n");
  */

  PrintOneOfConstants(message);

  PrintFields(message);

  // Constructor
  printer_.Print(
      "\n"  // TODO add comments
      "public function __construct($in = NULL, &$limit = PHP_INT_MAX) {\n"
      "  parent::__construct($in, $limit);\n"
      "}\n");

  // All the methods
  PrintRead(message, parentField);
  PrintWrite(message);
  PrintSize(message);
  PrintValidate(message);
  PrintSetterGetterMethods(message);
  PrintToString(message);

  // Class Insertion Point
  printer_.Print(
      "\n"
      "// @@protoc_insertion_point(class_scope:`full_name`)\n",
      "full_name", message.full_name());

  printer_.Outdent();
  printer_.Print("}\n\n");
}
