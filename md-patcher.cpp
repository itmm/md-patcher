#line 36 "README.md"
#include <cstdlib>
#line 90
#include <string>
#line 37

#line 75
static inline void run_tests() {
	// unit-tests
}

#line 38
int main(int argc, const char *argv[]) {
#line 80
	run_tests();
#line 94
	if (argc == 2 && argv[1] == std::string { "--run-only-tests" }) {
		return EXIT_SUCCESS;
	}
#line 39
	// parse input
	// write output
	return EXIT_SUCCESS;
}
