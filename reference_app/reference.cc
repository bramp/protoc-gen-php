/**
 * Simple test app for checking edge cases in the protobuf spec
 * `make && reference_app/reference && hd output`
 */
#include <cmath>
#include <cfloat> // DBL_MAX
#include <iostream>
#include <iomanip> // for std::setprecision()
#include <fstream>
#include <limits.h>
#include <string>
#include "conformance.pb.h"

using namespace std;

using conformance::TestAllTypes;

int main(int argc, char *argv[]) {
	(void)argc; (void)argv;

	GOOGLE_PROTOBUF_VERIFY_VERSION;

	double x = 0x2000000000000; 

    //   562949953421311.9375, 562949953421312 (2^49),         562949953421312.125
    //   72057594037927928, 72057594037927936 (2^56),         72057594037927952
	// 9223372036854774784, 9223372036854775808 (2^63), 9223372036854777856

	std::cout << std::setprecision(20);
	cout << nextafter(x, 0) << endl;
	cout << x << endl;
	cout << nextafter(x, DBL_MAX) << endl;

	TestAllTypes all;
	//all.set_optional_int32(-1);  // ff ff ff ff ff ff ff ff ff 01 
	//all.set_optional_int32(-2);  // fe ff ff ff ff ff ff ff ff 01
	//all.set_optional_int32(-8);  // f8 ff ff ff ff ff ff ff ff 01
	//all.set_optional_int32(-16); // f0 ff ff ff ff ff ff ff ff 01

	//all.set_optional_int32(1);  // 01

	//all.set_optional_int32(-2147483648); // 80 80 80 80 f8 ff ff  ff ff 01
	//all.set_optional_int32(2147483647);  // ff ff ff ff 07

	//all.set_optional_int64(-1); // ff ff ff ff ff ff ff  ff ff 01
	//all.set_optional_int64(1);  // 01

	//all.set_optional_int64(-2147483648); // 80 80 80 80 f8 ff ff  ff ff 01
	//all.set_optional_int64(2147483647);  // ff ff ff ff 07

	//all.set_optional_int64(−9223372036854775808);  // 80 80 80 80 80 80 80 80 80 01
	//all.set_optional_int64(+9223372036854775807);  // ff ff ff ff ff ff ff ff 7f

	//all.set_optional_uint64(18446744073709551615ULL); // ff ff ff ff ff ff ff ff ff 01
	all.set_optional_uint64(2147483647); // ff ff ff ff 07
	all.set_optional_uint64(2147483648); // 80 80 80 80 08
	all.set_optional_uint64(2147483649); // 81 80 80 80 08

	//all.set_optional_uint64(9223372036854775807ULL); // 2^63-1  ff ff ff ff ff ff ff ff 7f
	//all.set_optional_uint64(9223372036854775808ULL); // 2^63    80 80 80 80 80 80 80 80 80 01

	//all.set_optional_uint64(18446744073709551615ULL); // 2^64-1  ff ff ff ff ff ff ff ff ff 01
	//all.set_optional_uint64(72057594037927935); // ff ff ff ff ff ff ff  7f
	//all.set_optional_uint64(72057594037927936); // 80 80 80 80 80 80 80  80 01
	//all.set_optional_uint64(72057594037927937); // 81 80 80 80 80 80 80 80 01

	// Maps
	//(*all.mutable_map_int32_int32())[1] = 2; // c2 03 04 08 01 10 02 // tag 56 {1: 1, 2: 2}
	//(*all.mutable_map_int32_int32())[3] = 4; // c2 03 04 08 03 10 04 // tag 56 {1: 3, 2: 4}

	fstream output("output", ios::out | ios::trunc | ios::binary);
    if (!all.SerializeToOstream(&output)) {
      cerr << "Failed to write proto." << endl;
      return -1;
    }

	google::protobuf::ShutdownProtobufLibrary();
}