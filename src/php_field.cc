/**
 * PHP Protocol Buffer Generator Plugin for protoc
 * By Andrew Brampton (c) 2010,2016
 */

#include "php_generator.h"

void PHPFileGenerator::PrintOneOfConstants(const Descriptor& message) {
  // Print all the OneOf Enums constants at the top of the message
  if (message.oneof_decl_count() > 0) {
    printer_.Print("const NONE = 0;\n");  // TODO There is a bug here if a field
                                          // is also named none

    for (int i = 0; i < message.oneof_decl_count(); ++i) {
      const OneofDescriptor& oneof(Deref(message.oneof_decl(i)));

      for (int j = 0; j < oneof.field_count(); ++j) {
        const FieldDescriptor& field(Deref(oneof.field(j)));

        printer_.Print("const `name` = `number`;\n",
          "name", OneOfConstant(field.name()),
          "number", SimpleItoa(field.number()));
      }
    }
  }
}

void PHPFileGenerator::PrintFields(const Descriptor& message) {
  if (!options_.handle_unknown()) printer_.Print("\nprivate $_unknown;\n");

  map<string, string> variables;

  for (int i = 0; i < message.oneof_decl_count(); ++i) {
    const OneofDescriptor& oneof(Deref(message.oneof_decl(i)));

    FieldVariables(oneof, variables);

    printer_.Print(variables,
                   "private $`oneof_name` = null; // `oneof_definition`\n"
                   "private $`oneof_case` = `oneof_default`;\n");
  }

  for (int i = 0; i < message.field_count(); ++i) {
    const FieldDescriptor& field(Deref(message.field(i)));

    // Skip over fields in oneof
    if (field.containing_oneof() != NULL) {
      assert(!field.is_repeated());  // Proto spec doesn't allow repeated oneof
                                     // fields. We assume this elsewhere, so
                                     // lets check.
      continue;
    }

    FieldVariables(field, variables);

    if (field.is_repeated()) {
      printer_.Print(variables, "private $`name` = array(); // `definition`\n");
    } else if (FieldHasHas(field)) {
      printer_.Print(variables, "private $`name` = null; // `definition`\n");
    } else {
      printer_.Print(variables, "private $`name` = `default`; // `definition`\n");
    }
  }
}
