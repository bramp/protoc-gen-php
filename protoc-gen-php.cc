#include "strutil.h" // TODO This header is from the offical protobuf source, but it is not normally installed

#include <map>
#include <string>

#include <google/protobuf/descriptor.h>

#include <google/protobuf/compiler/plugin.h>
#include <google/protobuf/compiler/code_generator.h>

#include <google/protobuf/io/printer.h>
#include <google/protobuf/io/zero_copy_stream.h>


using namespace google::protobuf;
using namespace google::protobuf::compiler;

class PHPCodeGenerator : public CodeGenerator {
	private:

		void PrintMessage(io::Printer &printer, const Descriptor & message) const;
		void PrintMessages(io::Printer &printer, const FileDescriptor* file) const;

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

string FileClassName(const FileDescriptor & file) {
    string basename;
    string::size_type last_slash = file.name().find_last_of('/');
    if (last_slash == string::npos) {
      basename = file.name();
    } else {
      basename = file.name().substr(last_slash + 1);
    }
    return UnderscoresToCamelCaseImpl(StripProto(basename), true);
}

string FileJavaPackage(const FileDescriptor & file) {
    string result = "";//kDefaultPackage;
    if (!file.package().empty()) {
      if (!result.empty()) result += '.';
      result += file.package();
    }
    return result;
}

string ClassName(const FileDescriptor & descriptor) {
  string result = FileJavaPackage(descriptor);
  if (!result.empty()) result += '.';
  result += FileClassName(descriptor);
  return result;
}

string ToJavaName(const string& full_name, const FileDescriptor & file) {
  string result = ClassName(file);
  if (!result.empty()) {
    result += '.';
  }
  if (file.package().empty()) {
    result += full_name;
  } else {
    // Strip the proto package from full_name since we've replaced it with
    // the Java package.
    result += full_name.substr(file.package().size() + 1);
  }
  return result;
}

// These return the fully-qualified class name corresponding to the given
// descriptor.
string ClassName(const Descriptor & descriptor) {
  return ToJavaName(descriptor.full_name(), *descriptor.file());
}
string ClassName(const EnumDescriptor & descriptor) {
  return ToJavaName(descriptor.full_name(), *descriptor.file());
}
string ClassName(const ServiceDescriptor & descriptor) {
  return ToJavaName(descriptor.full_name(), *descriptor.file());
}

void PHPCodeGenerator::PrintMessage(io::Printer &printer, const Descriptor & message) const {

		printer.Print("class $name$ {\n", "name", message.name());
		printer.Indent();

		// Print nested messages
		for (int i = 0; i < message.nested_type_count(); ++i) {
			printer.Print("\n");
			PrintMessage(printer, *message.nested_type(i));
	        }

		// Print fields
                for (int i = 0; i < message.field_count(); ++i) {
                        printer.Print("\n");

			const FieldDescriptor &field ( *message.field(i) );

			map<string, string> variables;
			variables["name"]             = UnderscoresToCamelCase(field);
			variables["capitalized_name"] = UnderscoresToCapitalizedCamelCase(field);
			variables["number"]           = SimpleItoa(field.number());
			//variables["type"]             = ClassName(*field.message_type());
			variables["type"]             = "TYPE";
			variables["group_or_message"] = (field.type() == FieldDescriptor::TYPE_GROUP) ? "Group" : "Message";

			printer.Print(variables,
				"private $type$ $name$_ = null;\n"
				"public boolean has$capitalized_name$() { return !is_null($name$_); }\n"
				"public $type$ get$capitalized_name$() { return $name$_; }\n");

                }

//		map<string, string> m;
//		m["descriptor_key"]  = kDescriptorKey;
//		m["descriptor_name"] = ModuleLevelDescriptorName(message);
//		printer.Print(m, "$descriptor_key$ = $descriptor_name$\n");

		// Class Insertion Point
		printer.Print(
			"\n"
			"# @@protoc_insertion_point(class_scope:$full_name$)\n",
			"full_name", message.full_name());

		printer.Outdent();
		printer.Print("}\n");
}

void PHPCodeGenerator::PrintMessages(io::Printer &printer, const FileDescriptor* file) const {
	for (int i = 0; i < file->message_type_count(); ++i) {
		PrintMessage(printer, *file->message_type(i));
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

	io::Printer printer(output.get(), '$');

	PrintMessages(printer, file);

	return true;
}

int main(int argc, char* argv[]) {
	PHPCodeGenerator generator;
	return PluginMain(argc, argv, &generator);
}
