#line 42 "README.md"
#include <cstdlib>
#line 247
#include <map>
#line 355
#include <ostream>
#line 283
#include <sstream>
#line 92
#include <string>
#line 226
#include <vector>
#line 43

#line 531
#include "lazy-write/lazy-write.h"
#include "line-reader/line-reader.h"
#include "solid/require.h"

#line 1051
bool write_raw { false };

#line 960
static std::string link_in_line(const std::string &line) {
	std::string got;
	auto ci = line.find("](");
	if (ci != std::string::npos) {
		auto si = line.rfind('[', ci);
		auto ei = line.find(')', ci);
		if (si != std::string::npos && ei != std::string::npos) {
			return line.substr(ci + 2, ei - ci - 2);
		}
	}
	return got;
}
#line 535
static std::string line;
#line 855

static inline bool line_is_wildcard(
	std::string& indent, std::string& extension
) {
	const char pattern[] = "//" " ...";
	auto idx = line.find(pattern);
	if (idx == std::string::npos) { return false; }
	indent = line.substr(0, idx);
	extension = line.substr(idx + sizeof(pattern) - 1);
	return true;
}
#line 724

static std::string change_cur_file_name(const std::string &file) {
	std::string result { file };
	size_t start { 0 };
	for (;;) {
		auto begin { line.find('`', start) };
		if (begin == std::string::npos) { break; }
		auto end { line.find('`', begin + 1) };
		if (end == std::string::npos) { break; }
		auto got { line.substr(begin + 1, end - begin - 1) };
		if (
			got.find('.') != std::string::npos ||
			got.find('/') != std::string::npos
		) { result = got; }
		start = end + 1;
	}
	return result;
}
#line 661

static bool starts_with(
	const std::string &base,
	const std::string &prefix
) {
	if (prefix.empty()) { return true; }
	return base.size() >= prefix.size() &&
		base.substr(0, prefix.size()) == prefix;
}
#line 536
static Line_Reader_Pool reader;

static bool next() {
	return reader.next(line);
}

bool err(const std::string& message) {
	std::cerr << reader.pos().file_name() << ':' << reader.pos().line() <<
		' ' << message << '\n';
	require(false && "error occurred");
	return false;
}

#line 494
#include "lazy-write/lazy-write.h"
#line 156
#include "solid/require.h"

#line 444
std::string get_extension(std::string path) {
	auto got { path.rfind('.') };
	if (got == std::string::npos) { return std::string { }; }
	return path.substr(got + 1);
}
#line 180
class Line {
		std::string value_;
		std::string file_;
		int number_;
	public:
		Line(const std::string &value, const std::string &file, int number):
			value_ { value }, file_ { file }, number_ { number }
		{ }
		const std::string &value() const { return value_; }
		const std::string &file() const { return file_; }
		int number() const { return number_; }
};
#line 231

class File : public std::vector<Line> {
#line 414
		static bool with_lines_(std::string name) {
			std::string ext { get_extension(name) };
			return ext == "h" || ext == "c" || ext == "cpp";
		}
#line 233
	public:
#line 420
		const bool with_lines;
#line 234
		File(const std::string &name):
#line 423
			with_lines { with_lines_(name) },
#line 235
			name { name }
		{ }
		const std::string name;
};
#line 1000

void push_parts(std::vector<std::string> &parts, const std::string &path) {
	if (path.empty()) { return; }

	std::istringstream in { path };
	std::string part;
	while (std::getline(in, part, '/')) {
		if (part == ".") { continue; }
		if (part == ".." && ! parts.empty()) {
			parts.pop_back();
			continue;
		}
		parts.push_back(part);
	}
}
#line 304

std::ostream& write_file_to_stream(const File &f, std::ostream& out) {
#line 1055
	bool skipping { false };
	std::string end_line { };
#line 358
	std::string name { "-" };
	int line { 1 };
#line 306
	for (const auto &l : f) {
#line 1059
		if (skipping) {
			if (l.value() == end_line) {
				skipping = false;
				continue;
			}
			continue;
		}
		auto idx { l.value().find("#if 0") };
		if (!write_raw && idx != std::string::npos) {
			bool contains_nonspace { false };
			for (
				auto i { l.value().begin() };
				i < l.value().begin() + idx; ++i
			) {
				if (*i > ' ') { contains_nonspace = true; break; }
			}
			if (! contains_nonspace) {
				skipping = true;
				end_line = l.value();
				end_line.replace(idx, 5, "#endif");
				continue;
			}
		}
#line 362
		if (line != l.number() || name != l.file()) {
			// write line macro
#line 432
			if (f.with_lines) {
#line 378
				out << "#line " << l.number();
				if (name != l.file()) { out << " \"" << l.file() << "\""; }
				out.put('\n');
#line 435
			}
#line 364
			line = l.number();
			name = l.file();
		}
#line 307
		out << l.value(); out.put('\n');
#line 369
		++line;
#line 308
	}
#line 1084
	if (skipping) { err("no #endif for #if"); return out; }
#line 309
	return out;
}
#line 499

inline void write_file(const File &f) {
	Lazy_Write out { f.name };
	write_file_to_stream(f, out);
}
#line 288

std::string write_file_to_string(const File &f) {
	std::ostringstream out;
	write_file_to_stream(f, out);
	return out.str();
}
#line 252

static std::map<std::string, File> pool;
#line 783

// patch helpers
#line 896

template<typename IT>
static inline bool do_wildcard(
	const std::string &indent,
	File &file,
	IT &cur,
	const std::string& extension
) {
	int repeats { 1 };
	if (extension.length() > 0 && std::isdigit(extension[0])) {
		repeats = 0;
		for (char c : extension) {
			if (! isdigit(c)) { return err("nodigit in numeric extension"); }
			int digit = c - '0';
			if ((std::numeric_limits<int>::max() - digit) / 10 < repeats) {
				return err("number too big");
			}
			repeats = repeats * 10 + digit;
		}
		repeats = extension[0] - '0';
	}
	if (extension == ".") { repeats = 0; }
	if (! next()) {
		return err("end of file after wildcard\n");
	}
	while (cur != file.end()) { 
		if (! starts_with(cur->value(), indent)) { break; }
		if (line != "```" && cur->value() == line) {
			++cur;
			if (! --repeats) { next(); break; }
		} else {
			++cur;
		}
	}
	return true;
}
#line 785

static inline bool read_patch(File &file) {
	if (! next()) { return false; }
	auto cur { file.begin() };
	std::string indent;
	while (line != "```") {
		// handle code
#line 809
		std::string extension;
		if (line_is_wildcard(indent, extension)) {
			// do wildcard
#line 844
			if (! do_wildcard(indent, file, cur, extension)) {
				return false;
			}
#line 812
			continue;
		} else {
			// insert line
#line 827
			cur = ++file.insert(
				cur, Line {
					line, reader.pos().file_name(), reader.pos().line()
				}
			);
#line 815
		}
#line 792
		if (! next()) { return err("end of file in code block"); }
	}
	if (cur != file.end()) { return err("incomplete patch"); }
	return next();
}

#line 254

#line 76
static inline void run_tests() {
	// unit-tests
#line 944
	{ // find file name in line
		std::string l { "a line with [bla](bla.md) a link" };
		std::string got { link_in_line(l) };
		require(got == "bla.md");
	}
#line 872
	{ // find wildcard
		line = " a //" " ...";
		std::string indent;
		std::string extension;
		require(line_is_wildcard(indent, extension));
		require(indent == " a ");
		require(extension == "");
	}
	{ // find super wildcard
		line = " a //" " ....";
		std::string indent;
		std::string extension;
		require(line_is_wildcard(indent, extension));
		require(indent == " a ");
		require(extension == ".");
	}
#line 763
	{ // parse simple code block
		std::istringstream in { "```c\nint a = 2;\nreturn a;\n```\n" };
		reader.push_front("x.md", in);
		next();
		require(line == "```c");
		File file { "out.c" };
		require(! read_patch(file));
		require(file.size() == 2);
		require(file[0].value() == "int a = 2;");
		require(file[1].value() == "return a;");
		reader = Line_Reader_Pool { };
	}
#line 710
	{ // multiple valid filename candidates
		line = "xx `first` xx `2nd.x` xx `` xx `last.x` xx";
		std::string f { change_cur_file_name("out.c") };
		require(f == "last.x");
	}
#line 682
	{ // no file name change in empty line
		line = "";
		std::string f { change_cur_file_name("out.c") };
		require(f == "out.c");
	}
	{ // no file name change in simple line
		line = "abc x.cpp";
		std::string f { change_cur_file_name("out.c") };
		require(f == "out.c");
	}
	{ // no file name change in non-path snippets
		line = "a `Makefile` b";
		std::string f { change_cur_file_name("out.c") };
		require(f == "out.c");
	}
	{ // multiple filename candidates
		line = "xx `first` xx `2nd.x` xx `` xx `last` xx";
		std::string f { change_cur_file_name("out.c") };
		require(f == "2nd.x");
	}
#line 608
	{ // reading lines with line macro
		Line_Reader_Pool pool;
		std::istringstream in { "abc\n#line 3 \"z\"\ndef" };
		pool.push_back("x", in);
		std::string line;
		require(pool.next(line));
		require(line == "abc");
		require(pool.pos().file_name() == "x");
		require(pool.pos().line() == 1);
		require(pool.next(line));
		require(line == "def");
		require(pool.pos().file_name() == "z");
		require(pool.pos().line() == 3);
		require(! pool.next(line));
	}
#line 583
	{ // reading lines from multiple files
		Line_Reader_Pool pool;
		std::istringstream in1 { "abc" };
		std::istringstream in2 { "def" };
		pool.push_back("x", in1);
		pool.push_back("y", in2);
		std::string line;
		require(pool.next(line));
		require(line == "abc");
		require(pool.pos().file_name() == "x");
		require(pool.pos().line() == 1);
		require(pool.next(line));
		require(line == "def");
		require(pool.pos().file_name() == "y");
		require(pool.pos().line() == 1);
		require(! pool.next(line));
	}
#line 559
	{ // reading lines
		Line_Reader_Pool pool;
		std::istringstream in { "abc\ndef\n" };
		pool.push_back("x", in);
		std::string line;
		require(pool.next(line));
		require(line == "abc");
		require(pool.pos().file_name() == "x");
		require(pool.pos().line() == 1);
		require(pool.next(line));
		require(line == "def");
		require(pool.pos().file_name() == "x");
		require(pool.pos().line() == 2);
		require(! pool.next(line));
	}
#line 473
	{ // different files
		File f { "out.c" };
		auto it = f.begin();
		it = ++f.insert(it, { "line 1", "-", 1 });
		it = ++f.insert(it, { "line 2", "other.md", 2 });
		auto c { write_file_to_string(f) };
		require(c == "line 1\n#line 2 \"other.md\"\nline 2\n");
	}
#line 457
	{ // not starting at one
		File f { "out.c" };
		auto it = f.begin();
		it = ++f.insert(it, { "line 1", "-", 4 });
		it = ++f.insert(it, { "line 2", "-", 5 });
		auto c { write_file_to_string(f) };
		require(c == "#line 4\nline 1\nline 2\n");
	}
#line 392
	{ // check that file extension is honoured
		File fc { "out.c" };
		require(fc.with_lines);
		File fh { "out.h" };
		require(fh.with_lines);
		File fcpp { "out.cpp" };
		require(fcpp.with_lines);
		File no { "out_c" };
		require(! no.with_lines);
		File fjava { "out.java" };
		require(! fjava.with_lines);
		File fm { "Makefile" };
		require(! fm.with_lines);
	}
#line 337
	{ // non-continuous file
		File f { "out.c" };
		auto it = f.begin();
		it = ++f.insert(it, { "line 1", "-", 1 });
		it = ++f.insert(it, { "line 2", "-", 10 });
		auto c { write_file_to_string(f) };
		require(c == "line 1\n#line 10\nline 2\n");
	}
#line 319
	{ // copy simple file
		File f { "out.c" };
		auto it = f.begin();
		it = ++f.insert(it, { "line 1", "-", 1 });
		it = ++f.insert(it, { "line 2", "-", 2 });
		auto c { write_file_to_string(f) };
		require(c == "line 1\nline 2\n");
	}
#line 269
	{ // write emtpy file
		const File f { "out.c" };
		auto c { write_file_to_string(f) };
		require(c == "");
	}
#line 208
	{ // check empty file
		File f { "out.cpp" };
		require(f.name == "out.cpp");
		require(f.begin() == f.end());
	}
#line 160
	{ // check Line attributes
		const Line line { "some-line", "some-file", 42 };
		require(line.value() == "some-line");
		require(line.file() == "some-file");
		require(line.number() == 42);
	}
#line 78
}

#line 44
int main(int argc, const char *argv[]) {
#line 95
	run_tests();
#line 1087
	if (argc >= 2 && argv[1] == std::string { "--raw" }) {
		write_raw = true; --argc; ++argv;
	}
#line 96
	if (argc == 2 && argv[1] == std::string { "--run-only-tests" }) {
		return EXIT_SUCCESS;
	}
#line 82
	run_tests();
#line 45
	// parse input
#line 633
	reader.populate(argc, argv);
	std::string cur_file { "out.txt" };
	if (next()) for (;;) {
		if (starts_with(line, "```") && line.length() > 3) {
			auto f { pool.find(cur_file) };
			if (f == pool.end()) {
				pool.insert({cur_file, File { cur_file }});
				f = pool.find(cur_file);
			}
			if (! read_patch(f->second)) { break; }
#line 1120
			if (cur_file == "/dev/null") { pool.erase(f); }
#line 643
		} else {
			cur_file = change_cur_file_name(cur_file);
#line 1101
			if (line == "<!-- MD-PATCHER EXIT -->") { break; }
#line 981
			auto sub { link_in_line(line) };
			if (sub.size() > 3 && sub.rfind(".md") == sub.size() - 3) {
				// normalize path
#line 1024
				std::vector<std::string> parts;
				if (! sub.empty() && sub[0] != '/') {
					push_parts(parts, cur_file);
					if (!parts.empty()) { parts.pop_back(); }
				}
				push_parts(parts, sub);
				std::ostringstream out;
				bool first { true };
				for (auto part : parts) {
					if (! first) { out << '/'; }
					out << part;
					first = false;
				}
				sub = out.str();

#line 984
				reader.push_front(sub);
			}
#line 645
			if (! next()) { break; }
		}
	}
#line 46
	// write output
#line 514
	for (const auto &f: pool) {
		write_file(f.second);
	}
#line 47
	return EXIT_SUCCESS;
}
