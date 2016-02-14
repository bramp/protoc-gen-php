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

using google::protobuf::internal::WireFormatLite;  // TODO Should I be using
                                                   // internal?

template <class T>
T &Deref(T *t) {
  assert(t != NULL);
  return *t;
}


/**
* Returns the path to the generated file for this FileDescriptor
*/
string FileDescriptorToPath(const FileDescriptor &file);

string OneLineDefinition(const string &definition);
//
// Helper methods for turning Protobufs into PHP types
//

string DefaultValueAsString(const FieldDescriptor &field);

string UnderscoresToCamelCase(const string &s);
string UnderscoresToCapitalizedCamelCase(const string &s);

string ArrayToPHPString(const uint8 *a, size_t len);
string PHPEscape(const string &a);

string UnderscoresToCamelCaseImpl(const string& input, bool cap_next_letter);

string LowerString(const string& s);
string UpperString(const string& s);
string TrimString(const string& s);

string MakeTag(const FieldDescriptor &field);
string MakeTag(int field_number, WireFormatLite::WireType type);
