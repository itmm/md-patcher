#line 32 "README.md"
#include <cstdlib>
#line 231
#include <map>
#line 265
#include <sstream>
#line 83
#include <string>
#line 211
#include <vector>
#line 33

#line 507
#include "lazy-write/lazy-write.h"
#line 540
#include "line-reader/line-reader.h"
#line 139
#include "solid/require.h"

#line 901
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
#line 543
static std::string line;
#line 668

#line 845
static inline bool line_is_wildcard(
	std::string &indent
) {
	auto idx = line.find("//" " ...");
	if (idx == std::string::npos) { return false; }
	indent = line.substr(0, idx);
	return true;
}
#line 723
static void change_cur_file_name(std::string &file) {
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
		) { file = got; }
		start = end + 1;
	}
}
#line 669
static bool starts_with(
	const std::string &base,
	const std::string &prefix
) {
	if (prefix.empty()) { return true; }
	return base.size() >= prefix.size() && base.substr(0, prefix.size()) == prefix;
}
#line 544
static Line_Reader_Pool reader;

static bool next() {
	return reader.next(line);
}

void err(const std::string& message) {
	std::cerr << reader.pos().file_name() << ':' << reader.pos().line() << ' ' <<
		message << '\n';
	require(false && "error occurred");
}

#line 459
std::string get_extension(std::string path) {
	auto got { path.rfind('.') };
	if (got == std::string::npos) { return std::string { }; }
	return path.substr(got + 1);
}
#line 165
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
#line 216

class File : public std::vector<Line> {
#line 425
		static bool with_lines_(std::string name) {
			std::string ext { get_extension(name) };
			return ext == "h" || ext == "c" || ext == "cpp";
		}
#line 218
	public:
#line 430
		const bool with_lines;
#line 219
		File(const std::string &name):
#line 433
			with_lines { with_lines_(name) },
#line 220
			name { name }
		{ }
		const std::string name;
};
#line 236

#line 288
template<typename ST>
#line 373
void recursive_put_num(ST &s, int num) {
	if (num) {
		recursive_put_num(s, num / 10);
		s.put((num % 10) + '0');
	}
}

template<typename ST>
void put_num(ST &s, int num) {
	if (num == 0) { s.put('0'); }
	else {
		if (num < 0) {
			s.put('-');
			if (num < -9) { recursive_put_num(s, num / -10); }
			s.put('0' - (num % 10));
		} else { recursive_put_num(s, num); }
	}
}

template<typename ST>
#line 289
ST &write_file_to_stream(const File &f, ST &out) {
#line 339
	std::string name { "-" };
	int line { 1 };
#line 290
	for (const auto &l : f) {
#line 342
		if (line != l.number() || name != l.file()) {
			// write line macro
#line 445
			if (f.with_lines) {
#line 361
				out << "#line ";
				put_num(out, l.number());
				if (name != l.file()) { out << " \"" << l.file() << "\""; }
				out.put('\n');
#line 448
			}
#line 344
			line = l.number();
			name = l.file();
		}
#line 291
		out << l.value(); out.put('\n');
#line 348
		++line;
#line 292
	}
	return out;
}
#line 512

inline void write_file(const File &f) {
	Lazy_Write out { f.name };
	write_file_to_stream(f, out);
}
#line 271
std::string write_file_to_string(const File &f) {
	std::ostringstream out;
	return write_file_to_stream(f, out).str();
}
#line 237
static std::map<std::string, File> pool;

#line 756
// patch helpers

#line 862
template<typename IT>
static inline bool do_wildcard(const std::string &indent, File &file, IT &cur) {
	if (! next()) { err("end of file after wildcard"); }
	while (cur != file.end()) { 
		if (! starts_with(cur->value(), indent)) { break; }
		if (line != "```" && cur->value() == line) { break; }
		++cur;
	}
	return true;
}
#line 758
static inline bool read_patch(File &file) {
	if (! next()) { return false; }
	auto cur { file.begin() };
	std::string indent;
	while (line != "```") {
		// handle code
#line 782
		if (line_is_wildcard(indent)) {
			// do wildcard
#line 831
			if (! do_wildcard(indent, file, cur)) { return false; }
#line 784
			continue;
		} else if (cur != file.end() && line == cur->value()) {
			// lines match
			++cur;
		} else {
			// insert line
#line 808
			cur = ++file.insert(
				cur, Line {
					line, reader.pos().file_name(), reader.pos().line()
				}
			);
#line 790
		}
#line 764
		if (! next()) { err("end of file in code block"); }
	}
	if (cur != file.end()) { err("incomplete patch"); }
	return next();
}

#line 68
static inline void run_tests() {
	// unit-tests
#line 884
	{ // find file name in line
		std::string l { "a line with [bla](bla.md) a link" };
		std::string got { link_in_line(l) };
		require(got == "bla.md");
	}
#line 703
	{ // multiple valid filename candidates
		line = "xx `first` xx `2nd.x` xx `` xx `last.x` xx";
		std::string f { "out.c" };
		change_cur_file_name(f);
		require(f == "last.x");
	}
#line 688
	{ // multiple filename candidates
		line = "xx `first` xx `2nd.x` xx `` xx `last` xx";
		std::string f { "out.c" };
		change_cur_file_name(f);
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
#line 488
	{ // different files
		File f { "out.c" };
		auto it = f.begin();
		it = ++f.insert(it, { "line 1", "-", 1 });
		it = ++f.insert(it, { "line 2", "other.md", 2 });
		auto c { write_file_to_string(f) };
		require(c == "line 1\n#line 2 \"other.md\"\nline 2\n");
	}
#line 472
	{ // not starting at one
		File f { "out.c" };
		auto it = f.begin();
		it = ++f.insert(it, { "line 1", "-", 4 });
		it = ++f.insert(it, { "line 2", "-", 5 });
		auto c { write_file_to_string(f) };
		require(c == "#line 4\nline 1\nline 2\n");
	}
#line 401
	{ // get_num tests
		std::ostringstream s;
		put_num(s, 0);
		require(s.str() == "0");
		s.str("");
		put_num(s, -9);
		require(s.str() == "-9");
		s.str("");
		put_num(s, 2147483647);
		require(s.str() == "2147483647");
		s.str("");
		put_num(s, -2147483648);
		require(s.str() == "-2147483648");
	}
#line 321
	{ // non-continuous file
		File f { "out.c" };
		auto it = f.begin();
		it = ++f.insert(it, { "line 1", "-", 1 });
		it = ++f.insert(it, { "line 2", "-", 10 });
		auto c { write_file_to_string(f) };
		require(c == "line 1\n#line 10\nline 2\n");
	}
#line 303
	{ // copy simple file
		File f { "out.c" };
		auto it = f.begin();
		it = ++f.insert(it, { "line 1", "-", 1 });
		it = ++f.insert(it, { "line 2", "-", 2 });
		auto c { write_file_to_string(f) };
		require(c == "line 1\nline 2\n");
	}
#line 251
	{ // write emtpy file
		const File f { "out.c" };
		auto c { write_file_to_string(f) };
		require(c == "");
	}
#line 191
	{ // check empty file
		File f { "out.cpp" };
		require(f.name == "out.cpp");
		require(f.begin() == f.end());
	}
#line 143
	{ // check Line attributes
		const Line line { "some-line", "some-file", 42 };
		require(line.value() == "some-line");
		require(line.file() == "some-file");
		require(line.number() == 42);
	}
#line 70
}

#line 34
int main(int argc, const char *argv[]) {
#line 73
	run_tests();
#line 87
	if (argc == 2 && argv[1] == std::string { "--run-only-tests" }) {
		return EXIT_SUCCESS;
	}
#line 35
	// parse input
#line 638
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
		} else {
			change_cur_file_name(cur_file);
#line 922
			auto sub { link_in_line(line) };
			if (sub.size() > 3 && sub.rfind(".md") == sub.size() - 3) {
				// normalize path
				reader.push_front(sub);
			}
#line 650
			if (! next()) { break; }
		}
	}
#line 36
	// write output
#line 525
	for (const auto &f: pool) {
		write_file(f.second);
	}
#line 37
	return EXIT_SUCCESS;
}
