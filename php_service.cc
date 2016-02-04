/**
 * PHP Protocol Buffer Generator Plugin for protoc
 * By Andrew Brampton (c) 2010,2016
 */

#include "php_generator.h"

void PHPFileGenerator::PrintService(const ServiceDescriptor& s) {
  (void)s;  // Unused
  printer_.Print("////\n//TODO Service\n////\n");
}


void PHPFileGenerator::PrintServices() {
  for (int i = 0; i < file_.service_count(); ++i) {
    PrintService(Deref(file_.service(i)));
  }
}
