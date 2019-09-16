/*-
 * Copyright 2014 Diomidis Spinellis
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 */

#include <cstring>
#include <string>
#include <fstream>
#include <iostream>
#include <ostream>
#include <vector>

#include "errno.h"
#include "unistd.h"

#include "CTokenizer.h"
#include "CppTokenizer.h"
#include "CSharpTokenizer.h"
#include "JavaTokenizer.h"
#include "PHPTokenizer.h"
#include "PythonTokenizer.h"

// Process and print the metrics of stdin
static void
process_file(const std::string lang, const std::vector<std::string> opt,
		std::string filename, char processing_type)
{
	std::ifstream in;
	CharSource cs;
	TokenizerBase *t;

	if (!filename.empty()) {
		in.open(filename.c_str(), std::ifstream::in);
		if (!in.good()) {
			std::cerr << "Unable to open " << filename <<
				": " << strerror(errno) << std::endl;
			exit(EXIT_FAILURE);
		}
		std::cin.rdbuf(in.rdbuf());
	}

	if (lang == "" || lang == "Java")
		t = new JavaTokenizer(cs, filename, opt);
	else if (lang == "C")
		t = new CTokenizer(cs, filename, opt);
	else if (lang == "CSharp" || lang == "C#")
		t = new CSharpTokenizer(cs, filename, opt);
	else if (lang == "C++")
		t = new CppTokenizer(cs, filename, opt);
	else if (lang == "PHP")
		t = new PHPTokenizer(cs, filename, opt);
	else if (lang == "Python")
		t = new PythonTokenizer(cs, filename, opt);
	else {
		std::cerr << "Unknown language specified." << std::endl;
		std::cerr << "The following languages are supported:" << std::endl;
		std::cerr << "\tC" << std::endl;
		std::cerr << "\tCSharp (or C#)" << std::endl;
		std::cerr << "\tC++" << std::endl;
		std::cerr << "\tJava" << std::endl;
		std::cerr << "\tPHP" << std::endl;
		std::cerr << "\tPython" << std::endl;
		exit(1);
	}
	switch (processing_type) {
	case 'c':
		t->code_tokenize();
		break;
	case 'n':
		t->numeric_tokenize();
		break;
	case 's':
		t->symbolic_tokenize();
		break;
	case 't':
		t->type_tokenize();
		break;
	case 'T':
		t->type_code_tokenize();
		break;
	default:
		std::cerr << "Unknown processing type specified." << std::endl;
		std::cerr << "The following processing types are supported:" << std::endl;
		std::cerr << "\tc: output code; one token per line" << std::endl;
		std::cerr << "\tn: output numeric values" << std::endl;
		std::cerr << "\ts: output token symbols" << std::endl;
		std::cerr << "\tt: output token types" << std::endl;
		std::cerr << "\tT: output token types and code; one token per line" << std::endl;
		exit(1);
	}
}

bool endswith(std::string const &str, std::string const &ending) {
    if (ending.size() > str.size()) return false;
    return std::equal(ending.rbegin(), ending.rend(), str.rbegin());
}

std::string get_lang_from_ext(std::string filename) {
	if (endswith(filename, ".cpp") || endswith(filename, ".h") || endswith(filename, ".hpp")) return "C++";
	if (endswith(filename, ".cs")) return "C#";
	if (endswith(filename, ".c")) return "C";
	if (endswith(filename, ".java")) return "Java";
	if (endswith(filename, ".py")) return "Python";
	return "";
}

/* Calculate and print C metrics for the standard input */
int
main(int argc, char * const argv[])
{
	int opt;
	std::string lang = "";
	std::vector<std::string> processing_opt;
	char processing_type = 'n';
	bool get_files_from_stdin = false;

	while ((opt = getopt(argc, argv, "l:o:t:L:")) != -1)
		switch (opt) {
		case 'l':
			lang = optarg;
			break;
		case 'o':
			processing_opt.push_back(optarg);
			break;
		case 't':
			processing_type = *optarg;
			break;
		case 'L':
			get_files_from_stdin = true;
			break;
		default: /* ? */
			std::cerr << "Usage: " << argv[0] <<
				" [-l lang] [-o opt] [-t type] [-L] [file ...]" << std::endl;
			exit(EXIT_FAILURE);
		}


	if (get_files_from_stdin) {
		std::vector<std::string> filenames;
		std::string cur;
		while (std::getline(std::cin, cur)) {
			if (!cur.empty()) {
				filenames.push_back(cur);
			}
		}

		for (std::string &filename : filenames) {
			process_file(get_lang_from_ext(filename), processing_opt, filename, processing_type);
		}
		exit(EXIT_SUCCESS);
	}

	if (!argv[optind]) {
		process_file(lang, processing_opt, "", processing_type);
		exit(EXIT_SUCCESS);
	}

	// Read from file, if specified
	while (argv[optind]) {
		process_file(lang, processing_opt, argv[optind], processing_type);
		optind++;
	}

	exit(EXIT_SUCCESS);
}
