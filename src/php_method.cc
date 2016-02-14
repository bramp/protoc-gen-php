/**
 * PHP Protocol Buffer Generator Plugin for protoc
 * By Andrew Brampton (c) 2010,2016
 */

#include "php_generator.h"

void PHPFileGenerator::PrintValidate(const Descriptor& message) {
  if (!SupportsRequiredValue()) {
    return;
  }

  // Validate that the required fields are included
  printer_.Print(
      "\n"
      "public function validate() {\n");
  printer_.Indent();

  for (int i = 0; i < message.field_count(); ++i) {
    const FieldDescriptor& field(Deref(message.field(i)));

    if (field.is_required()) {
      printer_.Print("if ($this->`name` === null) return false;\n",
        "name", VariableName(field.name()));
    }
  }

  printer_.Print("return true;\n");
  printer_.Outdent();
  printer_.Print("}\n");
}

void PHPFileGenerator::PrintSetterGetterMethods(const Descriptor& message) {
  map<string, string> variables;

  for (int i = 0; i < message.oneof_decl_count(); ++i) {
    const OneofDescriptor& oneof(Deref(message.oneof_decl(i)));

    FieldVariables(oneof, variables);
    printer_.Print("\n");

    printer_.Print(variables,
      "public function clear`oneof_capitalized_name`() { $this->`oneof_case` = `oneof_default`; $this->`oneof_name` = null; }\n"
      "public function has`oneof_capitalized_name`()`bool_return_type` { return $this->`oneof_case` !== `oneof_default`; }\n"
      "public function get`oneof_capitalized_name`() { if($this->`oneof_case` !== `oneof_default`) return $this->`oneof_name`; else return null; }\n"
      "public function get`oneof_capitalized_name`Case()`oneof_case_return_type` { return $this->`oneof_case`; }\n");
  }

  for (int i = 0; i < message.field_count(); ++i) {
    const FieldDescriptor& field(Deref(message.field(i)));

    FieldVariables(field, variables);
    printer_.Print("\n");

    // TODO Change the set code to validate input depending on the variable type
    // TODO Add comments to the methods
    // TODO Add @Deprecated tag
    if (field.containing_oneof() != NULL) {
      // A oneof field
      printer_.Print(variables,
        "public function clear`capitalized_name`() { if ($this->`oneof_case` === self::`oneof`) clear`oneof_capitalized_name`(); }\n"
        "public function has`capitalized_name`()`bool_return_type` { return $this->`oneof_case` === self::`oneof`; }\n"
        "public function get`capitalized_name`()`return_type` { if($this->`oneof_case` === self::`oneof`) return $this->`oneof_name`; else return `default`;}\n"
        "public function set`capitalized_name`(`type`$value) { $this->`oneof_case` = self::`oneof`; $this->`oneof_name` = $value; }\n");

    } else if (field.is_repeated()) {
      // Repeated field
      printer_.Print(variables,
        "public function clear`capitalized_name`() { $this->`name` = array(); }\n"

        "public function get`capitalized_name`Count()`int_return_type` { return count($this->`name`); }\n"
        "public function get`capitalized_name`(`int_type`$index)`return_type` { return $this->`name`[$index]; }\n"
        "public function get`capitalized_name`Array()`array_return_type` { return $this->`name`; }\n"

        // TODO Change the set code to validate input depending on
        // the variable type
        "public function set`capitalized_name`(`int_type`$index, `type`$value) {$this->`name`[$index] = $value; }\n"
        "public function add`capitalized_name`(`type`$value) { $this->`name`[] = $value; }\n"
        "public function addAll`capitalized_name`(`array_type`$values) { foreach($values as $value) {$this->`name`[] = $value; }}\n");

    } else if (IsProto2()) {
      printer_.Print(variables,
        "public function clear`capitalized_name`() { $this->`name` = null; }\n"
        "public function has`capitalized_name`()`bool_return_type` { return $this->`name` !== null; }\n"
        "public function get`capitalized_name`()`return_type` { if($this->`name` !== null) return $this->`name`; else return `default`;}\n"
        "public function set`capitalized_name`(`type`$value) { $this->`name` = $value; }\n");

    } else { // IsProto3
      printer_.Print(
        variables,
        "public function clear`capitalized_name`() { $this->`name` = `default`; }\n"
        "public function get`capitalized_name`()`return_type` { return $this->`name`;}\n"
        "public function set`capitalized_name`(`type`$value) { $this->`name` = $value; }\n");
    }
  }
}
