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
		string ClassName(const string & full_name) const;

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

const string& FieldName(const FieldDescriptor & field) {
  // Groups are hacky:  The name of the field is just the lower-cased name
  // of the group type.  In Java, though, we would like to retain the original
  // capitalization of the type name.
  if (field.type() == FieldDescriptor::TYPE_GROUP) {
    return field.message_type()->name();
  } else {
    return field.name();
  }
}


string UnderscoresToCamelCase(const FieldDescriptor & field) {
  return UnderscoresToCamelCaseImpl(FieldName(field), false);
}

string UnderscoresToCapitalizedCamelCase(const FieldDescriptor & field) {
  return UnderscoresToCamelCaseImpl(FieldName(field), true);
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
string PHPCodeGenerator::ClassName(const string & full_name) const {
	string name (full_name);
	replace(name.begin(), name.end(), '.', '_');
	return name;
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
      return ClassName(field.enum_type()->name()) + "::" + field.default_value_enum()->name();

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

		printer.Print("// message `name`\n",
		              "name", message.full_name());

		printer.Print("class `name` {\n",
		              "name", ClassName(message.full_name()));

		printer.Indent();

		// Print fields
                for (int i = 0; i < message.field_count(); ++i) {
                        printer.Print("\n");

			const FieldDescriptor &field ( *message.field(i) );

			map<string, string> variables;
			variables["name"]             = UnderscoresToCamelCase(field) + "_"; // Variable name
			variables["capitalized_name"] = UnderscoresToCapitalizedCamelCase(field);
			variables["number"]           = SimpleItoa(field.number());
			variables["comment"]          = field.DebugString();
			variables["default"]          = DefaultValueAsString(field, true);

			switch (field.type()) {
//				If its a enum we should store it as a int
//				case FieldDescriptor::TYPE_ENUM:
//					variables["type"] = field.enum_type()->name() + " ";
//					break;

				case FieldDescriptor::TYPE_MESSAGE:
				case FieldDescriptor::TYPE_GROUP:
					variables["type"] = ClassName(field.message_type()->full_name()) + " ";
					break;

				default:
					variables["type"] = "";
			}


			printer.Print(variables,
				"// `comment`\n"
				"private $`name` = null;\n"
				"public function has`capitalized_name`() { return !is_null($this->`name`); }\n"
				"public function clear`capitalized_name`() { $this->`name` = null; }\n"

				"public function get`capitalized_name`() { if(is_null($this->`name`)) return `default`; else return $this->`name`; }\n"
			);

			// TODO Change the set code to validate input depending on the variable type
			printer.Print(variables,
				"public function set`capitalized_name`(`type`$value) { $this->`name` = $value; }\n"
			);

                }

//		map<string, string> m;
//		m["descriptor_key"]  = kDescriptorKey;
//		m["descriptor_name"] = ModuleLevelDescriptorName(message);
//		printer.Print(m, "$descriptor_key$ = $descriptor_name$\n");

		// Class Insertion Point
		printer.Print(
			"\n"
			"// @@protoc_insertion_point(class_scope:`full_name`)\n",
			"full_name", message.full_name());

		printer.Outdent();
		printer.Print("}\n\n");
}

void PHPCodeGenerator::PrintEnum(io::Printer &printer, const EnumDescriptor & e) const {

		printer.Print("// enum `name`\n",
		              "name", e.full_name() );

		printer.Print("class `name` {\n",
		              "name", ClassName(e.full_name()) );

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

	printer.Print("<?php\n");

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
