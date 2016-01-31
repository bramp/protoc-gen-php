#include <map>
#include <string>

#include <google/protobuf/descriptor.h>
#include <google/protobuf/stubs/common.h>
#include <google/protobuf/wire_format_lite.h>

using std::map;
using std::string;

using google::protobuf::uint8;
using google::protobuf::FileDescriptor;
using google::protobuf::FieldDescriptor;
using google::protobuf::OneofDescriptor;
using google::protobuf::internal::WireFormatLite; // TODO Should I be using internal?

template <class T>
T & Deref(T * t) {
	assert(t != nullptr);
	return *t;
}

//
// Helper methods for turning Protobufs into PHP types
//

string DefaultValueAsString(const FieldDescriptor & field, bool quote_string_type);

// Maps a message full_name into a PHP name
template <class DescriptorType>
string ClassName(const DescriptorType & descriptor) {
	string name (descriptor.full_name());
	replace(name.begin(), name.end(), '.', '_');
	return name;
}

// Returns the member name
// TODO Make this smarter to avoid illegal names
string VariableName(const string & s);

// Returns the constant used for this field
string OneOfConstant(const string & s);

string FileDescriptorToPath(const FileDescriptor & file);

string UnderscoresToCamelCase(const string & s);
string UnderscoresToCapitalizedCamelCase(const string & s);

string ArrayToPHPString(const uint8 *a, size_t len);
string PHPEscape(const string & a);

string MakeTag(const FieldDescriptor & field);
string MakeTag(int field_number, WireFormatLite::WireType type);

// Does this field have a hasX method
bool FieldHasHas(const FieldDescriptor & field);

void FieldVariables(const OneofDescriptor & oneof, map<string, string> & variables);
void FieldVariables(const FieldDescriptor & field, map<string, string> & variables);
