/**
 * Simple test app for checking edge cases in the protobuf spec
 */
#include <iostream>
#include <fstream>
#include <string>
#include "conformance.pb.h"

using namespace std;

using conformance::TestAllTypes;

int main(int argc, char *argv[]) {
	(void)argc; (void)argv;

	GOOGLE_PROTOBUF_VERIFY_VERSION;

	TestAllTypes all;
	//all.set_optional_int32(-1); // ff ff ff ff ff ff ff ff ff 01
	//all.set_optional_int32(1);  // 01

	//all.set_optional_int32(-2147483648); // 80 80 80 80 f8 ff ff  ff ff 01
	//all.set_optional_int32(2147483647);  // ff ff ff ff 07

	all.set_optional_int64(-1); // ff ff ff ff ff ff ff  ff ff 01
	all.set_optional_int64(1);  // 01

	all.set_optional_int64(-2147483648); // 80 80 80 80 f8 ff ff  ff ff 01
	all.set_optional_int64(2147483647);  // ff ff ff ff 07
	

	fstream output("output", ios::out | ios::trunc | ios::binary);
    if (!all.SerializeToOstream(&output)) {
      cerr << "Failed to write proto." << endl;
      return -1;
    }

	google::protobuf::ShutdownProtobufLibrary();
}