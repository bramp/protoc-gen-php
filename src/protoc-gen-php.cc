/**
 * PHP Protocol Buffer Generator Plugin for protoc
 * By Andrew Brampton (c) 2010,2016
 *
 * TODO
 *  Extensions
 *  Services
 *  Better validation (add code to check setted values are valid)
 *  option optimize_for = CODE_SIZE/SPEED;
 *  Place field names in the generated exceptions
 *  Add comments to the generated code.
 *  Add @Deprecated support
 *  Add extra methods to the built in types, such as a Timestamp Helper
 *  Support proto3 json mapping
 *  Add Reserved support
 *  Add Map Support
 *  Add write_bytes(...)
 *  Cleanup write/size code
 *  Add strict checking of UTF-8 in proto3
 *  Add to PEAR and/or Composer
 *  ToString needs a lot of work
 *  Implement a CodedInputStream and CodedOutputStream
 *  Writing packed fields
 *  Support the autoloading standard
 *  Format the PHP
 *  Have a mode where accurate integers work (using GMP or something)
 *  Add a clear() method
 *  PSR-1/2
 *    Four space indent
 *  Write out unknown fields
 *  Add code to check all the included protobuf code was generated for the same version of PHP
 *  Add a codec field, that says how exactly to encode/decode
 */

#include "php_generator.h"

#include <map>
#include <string>
#include <iostream>

#include <google/protobuf/descriptor.h>

#include <google/protobuf/compiler/plugin.h>
#include <google/protobuf/compiler/code_generator.h>

#include <google/protobuf/io/printer.h>
#include <google/protobuf/io/zero_copy_stream.h>

using std::string;

using namespace google::protobuf;
using namespace google::protobuf::compiler;
using namespace google::protobuf::internal;

class PHPCodeGenerator : public CodeGenerator {
 public:
  PHPCodeGenerator();
  virtual ~PHPCodeGenerator();

  bool Generate(const FileDescriptor* file, const string& parameter,
                GeneratorContext* context, string* error) const;
};

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
  scoped_ptr<io::ZeroCopyOutputStream> output(context->Open(php_filename));

  // TODO Move this into the constructor
  io::Printer printer(output.get(), '`');

  PHPFileGenerator main(printer, *file, parameter);
  main.Generate(error);

  for (int i = 0; i < file->dependency_count(); i++) {
    // TODO Check if we have processed this file already (due to imports), and
    // if so skip
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
