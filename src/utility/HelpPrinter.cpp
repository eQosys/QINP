#include "HelpPrinter.h"

#include <cstdio>

#define HELP_TEXT \
	"Usage: %s [options] [input_files]\n" \
	"Options:\n" \
	"  -h, --help\n" \
	"    Prints this help message.\n" \
	"  -v, --verbose\n" \
	"    Prints verbose output.\n" \
	"    Generates informational comments in the assembly code.\n" \
	"  -i, --import-dir [path]\n" \
	"    Specifies an import directory.\n" \
	"  -o, --output-file [path]\n" \
	"    Specifies the output path of the generated executable.\n" \
	"  -k, --keep-intermediate\n" \
	"    Keeps the generated assembly file.\n" \
	"  -r, --run\n" \
	"    Runs the generated program.\n" \
	"  -A, --architecture [architecture]\n" \
	"    Specifies the target architecture. (x86, qipu)\n" \
	"    Only x86 is supported for now.\n" \
	"  -P, --platform [platform]\n" \
	"    Specifies the target platform. (linux, windows, macos)\n" \
	"    Only linux and windows are supported for now.\n" \
	"  -a, --runarg [arg]\n" \
	"    Specifies a single argument to pass to the generated program.\n" \
	"    Only used when --run is specified.\n" \
	"  -x, --extern-library [filepath]\n" \
	"    Specifies a library/object file to link against.\n" \
	"  -e, --export-symbol-info\n" \
	"    Writes the symbols (including unused ones) of the parsed program code\n" \
	"    and additional info to the specified file.\n" \
	"  -c, --export-comments\n" \
	"    Writes the comments of the parsed program code to the specified file.\n"

void print_help(const char* prog_name)
{
    printf(HELP_TEXT, prog_name);
}