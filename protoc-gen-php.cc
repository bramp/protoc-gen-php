#include "strutil.h" // TODO This header is from the offical protobuf source, but it is not normally installed

#include <map>
#include <string>
#include <algorithm>

#include <google/protobuf/descriptor.h>

#include <google/protobuf/compiler/plugin.h>
#include <google/protobuf/compiler/code_generator.h>

#include <google/protobuf/io/printer.h>
#include <google/protobuf/io/zero_copy_stream.h>


using namespace google::protobuf;
using namespace google::protobuf::compiler;

class PHPCodeGenerator : public CodeGenerator {
	private:

		void PrintMessage   (io::Printer &printer, const Descriptor & message) const;
		void PrintMessages  (io::Printer &printer, const FileDescriptor & file) const;

		void PrintEnum      (io::Printer &printer, const EnumDescriptor & e) const;
		void PrintEnums     (io::Printer &printer, const FileDescriptor & file) const;

		void PrintService   (io::Printer &printer, const ServiceDescriptor & service) const;
		void PrintServices  (io::Printer &printer, const FileDescriptor & file) const;

		string DefaultValueAsString(const FieldDescriptor & field, bool quote_string_type) const;

		// Maps names into PHP names
		template <class DescriptorType>
		string ClassName(const DescriptorType & descriptor) const;

		string VariableName(const FieldDescriptor & field) const;

	public:

		PHPCodeGenerator();
		~PHPCodeGenerator();

		bool Generate(const FileDescriptor* file, const string& parameter, OutputDirectory* output_directory, string* error) const;

};

PHPCodeGenerator::PHPCodeGenerator() {}
PHPCodeGenerator::~PHPCodeGenerator() {}

string UnderscoresToCamelCaseImpl(const string& input, bool cap_next_letter) {
  string result;
  // Note:  I distrust ctype.h due to locales.
  for (int i = 0; i < input.size(); i++) {
    if ('a' <= input[i] && input[i] <= 'z') {
      if (cap_next_letter) {
        result += input[i] + ('A' - 'a');
      } else {
        result += input[i];
      }
      cap_next_letter = false;
    } else if ('A' <= input[i] && input[i] <= 'Z') {
      if (i == 0 && !cap_next_letter) {
        // Force first letter to lower-case unless explicitly told to
        // capitalize it.
        result += input[i] + ('a' - 'A');
      } else {
        // Capital letters after the first are left as-is.
        result += input[i];
      }
      cap_next_letter = false;
    } else if ('0' <= input[i] && input[i] <= '9') {
      result += input[i];
      cap_next_letter = true;
    } else {
      cap_next_letter = true;
    }
  }
  return result;
}

string UnderscoresToCamelCase(const FieldDescriptor & field) {
  return UnderscoresToCamelCaseImpl(field.name(), false);
}

string UnderscoresToCapitalizedCamelCase(const FieldDescriptor & field) {
  return UnderscoresToCamelCaseImpl(field.name(), true);
}

string StripProto(const string& filename) {
  if (HasSuffixString(filename, ".protodevel")) {
    return StripSuffixString(filename, ".protodevel");
  } else {
    return StripSuffixString(filename, ".proto");
  }
}

string LowerString(const string & s) {
  string newS (s);
  LowerString(&newS);
  return newS;
}

string UpperString(const string & s) {
  string newS (s);
  UpperString(&newS);
  return newS;
}

// Maps a Message full_name into a PHP name
template <class DescriptorType>
string PHPCodeGenerator::ClassName(const DescriptorType & descriptor) const {
	string name (descriptor.full_name());
	replace(name.begin(), name.end(), '.', '_');
	return name;
}

string PHPCodeGenerator::VariableName(const FieldDescriptor & field) const {
	return UnderscoresToCamelCase(field) + '_';
}

string PHPCodeGenerator::DefaultValueAsString(const FieldDescriptor & field, bool quote_string_type) const {
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
        return "\"" + CEscape(field.default_value_string()) + "\"";

      if (field.type() == FieldDescriptor::TYPE_BYTES)
        return CEscape(field.default_value_string());

      return field.default_value_string();

    case FieldDescriptor::CPPTYPE_ENUM:
      return ClassName(*field.enum_type()) + "::" + field.default_value_enum()->name();

    case FieldDescriptor::CPPTYPE_MESSAGE:
      return "null";

  }
  return "";
}


void PHPCodeGenerator::PrintMessage(io::Printer &printer, const Descriptor & message) const {

		// Print nested messages
		for (int i = 0; i < message.nested_type_count(); ++i) {
			printer.Print("\n");
			PrintMessage(printer, *message.nested_type(i));
	        }

		// Print nested enum
		for (int i = 0; i < message.enum_type_count(); ++i) {
			PrintEnum(printer, *message.enum_type(i) );
		}

		printer.Print("// message `full_name`\n"
		              "class `name` extends Message {\n",
		              "full_name", message.full_name(),
		              "name", ClassName(message)
		);

		printer.Indent();

		// Print fields map
		printer.Print(
			"// Array maps field indexes to members\n"
			"private $_map = array (\n"
		);
		printer.Indent();
                for (int i = 0; i < message.field_count(); ++i) {
			const FieldDescriptor &field ( *message.field(i) );

			printer.Print("`index` => `value`,\n",
				"index", SimpleItoa(field.number()),
				"value", VariableName(field)
			);
		}
		printer.Outdent();
		printer.Print(");\n\n");

		vector<const FieldDescriptor *> required_fields;

		// Print fields variables and methods
                for (int i = 0; i < message.field_count(); ++i) {
                        printer.Print("\n");

			const FieldDescriptor &field ( *message.field(i) );

			if (field.is_required())
				required_fields.push_back( &field );

			map<string, string> variables;
			variables["name"]             = VariableName(field);
			variables["capitalized_name"] = UnderscoresToCapitalizedCamelCase(field);
			variables["comment"]          = field.DebugString();
			variables["default"]          = DefaultValueAsString(field, true);

			switch (field.type()) {
//				If its a enum we should store it as a int
//				case FieldDescriptor::TYPE_ENUM:
//					variables["type"] = field.enum_type()->name() + " ";
//					break;

				case FieldDescriptor::TYPE_MESSAGE:
				case FieldDescriptor::TYPE_GROUP:
					variables["type"] = ClassName(*field.message_type()) + " ";
					break;

				default:
					variables["type"] = "";
			}

			if (field.is_repeated()) {
				// Repeated field
				printer.Print(variables,
					"// `comment`\n"
					"private $`name` = null;\n"
					"public function clear`capitalized_name`() { $this->`name` = null; }\n"

					"public function get`capitalized_name`Count() { if ($this->`name` === null ) return 0; else return count($this->`name`); }\n"
					"public function get`capitalized_name`($index) { return $this->`name`[$index]; }\n"
					"public function get`capitalized_name`Array() { if ($this->`name` === null ) return array(); else return $this->`name`; }\n"

					"public function set`capitalized_name`($index, $value) {$this->`name`[$index] = $value;	}\n"
					"public function add`capitalized_name`($value) { $this->`name`[] = $value; }\n"
					"public function addAll`capitalized_name`(array $values) { foreach($values as $value) {$this->`name`[] = $value;} }\n"
				);

			} else {
				// Non repeated field
				printer.Print(variables,
					"// `comment`\n"
					"private $`name` = null;\n"
					"public function clear`capitalized_name`() { $this->`name` = null; }\n"
					"public function has`capitalized_name`() { return $this->`name` !== null; }\n"

					"public function get`capitalized_name`() { if($this->`name` === null) return `default`; else return $this->`name`; }\n"
				);

				// TODO Change the set code to validate input depending on the variable type
				printer.Print(variables,
					"public function set`capitalized_name`(`type`$value) { $this->`name` = $value; }\n"
				);
			}
                }

		// Validate required fields are included
		printer.Print(
			"\n"
			"public function validateRequired() {\n"
		);
		printer.Indent();
		for (int i = 0; i < required_fields.size(); ++i) {
			printer.Print("if ($this->`name` === null) return false;\n",
				"name", VariableName(*required_fields[i])
			);
		}
		printer.Print("return true;\n");
		printer.Outdent();
		printer.Print("}\n");

		// Class Insertion Point
		printer.Print(
			"\n"
			"// @@protoc_insertion_point(class_scope:`full_name`)\n",
			"full_name", message.full_name()
		);

		printer.Outdent();
		printer.Print("}\n\n");
}

void PHPCodeGenerator::PrintEnum(io::Printer &printer, const EnumDescriptor & e) const {

		printer.Print("// enum `full_name`\n"
		              "class `name` {\n",
		              "full_name", e.full_name(),
		              "name", ClassName(e)
		);

		printer.Indent();

		// Print fields
                for (int j = 0; j < e.value_count(); ++j) {
			const EnumValueDescriptor &value ( *e.value(j) );

			map<string, string> variables;
			variables["name"]   = UpperString(value.name());
			variables["number"] = SimpleItoa(value.number());

			printer.Print(variables,
				"const `name` = `number`;\n");

                }

		printer.Outdent();
		printer.Print("}\n\n");
}

void PHPCodeGenerator::PrintMessages(io::Printer &printer, const FileDescriptor & file) const {
	for (int i = 0; i < file.message_type_count(); ++i) {
		PrintMessage(printer, *file.message_type(i));
	}
}

void PHPCodeGenerator::PrintEnums(io::Printer &printer, const FileDescriptor & file) const {
	for (int i = 0; i < file.enum_type_count(); ++i) {
		PrintEnum(printer, *file.enum_type(i) );
	}
}

void PHPCodeGenerator::PrintServices(io::Printer &printer, const FileDescriptor & file) const {
	for (int i = 0; i < file.service_count(); ++i) {
		printer.Print("////\n//TODO Service\n////\n");
	}
}

bool PHPCodeGenerator::Generate(const FileDescriptor* file,
				const string& parameter,
				OutputDirectory* output_directory,
				string* error) const {

	string php_filename;
//	php_filename += file_generator.filename();
	php_filename += "test";
	php_filename += ".php";

	// Generate main file.
	scoped_ptr<io::ZeroCopyOutputStream> output(
		output_directory->Open(php_filename)
	);

	io::Printer printer(output.get(), '`');

	printer.Print(
		"<?php\n"
		"require('protocolbuffers.inc.php');\n"
	);

	PrintMessages  (printer, *file);
	PrintEnums     (printer, *file);
	PrintServices  (printer, *file);

	printer.Print("?>");

	return true;
}

int main(int argc, char* argv[]) {
	PHPCodeGenerator generator;
	return PluginMain(argc, argv, &generator);
}
