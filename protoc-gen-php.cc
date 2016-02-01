/**
 * PHP Protocol Buffer Generator Plugin for protoc
 * By Andrew Brampton (c) 2010,2016
 *
 * TODO
 *  Extensions
 *  Services
 *  Packages
 *  Better validation (add code to check setted values are valid)
 *  option optimize_for = CODE_SIZE/SPEED;
 *  Place field names in the generated exceptions
 *  Add comments to the generated code.
 *  Add @Deprecated support
 *  Add extra methods to the built in types, such as a Timestamp Helper
 *  Support proto3 json mapping
 *  Add Reserved support
 *  Add Map Support
 *  Add Package (namespace support)
 *  Add proto option for which version of PHP we are targeting
 *  If PHP>7 Add more type hints to the generated code.
 *  Add write_bytes(...)
 *  Cleanup write/size code
 *  Add strict checking of UTF-8 in proto3
 *  Add to PEAR and/or Composer
 *  ToString needs a lot of work
 *  Implement a CodedInputStream and CodedOutputStream 
 *  Writing packed fields
 *  Support the autoloading standard
 *  Format the C++
 *  Format the PHP
 *  Have a mode where accurate integers work
 */

// TODO strutil.h is from the offical protobuf source, but it is not installed. Replace it
//      We use LowerString, UpperString, SimpleItoa, CEscape from it.
#include "strutil.h" 
#include "php_util.h"
#include "util.h"


#include <map>
#include <string>
#include <algorithm>
#include <cstdio> // for sprintf
#include <iostream>

#include <google/protobuf/descriptor.h>
#include <google/protobuf/wire_format.h>
#include <google/protobuf/wire_format_lite.h>
#include <google/protobuf/wire_format_lite_inl.h>


#include <google/protobuf/compiler/plugin.h>
#include <google/protobuf/compiler/code_generator.h>

#include <google/protobuf/io/printer.h>
#include <google/protobuf/io/zero_copy_stream.h>

#include "php_options.pb.h"

using std::string;

using namespace google::protobuf;
using namespace google::protobuf::compiler;
using namespace google::protobuf::internal;


class PHPCodeGenerator : public CodeGenerator {
	public:
		PHPCodeGenerator();
		virtual ~PHPCodeGenerator();

		bool Generate(const FileDescriptor* file, const string& parameter, GeneratorContext * context, string* error) const;
};

/**
 * Generates a single PHP file from a proto file
 */
class PHPFileGenerator {
	public:

		PHPFileGenerator(io::Printer & printer, const FileDescriptor & file, const string & parameter);
		virtual ~PHPFileGenerator();

		bool Generate(string* error);

	private:
		void PrintEnums();
		void PrintMessages();
		void PrintServices();

		void PrintMessage   (const Descriptor & message);
		void PrintEnum      (const EnumDescriptor & enu);
		void PrintService   (const ServiceDescriptor & service);

		void PrintOneOfConstants(const Descriptor & message);
		void PrintFields    (const Descriptor & message);
		void PrintSetterGetterMethods(const Descriptor & message);

		// Print the read() method
		void PrintRead(const Descriptor & message, const FieldDescriptor * parentField);

		// Print the write() method
		void PrintWrite(const Descriptor & message);

		// Print the size() method
		void PrintSize(const Descriptor & message);

		// Print the validate() method
		void PrintValidate(const Descriptor & message);

		// Print the __toString() method
		void PrintToString(const Descriptor & message);

		bool IsProto2() const {
			return file_.syntax() == FileDescriptor::SYNTAX_PROTO2;
		}

		bool IsProto3() const {
			return file_.syntax() == FileDescriptor::SYNTAX_PROTO3;
		}

		// Does this protobuf support the required field
		bool SupportsRequiredValue() const {
			return IsProto2();
		}

		//
		// Members
		//
		io::Printer & printer_;
		const FileDescriptor & file_;
		const string & parameter_;

		const PHPFileOptions & options_;
		const string namespace_;
		const char * ns_;

};

PHPFileGenerator::PHPFileGenerator(io::Printer & printer, const FileDescriptor & file, const string& parameter) :
		printer_(printer), file_(file), parameter_(parameter),

		// Parse the file options
		options_(file.options().GetExtension(php)), namespace_(options_.namespace_()), ns_(namespace_.empty() ? "" : "\\") {

			(void) parameter_; // Unused
}

PHPFileGenerator::~PHPFileGenerator() {}


void PHPFileGenerator::PrintRead(const Descriptor & message, const FieldDescriptor * parentField) {

	// Read
	printer_.Print(
		"\n"
		"public function read($fp, &$limit = PHP_INT_MAX) {\n"
	);
	printer_.Indent();

	printer_.Print("while(!feof($fp) && $limit > 0) {\n");
	printer_.Indent();

	printer_.Print(
		"$tag = `ns`Protobuf::read_varint($fp, $limit);\n"
		"if ($tag === false) break;\n"
		"$wire  = $tag & 0x07;\n"
		"$field = $tag >> 3;\n"
		//"var_dump(\"`name`: Found field: '$field' type: '\" . `ns`Protobuf::get_wiretype($wire) . \"' $limit bytes left\");\n"
		"switch($field) {\n",
		"name", ClassName(message),
		"ns", ns_
	);
	printer_.Indent();

	// If we are a group message, we need to add a end group case
	if (parentField && parentField->type() == FieldDescriptor::TYPE_GROUP) {
		printer_.Print("case `index`: //`name`\n", // TODO BUG `name` is used but not set
			"index", SimpleItoa(parentField->number())
		);
		printer_.Print( "  ASSERT('$wire === 4');\n"
					   "  break 2;\n");
	}

	for (int i = 0; i < message.field_count(); ++i) {
		const FieldDescriptor & field = Deref(message.field(i));

		int wire = -1;
		string commands;

		switch (field.type()) {
			case FieldDescriptor::TYPE_DOUBLE: // double, exactly eight bytes on the wire
				wire = 1;
				commands = "$tmp = `ns`Protobuf::read_double($fp, `$limit`);\n"
						   "if ($tmp === false) throw new Exception('Protobuf::read_double returned false');\n"
						   "$this->`name``[]` = $tmp;\n";
				break;

			case FieldDescriptor::TYPE_FLOAT: // float, exactly four bytes on the wire.
				wire = 5;
				commands = "$tmp = `ns`Protobuf::read_float($fp, `$limit`);\n"
						   "if ($tmp === false) throw new Exception('Protobuf::read_float returned false');\n"
						   "$this->`name``[]` = $tmp;\n";
				break;

			case FieldDescriptor::TYPE_INT64:  // int64, varint on the wire.
			case FieldDescriptor::TYPE_UINT64: // uint64, varint on the wire.
			case FieldDescriptor::TYPE_INT32:  // int32, varint on the wire.
			case FieldDescriptor::TYPE_UINT32: // uint32, varint on the wire
			case FieldDescriptor::TYPE_ENUM:   // Enum, varint on the wire
				wire = 0;
				commands = "$tmp = `ns`Protobuf::read_varint($fp, `$limit`);\n"
				           "if ($tmp === false) throw new Exception('Protobuf::read_varint returned false');\n"
				           "$this->`name``[]` = $tmp;\n";
				break;

			case FieldDescriptor::TYPE_FIXED64: // uint64, exactly eight bytes on the wire.
				wire = 1;
				commands = "$tmp = `ns`Protobuf::read_uint64($fp, `$limit`);\n"
						   "if ($tmp === false) throw new Exception('Protobuf::read_unint64 returned false');\n"
						   "$this->`name``[]` = $tmp;\n";
				break;

			case FieldDescriptor::TYPE_SFIXED64: // int64, exactly eight bytes on the wire
				wire = 1;
				commands = "$tmp = `ns`Protobuf::read_int64($fp, `$limit`);\n"
						   "if ($tmp === false) throw new Exception('Protobuf::read_int64 returned false');\n"
						   "$this->`name``[]` = $tmp;\n";
				break;

			case FieldDescriptor::TYPE_FIXED32: // uint32, exactly four bytes on the wire.
				wire = 5;
				commands = "$tmp = `ns`Protobuf::read_uint32($fp, `$limit`);\n"
						   "if ($tmp === false) throw new Exception('Protobuf::read_uint32 returned false');\n"
						   "$this->`name``[]` = $tmp;\n";
				break;

			case FieldDescriptor::TYPE_SFIXED32: // int32, exactly four bytes on the wire
				wire = 5;
				commands = "$tmp = `ns`Protobuf::read_int32($fp, `$limit`);\n"
						   "if ($tmp === false) throw new Exception('Protobuf::read_int32 returned false');\n"
						   "$this->`name``[]` = $tmp;\n";
				break;

			case FieldDescriptor::TYPE_BOOL: // bool, varint on the wire.
				wire = 0;
				commands = "$tmp = `ns`Protobuf::read_varint($fp, `$limit`);\n"
				           "if ($tmp === false) throw new Exception('Protobuf::read_varint returned false');\n"
				           "$this->`name``[]` = ($tmp > 0) ? true : false;\n";
				break;

			case FieldDescriptor::TYPE_STRING:  // UTF-8 text.
			case FieldDescriptor::TYPE_BYTES:   // Arbitrary byte array.
				wire = 2;
				commands = "$len = `ns`Protobuf::read_varint($fp, `$limit`);\n"
				           "if ($len === false) throw new Exception('Protobuf::read_varint returned false');\n"				           
						   "$tmp = `ns`Protobuf::read_bytes($fp, $len, `$limit`);\n"
				           "if ($tmp === false) throw new Exception(\"read_bytes($len) returned false\");\n"
						   "$this->`name``[]` = $tmp;\n";
				break;

			case FieldDescriptor::TYPE_GROUP: { // Tag-delimited message.  Deprecated.
				const Descriptor & d( Deref(field.message_type()) );
				wire = 3;
				commands = "$this->`name``[]` = new " + ClassName(d) + "($fp, `$limit`);\n";
				break;
			}

			case FieldDescriptor::TYPE_MESSAGE: { // Length-delimited message.
				const Descriptor & d( Deref(field.message_type()) );
				wire = 2;
				commands = "$len = `ns`Protobuf::read_varint($fp, `$limit`);\n"
				           "if ($len === false) throw new Exception('Protobuf::read_varint returned false');\n"
						   "`$limit` -= $len;\n"
						   "$this->`name``[]` = new " + ClassName(d) + "($fp, $len);\n"
						   "if ($len !== 0) throw new Exception('new " + ClassName(d) + " did not read the full length');\n";
				break;
			}

			case FieldDescriptor::TYPE_SINT32:   // int32, ZigZag-encoded varint on the wire
				wire = 5;
				commands = "$tmp = `ns`Protobuf::read_zint32($fp, `$limit`);\n"
				           "if ($tmp === false) throw new Exception('Protobuf::read_zint32 returned false');\n"
						   "$this->`name``[]` = $tmp;\n";
				break;

			case FieldDescriptor::TYPE_SINT64:   // int64, ZigZag-encoded varint on the wire
				wire = 1;
				commands = "$tmp = `ns`Protobuf::read_zint64($fp, `$limit`);\n"
				           "if ($tmp === false) throw new Exception('Protobuf::read_zint64 returned false');\n"
				           "$this->`name``[]` = $tmp;\n";
				break;

			default:
				// TODO use the proper exception
				throw "Error " + field.full_name() + ": Unsupported type";
		}

		map<string, string> variables;
		FieldVariables(field, variables);
		variables["ns"] = ns_;
		variables["$limit"] = "$limit";

		printer_.Print("case `index`: // `definition`\n",
			"index", SimpleItoa(field.number()),
			"definition", TrimString(field.DebugString())
		);
		printer_.Indent();

		if (field.is_packable()) { // is_repeated() && IsTypePackable(type())
			
			assert(wire == 0 || wire == 1 || wire == 5); // the packable value can only be a primitive type

			printer_.Print(
				"if($wire !== 2 && $wire !== `wire`) {\n"
				"  throw new Exception(\"Incorrect wire format for field $field, expected: 2 or `wire` got: $wire\");\n"
				"}\n"
				"if ($wire === `wire`) {\n", "wire", SimpleItoa(wire)
			);
			printer_.Indent();
			printer_.Print(variables, commands.c_str());
			printer_.Outdent();
			printer_.Print(
				"} elseif ($wire === 2) {\n"
				"  $len = Protobuf::read_varint($fp, $limit);\n"
				//"  $limit -= $len;\n"
				"  while ($len > 0) {\n"
			);
			
			printer_.Indent(); printer_.Indent();
			variables["$limit"] = "$len"; // Switch this to len as we are in a loop, and no longer using limit
			printer_.Print(variables, commands.c_str());
			printer_.Outdent(); printer_.Outdent();

			printer_.Print(
				"  }\n"
				"}\n"
			);
		} else {
			printer_.Print(
				"if($wire !== `wire`) {\n"
				"  throw new Exception(\"Incorrect wire format for field $field, expected: `wire` got: $wire\");\n"
				"}\n", "wire", SimpleItoa(wire)
			);

			printer_.Print(variables, commands.c_str());

			if (field.containing_oneof()) {
				printer_.Print(variables,
					"$this->`oneof_case` = self::`oneof`;\n"
				);
			}
		}

		printer_.Print("\nbreak;\n");
		printer_.Outdent();
	}

	if (options_.skip_unknown()) {
		printer_.Print(
			"default:\n"
			"  $limit -= `ns`Protobuf::skip_field($fp, $wire);\n",
			"name", ClassName(message),
			"ns", ns_
		);
	} else {
		// TODO This fails to parse packed fields
		printer_.Print(
			"default:\n"
			"  $field_idx = $field . '-' . Protobuf::get_wiretype($wire);\n"
			"  $this->_unknown[$field_idx][] = `ns`Protobuf::read_field($fp, $wire, $limit);\n",
			"name", ClassName(message),
			"ns", ns_
		);
	}

	printer_.Outdent();
	printer_.Outdent();
	printer_.Print(
		"  }\n" // switch
		"}\n"   // while
	);

	if (SupportsRequiredValue()) {
		printer_.Print(
			"if (!$this->validate()) throw new Exception('Required fields are missing');\n"
		);
	}

	printer_.Outdent();
	printer_.Print("}\n");
}


/**
 * Some notes
 * Tag    <varint fieldID wireType>
 * Field  <tag> <value>
 * Length <tag> <len> <data>
 * Group  <start tag> <field>+ <end tag>
 * Embedded Message <tag> <len> <field>+
 * Start <field>+ (You have to know what type of Message it is, and it is not length prefixed)
 *
 * The Message class should not print its own length (this should be printed by the parent Message)
 * The Group class should only print its field, the parent should print the start/end tag
 * Otherwise the Message/Group will print everything of the fields.
 */

/**
 * Prints the write() method for this Message
 * @param printer
 * @param message
 * @param parentField
 */
void PHPFileGenerator::PrintWrite(const Descriptor & message) {

	// Write
	printer_.Print(
		"\n"
		"public function write($fp) {\n"
	);
	printer_.Indent();

	if (SupportsRequiredValue()) {
		printer_.Print(
			"if (!$this->validate())\n"
			"  throw new Exception('Required fields are missing');\n" // TODO Add list of missing fields
		);
	}

	map<string, string> variables;

	for (int i = 0; i < message.field_count(); ++i) {
		const FieldDescriptor &field ( Deref(message.field(i)) );

		FieldVariables(field, variables);

		// TODO We don't support writing packed fields

		string commands;
		switch (field.type()) {
			case FieldDescriptor::TYPE_DOUBLE: // double, exactly eight bytes on the wire
				commands = "`ns`Protobuf::write_double($fp, `var`);\n";
				break;

			case FieldDescriptor::TYPE_FLOAT: // float, exactly four bytes on the wire.
				commands = "`ns`Protobuf::write_float($fp, `var`);\n";
				break;

			case FieldDescriptor::TYPE_INT64:  // int64, varint on the wire.
			case FieldDescriptor::TYPE_UINT64: // uint64, varint on the wire.
			case FieldDescriptor::TYPE_INT32:  // int32, varint on the wire.
			case FieldDescriptor::TYPE_UINT32: // uint32, varint on the wire
			case FieldDescriptor::TYPE_ENUM:   // Enum, varint on the wire
				commands = "`ns`Protobuf::write_varint($fp, `var`);\n";
				break;

			case FieldDescriptor::TYPE_FIXED64: // uint64, exactly eight bytes on the wire.
				commands = "`ns`Protobuf::write_uint64($fp, `var`);\n";
				break;

			case FieldDescriptor::TYPE_SFIXED64: // int64, exactly eight bytes on the wire
				commands = "`ns`Protobuf::write_int64($fp, `var`);\n";
				break;

			case FieldDescriptor::TYPE_FIXED32: // uint32, exactly four bytes on the wire.
				commands = "`ns`Protobuf::write_uint32($fp, `var`);\n";
				break;

			case FieldDescriptor::TYPE_SFIXED32: // int32, exactly four bytes on the wire
				commands = "`ns`Protobuf::write_int32($fp, `var`);\n";
				break;

			case FieldDescriptor::TYPE_BOOL: // bool, varint on the wire.
				commands = "`ns`Protobuf::write_varint($fp, `var` ? 1 : 0);\n"; // TODO Change to be raw encoded values for 1 and 0
				break;

			case FieldDescriptor::TYPE_STRING:  // UTF-8 text.
			case FieldDescriptor::TYPE_BYTES:   // Arbitrary byte array.
				commands = "`ns`Protobuf::write_varint($fp, strlen(`var`));\n"
						   "fwrite($fp, `var`);\n";
				break;

			case FieldDescriptor::TYPE_GROUP: { // Tag-delimited message.  Deprecated.
				// The start tag has already been printed, but also print the end tag
				string endTag = MakeTag(field.number(), WireFormatLite::WIRETYPE_END_GROUP);
				commands = "`var`->write($fp); // group\n"
				           "fwrite($fp, \"" + PHPEscape(endTag) + "\", " + SimpleItoa(endTag.size()) + ");\n";
				break;
			}
			case FieldDescriptor::TYPE_MESSAGE: // Length-delimited message.
				commands = "`ns`Protobuf::write_varint($fp, `var`->size());\n"
				           "`var`->write($fp);\n";
				break;

			case FieldDescriptor::TYPE_SINT32:   // int32, ZigZag-encoded varint on the wire
				commands = "`ns`Protobuf::write_zint32($fp, `var`);\n";
				break;

			case FieldDescriptor::TYPE_SINT64:   // int64, ZigZag-encoded varint on the wire
				commands = "`ns`Protobuf::write_zint64($fp, `var`);\n";
				break;

			default:
				// TODO use the proper exception
				throw "Error " + field.full_name() + ": Unsupported type";
		}

		variables["ns"] = ns_; // TODO Is this already in the map?

		if (field.is_repeated()) {
			// TODO If we store default values in the repeated field, we will seralise them
			variables["var"] = "$v";
			printer_.Print(variables, "foreach($this->`name` as $v) {\n");

		} else {
			//variables["var"] = VariableName(field.name());
			variables["var"] = "$this->" + VariableName(field.name());

			if (field.containing_oneof() != nullptr) {
				const OneofDescriptor & oneof (Deref(field.containing_oneof()));

				variables["var"] = "$this->" + VariableName(oneof.name());
				printer_.Print(variables, "if ($this->`oneof_case` === self::`oneof`) {\n");

			} else if (FieldHasHas(field)) {
				// TODO is_null needed. I think we can ensure the default value is null, and remove this branch
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

void PHPFileGenerator::PrintSize(const Descriptor & message) {

	// Print the calc size method
	printer_.Print(
		"\n"
		"public function size() {\n"
		"  $size = 0;\n"
	);
	printer_.Indent();

	map<string, string> variables;

	for (int i = 0; i < message.field_count(); ++i) {
		const FieldDescriptor &field ( Deref(message.field(i)) );

		FieldVariables(field, variables);
		string command;

		int len = WireFormat::TagSize(field.number(), field.type());

		switch (WireFormat::WireTypeForField(&field)) {

			case WireFormatLite::WIRETYPE_VARINT:
				if (field.type() == FieldDescriptor::TYPE_BOOL) {
					len++; // A bool will always take 1 byte
					command = "$size += `len`;\n";
				} else {
					command = "$size += `len` + `ns`Protobuf::size_varint(`var`);\n";
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

				command += "$size += `len` + `ns`Protobuf::size_varint($l) + $l;\n";
				break;

			case WireFormatLite::WIRETYPE_START_GROUP:
			case WireFormatLite::WIRETYPE_END_GROUP:
				// WireFormat::TagSize returns the tag size * two when using groups, to account for both the start and end tag
				command += "$size += `len` + `var`->size();\n";
				break;

			default:
				// TODO use the proper exception
				throw "Error " + field.full_name() + ": Unsupported wire type";
		}

		variables["len"] = SimpleItoa(len);
		variables["ns"]  = ns_;

		if (field.is_repeated()) {
			// TODO Support packed size
			variables["var"] = "$v";
			printer_.Print(variables,  "foreach($this->`name` as $v) {\n");


		} else {
			variables["var"] = "$this->" + VariableName(field.name());

			if (field.containing_oneof() != nullptr) {
				const OneofDescriptor & oneof (Deref(field.containing_oneof()));

				variables["var"] = "$this->" + VariableName(oneof.name());
				printer_.Print(variables, "if ($this->`oneof_case` === self::`oneof`) {\n");

			} else if (FieldHasHas(field)) {
				// TODO is_null needed. I think we can ensure the default value is null, and remove this branch
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
		"}\n"
	);
}

void PHPFileGenerator::PrintValidate(const Descriptor & message) {

	if (!SupportsRequiredValue()) {
		return;
	}

	// Validate that the required fields are included
	printer_.Print(
		"\n"
		"public function validate() {\n"
	);
	printer_.Indent();

	for (int i = 0; i < message.field_count(); ++i) {
		const FieldDescriptor &field ( Deref(message.field(i)) );

		if (field.is_required()) {
			printer_.Print("if ($this->`name` === null) return false;\n",
				"name", VariableName(field.name())
			);
		}
	}

	printer_.Print("return true;\n");
	printer_.Outdent();
	printer_.Print("}\n");
}

void PHPFileGenerator::PrintToString(const Descriptor & message) {

	printer_.Print(
		"\n"
		"public function __toString() {\n"
		"  return ''"
	);
	printer_.Indent();

	if (!options_.skip_unknown()) {
		printer_.Print("\n     . `ns`Protobuf::toString('unknown', $this->_unknown, array())",
			"ns", ns_);
	}

	map<string, string> variables;

	for (int i = 0; i < message.oneof_decl_count(); ++i) {
		const OneofDescriptor & oneof ( Deref(message.oneof_decl(i)) );

		variables.clear();
		FieldVariables(oneof, variables);
		variables["ns"]   = ns_;

		printer_.Print(variables,
			"\n     . `ns`Protobuf::toString('`oneof_field`_case', $this->`oneof_case`, `oneof_default`)"
			"\n     . `ns`Protobuf::toString('`oneof_field`', $this->`oneof_name`, null)"
		);
	}

	for (int i = 0; i < message.field_count(); ++i) {
		const FieldDescriptor &field ( Deref(message.field(i)) );

		if (field.containing_oneof()) {
			continue;
		}

		FieldVariables(field, variables);
		variables["ns"]   = ns_;

		printer_.Print(variables,
			"\n     . `ns`Protobuf::toString('`field`', $this->`name`, `default`)"
		);
/*
		if (field.type() == FieldDescriptor::TYPE_ENUM) {
			variables["enum"] = ClassName(Deref(field.enum_type()));
			printer_.Print(variables,
				"\n     . `ns`Protobuf::toString('`field`', `enum`::toString($this->`name`))"
			);
		} else {
			printer_.Print(variables,
				"\n     . `ns`Protobuf::toString('`field`', $this->`name`)"
			);
		}
*/

	}
	printer_.Print(";\n");
	printer_.Outdent();
	printer_.Print("}\n");
}

void PHPFileGenerator::PrintOneOfConstants(const Descriptor & message) {
	// Print all the OneOf Enums constants at the top of the message
	if (message.oneof_decl_count() > 0) {
		printer_.Print("const NONE = 0;\n"); // TODO There is a bug here if a field is also named none

		for (int i = 0; i < message.oneof_decl_count(); ++i) {
			const OneofDescriptor & oneof( Deref(message.oneof_decl(i)) );

			for (int j = 0; j < oneof.field_count(); ++j) {
				const FieldDescriptor & field ( Deref(oneof.field(j)) );

				printer_.Print(
					"const `name` = `number`;\n",
					"name",   OneOfConstant(field.name()),
					"number", SimpleItoa(field.number())
				);
			}
		}
	}
}


void PHPFileGenerator::PrintFields(const Descriptor & message) {

	if (!options_.skip_unknown())
		printer_.Print("\nprivate $_unknown;\n");

	map<string, string> variables;

	for (int i = 0; i < message.oneof_decl_count(); ++i) {
		const OneofDescriptor & oneof ( Deref(message.oneof_decl(i)) );

		FieldVariables(oneof, variables);

		printer_.Print(variables,
			"private $`oneof_name` = null; // `oneof_definition`\n"
			"private $`oneof_case` = `oneof_default`;\n"
		);
	}

	for (int i = 0; i < message.field_count(); ++i) {
		const FieldDescriptor & field ( Deref(message.field(i)) );

		// Skip over fields in oneof
		if (field.containing_oneof() != nullptr) {
			assert(!field.is_repeated()); // Proto spec doesn't allow repeated oneof fields. We assume this elsewhere, so lets check.
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

void PHPFileGenerator::PrintSetterGetterMethods(const Descriptor & message) {

	map<string, string> variables;

	for (int i = 0; i < message.oneof_decl_count(); ++i) {
		const OneofDescriptor & oneof( Deref(message.oneof_decl(i)) );

		FieldVariables(oneof, variables);
		printer_.Print("\n");

		printer_.Print(variables,
			"public function clear`oneof_capitalized_name`() { $this->`oneof_case` = `oneof_default`; $this->`oneof_name` = null; }\n"
			"public function has`oneof_capitalized_name`() { return $this->`oneof_case` !== `oneof_default`; }\n"
			"public function get`oneof_capitalized_name`() { if($this->`oneof_case` !== `oneof_default`) return $this->`oneof_name`; else return null; }\n"
			"public function get`oneof_capitalized_name`Case() { return $this->`oneof_case`; }\n"
		);
	}

	for (int i = 0; i < message.field_count(); ++i) {
		const FieldDescriptor & field ( Deref(message.field(i)) );
		
		FieldVariables(field, variables);
		printer_.Print("\n");

		// TODO Change the set code to validate input depending on the variable type
		// TODO Add comments to the methods
		// TODO Add @Deprecated tag
		if (field.containing_oneof() != nullptr) {
			// A oneof field
			printer_.Print(variables,
				"public function clear`capitalized_name`() { if ($this->`oneof_case` === self::`oneof`) clear`oneof_capitalized_name`(); }\n"
				"public function has`capitalized_name`() { return $this->`oneof_case` === self::`oneof`; }\n"
				"public function get`capitalized_name`() { if($this->`oneof_case` === self::`oneof`) return $this->`oneof_name`; else return `default`;}\n"
				"public function set`capitalized_name`(`type`$value) { $this->`oneof_case` = self::`oneof`; $this->`oneof_name` = $value; }\n"
			);

		} else if (field.is_repeated()) {
			// Repeated field
			printer_.Print(variables,
				"public function clear`capitalized_name`() { $this->`name` = array(); }\n"

				"public function get`capitalized_name`Count() { return count($this->`name`); }\n"
				"public function get`capitalized_name`(int $index) { return $this->`name`[$index]; }\n"
				"public function get`capitalized_name`Array() { return $this->`name`; }\n"
	
				// TODO Change the set code to validate input depending on the variable type
				"public function set`capitalized_name`(int $index, `type`$value) {$this->`name`[$index] = $value; }\n"
				"public function add`capitalized_name`(`type`$value) { $this->`name`[] = $value; }\n"
				"public function addAll`capitalized_name`(array $values) { foreach($values as $value) {$this->`name`[] = $value; } }\n"
			);

		} else if (IsProto2()) {
			printer_.Print(variables,
				"public function clear`capitalized_name`() { $this->`name` = null; }\n"
				"public function has`capitalized_name`() { return $this->`name` !== null; }\n"
				"public function get`capitalized_name`() { if($this->`name` !== null) return $this->`name`; else return `default`;}\n"
				"public function set`capitalized_name`(`type`$value) { $this->`name` = $value; }\n"
			);

		} else {
			printer_.Print(variables,
				"public function clear`capitalized_name`() { $this->`name` = `default`; }\n"
				"public function get`capitalized_name`() { return $this->`name`;}\n"
				"public function set`capitalized_name`(`type`$value) { $this->`name` = $value; }\n"
			);
		}
	}
}

void PHPFileGenerator::PrintMessage(const Descriptor & message) {

	// Print nested messages
	for (int i = 0; i < message.nested_type_count(); ++i) {
		printer_.Print("\n");
		PrintMessage( Deref(message.nested_type(i)) );
	}

	// Print nested enum
	for (int i = 0; i < message.enum_type_count(); ++i) {
		PrintEnum( Deref(message.enum_type(i)) );
	}

	// Find out if we are a nested type, if so what kind
	const FieldDescriptor * parentField = NULL;
	const char * type = "message";
	if (message.containing_type() != NULL) {
		const Descriptor & parent ( Deref(message.containing_type()) );

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
		"class `name` extends ProtobufMessage {\n",
			"name", ClassName(message),
			"type", type,
			"full_name", message.full_name()
	);
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
		"\n" // TODO add comments
		"public function __construct($in = NULL, &$limit = PHP_INT_MAX) {\n"
       	"	parent::__construct($in, $limit);\n"
		"}\n"
	);

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
		"full_name", message.full_name()
	);

	printer_.Outdent();
	printer_.Print("}\n\n");
}

void PHPFileGenerator::PrintEnum(const EnumDescriptor & e) {

	printer_.Print(
		"// enum `full_name`\n"
		"class `name` extends ProtobufEnum {\n",
		"full_name", e.full_name(),
		"name", ClassName(e)
	);

	printer_.Indent();

	// Print fields
	for (int j = 0; j < e.value_count(); ++j) {
		const EnumValueDescriptor &value ( *e.value(j) );

		printer_.Print(
			"const `name` = `number`;\n",
			"name",   UpperString(value.name()),
			"number", SimpleItoa(value.number())
		);
	}

	// Print values array
	printer_.Print("\npublic static $values_ = array(\n");
	printer_.Indent();
	for (int j = 0; j < e.value_count(); ++j) {
		const EnumValueDescriptor &value ( *e.value(j) );

		printer_.Print(
			"`number` => self::`name`,\n",
			"number", SimpleItoa(value.number()),
			"name",   UpperString(value.name())
		);
	}
	printer_.Outdent();
	printer_.Print(");\n\n");

	printer_.Print( // TODO Move this into a base enum class - but check how static functions are inhertieted
		"public static function isValid($value) {\n"
		"  return array_key_exists($value, self::$values_);\n"
		"}\n\n"
	);

	// Print a toString. // TODO Change to __toString()
	printer_.Print(
		"public static function toString($value) {\n"
		"  checkArgument(is_int($value), 'value must be a integer');\n"
		//"  if (is_null($value)) return null;\n"
		"  if (array_key_exists($value, self::$values_))\n"
		"    return self::$values_[$value];\n"
		"  return 'UNKNOWN';\n"
		"}\n"
	);

	printer_.Outdent();
	printer_.Print("}\n\n");
}

void PHPFileGenerator::PrintService(const ServiceDescriptor & s) {
	(void)s; // Unused
	printer_.Print("////\n//TODO Service\n////\n");
}

void PHPFileGenerator::PrintMessages() {
	for (int i = 0; i < file_.message_type_count(); ++i) {
		PrintMessage(Deref(file_.message_type(i)));
	}
}

void PHPFileGenerator::PrintEnums() {
	for (int i = 0; i < file_.enum_type_count(); ++i) {
		PrintEnum(Deref(file_.enum_type(i)));
	}
}

void PHPFileGenerator::PrintServices() {
	for (int i = 0; i < file_.service_count(); ++i) {
		PrintService(Deref(file_.service(i)));
	}
}

bool PHPFileGenerator::Generate(string* error) {

	// TODO Check if the last-mod time of the php file is newer than the proto. If so skip.
	string php_filename = FileDescriptorToPath(file_);

	try {
		printer_.Print(
			"<?php\n"
			"// Please include protocolbuffers before this file, for example:\n"
			"//   require('protocolbuffers.inc.php');\n"
			"//   require('`filename`');\n",
			"filename", php_filename.c_str()
		);

		// TODO Move the following into a method
		for (int i = 0; i < file_.dependency_count(); i++) {
			const FileDescriptor & dep_file ( Deref(file_.dependency(i)) );

			printer_.Print("require('`filename`');\n", "filename", FileDescriptorToPath(dep_file).c_str());
		}

		printer_.Print("\n");

		if (!namespace_.empty()) {
			printer_.Print("namespace `namespace` {\n", "namespace", namespace_.c_str());
			printer_.Indent();
		}

		PrintEnums();
		PrintMessages();
		PrintServices();

		if (!namespace_.empty()) {
			printer_.Outdent();
			printer_.Print("}");
		}

	} catch (const std::string &msg) {
		*error = msg;
		return false;

	} catch (const char *msg) {
		error->assign( msg );
		return false;
	}

	return true;
}

PHPCodeGenerator::PHPCodeGenerator() {}
PHPCodeGenerator::~PHPCodeGenerator() {}

bool PHPCodeGenerator::Generate(const FileDescriptor* file,
				const string& parameter,
				GeneratorContext* context,
				string* error) const {

	assert(file != nullptr);
	assert(context != nullptr);

	string php_filename = FileDescriptorToPath(*file);
	cerr << "Generating " << php_filename << endl;

	// Generate main file.
	// TODO Check for error opening
	scoped_ptr<io::ZeroCopyOutputStream> output(
		context->Open(php_filename)
	);

	// TODO Move this into the constructor
	io::Printer printer(output.get(), '`');

	PHPFileGenerator main(printer, *file, parameter);
	main.Generate(error);

	for (int i = 0; i < file->dependency_count(); i++) {
		// TODO Check if we have processed this file already (due to imports), and if so skip
		// TODO Keep track of this in the PHPCodeGenerator
		if (!Generate(file->dependency(i), parameter, context, error)) {
			return false;
		}
	}

	return true;
}


int main(int argc, char* argv[]) {
	PHPCodeGenerator generator;
	return PluginMain(argc, argv, &generator);
}
