/**
 * @file main.cpp
 * @author EA
 */

#include <iostream>
#include <string>
#include <unordered_map>
#include <filesystem>
#include <vector>
#include <getopt.h>

#include <src/maxcoverage.h>

using namespace std::filesystem;

// used to convert the default values of the compression to a string
#define STRINGIFY( x) #x
#define STR(x) STRINGIFY(x)

static void print_usage(bool error) {
	static const char* usage_str = \
	"Usage: ..-cli\n"
    "-h                                        show this help\n"
	;
	FILE* os = error ? stderr : stdout;
	fprintf(os, "%s", usage_str);
}


#define check_mode(mode_compress, mode_read, compress_expected, option_name) \
do { \
    if(compress_expected) { \
        if(mode_read) { \
            fprintf(stderr, "option '-%s' not allowed when compressing\n", option_name); \
            return -1; \
        } \
        mode_compress = true; \
    } \
    else { \
        if(mode_compress) { \
            fprintf(stderr, "option '-%s' not allowed when reading the compressed graph\n", option_name); \
            return -1; \
        } \
        mode_read = true; \
    } \
} while(0)

int main(int argc, char** argv) {
	if(argc <= 1) {
		print_usage(true);
		return EXIT_FAILURE;
	}

    int opt;
    std::string input_file;
    std::string output_file;
    std::string test_string;
    bool mode_compress = false;
    bool mode_read = false;
    while ((opt = getopt(argc, argv, "hi:o:t:")) != -1) {
        switch (opt) {
            case 'i':
                check_mode(mode_compress, mode_read, true, "i");
                input_file = optarg;
                if (!exists(input_file)) {
                    printf("Invalid input file.");
                    return EXIT_FAILURE;
                }
                break;
            case 'o':
                check_mode(mode_compress, mode_read, true, "o");
                output_file = optarg;
                if (output_file.empty()) {
                    printf("Output file not specified correctly.");
                    return -1;
                }
                break;
            case 't':
                check_mode(mode_compress, mode_read, false, "t");
                test_string = optarg;
                break;
            case 'h':
            default:
                print_usage(true);
                return 0;
        }
    }

    if (mode_compress) {

    }
    if (mode_read) {
        compute_max_coverage(test_string);
    }
    return EXIT_SUCCESS;
}
