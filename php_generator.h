/**
 * PHP Protocol Buffer Generator Plugin for protoc
 * By Andrew Brampton (c) 2010,2016
 */

// TODO strutil.h is from the offical protobuf source, but it is not installed.
// Replace it
//      We use LowerString, UpperString, SimpleItoa from it.
#include "php_util.h"
#include "strutil.h"

#include <map>
#include <string>

#include <google/protobuf/descriptor.h>
#include <google/protobuf/io/printer.h>

#include "php_options.pb.h"

using std::map;
using std::string;

using namespace google::protobuf;
using namespace google::protobuf::compiler;
using namespace google::protobuf::internal;

/**
 * Generates a single PHP file from a proto file
 */
class PHPFileGenerator {
 public:
  PHPFileGenerator(io::Printer& printer, const FileDescriptor& file,
                   const string& parameter);
  virtual ~PHPFileGenerator();

  bool Generate(string* error);

 private:
  void PrintEnums();
  void PrintMessages();
  void PrintServices();

  void PrintMessage(const Descriptor& message);
  void PrintEnum(const EnumDescriptor& enu);
  void PrintService(const ServiceDescriptor& service);

  void PrintOneOfConstants(const Descriptor& message);
  void PrintFields(const Descriptor& message);
  void PrintSetterGetterMethods(const Descriptor& message);

  // Print the read() method
  void PrintRead(const Descriptor& message, const FieldDescriptor* parentField);

  // Print the write() method
  void PrintWrite(const Descriptor& message);

  // Print the size() method
  void PrintSize(const Descriptor& message);

  // Print the validate() method
  void PrintValidate(const Descriptor& message);

  // Print the __toString() method
  void PrintToString(const Descriptor& message);

  bool IsProto2() const {
    return file_.syntax() == FileDescriptor::SYNTAX_PROTO2;
  }

  bool IsProto3() const {
    return file_.syntax() == FileDescriptor::SYNTAX_PROTO3;
  }

  int TargetPHP() const {
    return 53; // TODO Make configurable
  }

  // Does this protobuf support the required field
  bool SupportsRequiredValue() const { return IsProto2(); }

  // Should we be using a namespace
  bool UseNamespaces() const { return TargetPHP() >= 53; }

  //
  // Helper methods to generate names
  //
  bool FieldHasHas(const FieldDescriptor &field) {
    return IsProto2() || field.containing_oneof();
  }

  // TODO Make these smarter to avoid illegal names
  string OneOfConstant(const string &s) { return UpperString(s); }
  string VariableName(const string &s) { return UnderscoresToCamelCase(s); }

  string NamespaceName(const FileDescriptor &f) {
    // TODO Ensure namespace is PHP friendly.
    string name(f.package());
    replace(name.begin(), name.end(), '.', '\\');
    return name;
  }

  void FieldVariables(const FieldDescriptor &field, map<string, string> &variables);
  void FieldVariables(const OneofDescriptor &oneof, map<string, string> &variables);

  //
  // Members
  //
  io::Printer& printer_;
  const FileDescriptor& file_;
  const string& parameter_;

  const PHPFileOptions& options_;
};