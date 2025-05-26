#line 42 "README.md"
#include <cstdlib>
#line 249
#include <map>
#line 357
#include <ostream>
#line 285
#include <sstream>
#line 92
#include <string>
#line 229
#include <vector>
#line 43

#line 501
#include "lazy-write/lazy-write.h"
#line 538
#include "line-reader/line-reader.h"
#line 157
#include "solid/require.h"

#line 1067
bool write_raw { false };

#line 975
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
#line 541
static std::string line;
#line 670

#line 889
static inline bool line_is_wildcard(
	std::string &indent
) {
	auto idx = line.find("//" " ...");
	if (idx == std::string::npos) { return false; }
	indent = line.substr(0, idx);
	return true;
}
#line 738
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
#line 671
static bool starts_with(
	const std::string &base,
	const std::string &prefix
) {
	if (prefix.empty()) { return true; }
	return base.size() >= prefix.size() &&
		base.substr(0, prefix.size()) == prefix;
}
#line 542
static Line_Reader_Pool reader;
static bool do_match { true };

static bool next() {
	return reader.next(line);
}

void err(const std::string& message) {
	std::cerr << reader.pos().file_name() << ':' << reader.pos().line() <<
		' ' << message << '\n';
	require(false && "error occurred");
}

#line 451
std::string get_extension(std::string path) {
	auto got { path.rfind('.') };
	if (got == std::string::npos) { return std::string { }; }
	return path.substr(got + 1);
}
#line 183
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
#line 234

class File : public std::vector<Line> {
#line 417
		static bool with_lines_(std::string name) {
			std::string ext { get_extension(name) };
			return ext == "h" || ext == "c" || ext == "cpp";
		}
#line 236
	public:
#line 422
		const bool with_lines;
#line 237
		File(const std::string &name):
#line 425
			with_lines { with_lines_(name) },
#line 238
			name { name }
		{ }
		const std::string name;
};
#line 254

#line 1015
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
#line 307
std::ostream& write_file_to_stream(const File &f, std::ostream& out) {
#line 1071
	bool skipping { false };
	std::string end_line { };
#line 360
	std::string name { "-" };
	int line { 1 };
#line 308
	for (const auto &l : f) {
#line 1075
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
#line 363
		if (line != l.number() || name != l.file()) {
			// write line macro
#line 437
			if (f.with_lines) {
#line 381
				out << "#line " << l.number();
				if (name != l.file()) { out << " \"" << l.file() << "\""; }
				out.put('\n');
#line 440
			}
#line 365
			line = l.number();
			name = l.file();
		}
#line 309
		out << l.value(); out.put('\n');
#line 369
		++line;
#line 310
	}
#line 1100
	if (skipping) { err("no #endif for #if"); }
#line 311
	return out;
}
#line 506

inline void write_file(const File &f) {
	Lazy_Write out { f.name };
	write_file_to_stream(f, out);
}
#line 291
std::string write_file_to_string(const File &f) {
	std::ostringstream out;
	write_file_to_stream(f, out);
	return out.str();
}
#line 255
static std::map<std::string, File> pool;

#line 797
// patch helpers

#line 924
template<typename IT>
static inline bool do_wildcard(
	const std::string &indent,
	File &file,
	IT &cur,
	bool is_super
) {
	if (! next()) {
		err("end of file after wildcard\n");
		return false;
	}
	while (cur != file.end()) { 
		if (! starts_with(cur->value(), indent)) { break; }
		if (line != "```" && cur->value() == line && ! is_super) {
			++cur;
			next();
			break;
		}
		++cur;
	}
	return true;
}
#line 799
static inline bool read_patch(File &file) {
	if (! next()) { return false; }
	auto cur { file.begin() };
	std::string indent;
	while (line != "```") {
		// handle code
#line 823
		if (line_is_wildcard(indent)) {
			// do wildcard
#line 872
			bool is_super { line.find("//" " ....") != std::string::npos };
			if (! do_wildcard(indent, file, cur, is_super)) {
				return false;
			}
#line 825
			continue;
		} else if (do_match && cur != file.end() && line == cur->value()) {
			// lines match
			++cur;
		} else {
			// insert line
#line 849
			cur = ++file.insert(
				cur, Line {
					line, reader.pos().file_name(), reader.pos().line()
				}
			);
#line 831
		}
#line 805
		if (! next()) { err("end of file in code block"); }
	}
	if (cur != file.end()) { err("incomplete patch"); }
	return next();
}

#line 76
static inline void run_tests() {
	// unit-tests
#line 958
	{ // find file name in line
		std::string l { "a line with [bla](bla.md) a link" };
		std::string got { link_in_line(l) };
		require(got == "bla.md");
	}
#line 903
	{ // find wildcard
		line = " a //" " ...";
		std::string indent;
		require(line_is_wildcard(indent));
		require(indent == " a ");
	}
	{ // find super wildcard
		line = " a //" " ....";
		std::string indent;
		require(line_is_wildcard(indent));
		require(indent == " a ");
	}
#line 776
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
#line 719
	{ // multiple valid filename candidates
		line = "xx `first` xx `2nd.x` xx `` xx `last.x` xx";
		std::string f { change_cur_file_name("out.c") };
		require(f == "last.x");
	}
#line 691
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
#line 614
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
#line 589
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
#line 565
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
#line 480
	{ // different files
		File f { "out.c" };
		auto it = f.begin();
		it = ++f.insert(it, { "line 1", "-", 1 });
		it = ++f.insert(it, { "line 2", "other.md", 2 });
		auto c { write_file_to_string(f) };
		require(c == "line 1\n#line 2 \"other.md\"\nline 2\n");
	}
#line 464
	{ // not starting at one
		File f { "out.c" };
		auto it = f.begin();
		it = ++f.insert(it, { "line 1", "-", 4 });
		it = ++f.insert(it, { "line 2", "-", 5 });
		auto c { write_file_to_string(f) };
		require(c == "#line 4\nline 1\nline 2\n");
	}
#line 395
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
#line 339
	{ // non-continuous file
		File f { "out.c" };
		auto it = f.begin();
		it = ++f.insert(it, { "line 1", "-", 1 });
		it = ++f.insert(it, { "line 2", "-", 10 });
		auto c { write_file_to_string(f) };
		require(c == "line 1\n#line 10\nline 2\n");
	}
#line 321
	{ // copy simple file
		File f { "out.c" };
		auto it = f.begin();
		it = ++f.insert(it, { "line 1", "-", 1 });
		it = ++f.insert(it, { "line 2", "-", 2 });
		auto c { write_file_to_string(f) };
		require(c == "line 1\nline 2\n");
	}
#line 271
	{ // write emtpy file
		const File f { "out.c" };
		auto c { write_file_to_string(f) };
		require(c == "");
	}
#line 211
	{ // check empty file
		File f { "out.cpp" };
		require(f.name == "out.cpp");
		require(f.begin() == f.end());
	}
#line 161
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
#line 81
	run_tests();
#line 1105
	if (argc >= 2 && argv[1] == std::string { "--raw" }) {
		write_raw = true; --argc; ++argv;
	}
#line 96
	if (argc == 2 && argv[1] == std::string { "--run-only-tests" }) {
		return EXIT_SUCCESS;
	}
#line 45
	// parse input
#line 639
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
#line 1136
			if (cur_file == "/dev/null") { pool.erase(f); }
#line 649
		} else {
			cur_file = change_cur_file_name(cur_file);
#line 1119
			if (line == "<!-- MD-PATCHER EXIT -->") { break; }
#line 996
			auto sub { link_in_line(line) };
			if (sub.size() > 3 && sub.rfind(".md") == sub.size() - 3) {
				// normalize path
#line 1038
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

#line 999
				reader.push_front(sub);
			}
#line 651
			if (! next()) { break; }
		}
	}
#line 46
	// write output
#line 521
	for (const auto &f: pool) {
		write_file(f.second);
	}
#line 47
	return EXIT_SUCCESS;
}
