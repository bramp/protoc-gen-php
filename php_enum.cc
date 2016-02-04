/**
 * PHP Protocol Buffer Generator Plugin for protoc
 * By Andrew Brampton (c) 2010,2016
 */
 
#include "php_generator.h"


void PHPFileGenerator::PrintEnums() {
  for (int i = 0; i < file_.enum_type_count(); ++i) {
    PrintEnum(Deref(file_.enum_type(i)));
  }
}

void PHPFileGenerator::PrintEnum(const EnumDescriptor& e) {
  printer_.Print(
      "// enum `full_name`\n"
      "abstract class `name` extends ProtobufEnum {\n",
      "full_name", e.full_name(),
      "name", ClassName(e));

  printer_.Indent();

  // Print fields
  for (int j = 0; j < e.value_count(); ++j) {
    const EnumValueDescriptor& value(*e.value(j));

    printer_.Print("const `name` = `number`;\n",
      "name", UpperString(value.name()),
      "number", SimpleItoa(value.number()));
  }

  // Print values array
  printer_.Print("\npublic static $_values = array(\n");
  printer_.Indent();
  for (int j = 0; j < e.value_count(); ++j) {
    const EnumValueDescriptor& value(*e.value(j));

    printer_.Print("`number` => \"`name`\",\n",
      "number", SimpleItoa(value.number()),
      "name", PHPEscape(value.name()));
  }
  printer_.Outdent();
  printer_.Print(");\n\n");

  printer_.Print(  // TODO Move this into a base enum class - but check how
                   // static functions are inhertieted
      "public static function isValid($value) {\n"
      "  return array_key_exists($value, self::$_values);\n"
      "}\n\n");

  // Print a toString. // TODO Change to __toString()
  printer_.Print(
      "public static function toString($value) {\n"
      "  checkArgument(is_int($value), 'value must be a integer');\n"
      //"  if (is_null($value)) return null;\n"
      "  if (array_key_exists($value, self::$_values))\n"
      "    return self::$_values[$value];\n"
      "  return 'UNKNOWN';\n"
      "}\n");

  printer_.Outdent();
  printer_.Print("}\n\n");
}
