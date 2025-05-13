#line 32 "README.md"
#include <cstdlib>
#line 231
#include <map>
#line 336
#include <ostream>
#line 265
#include <sstream>
#line 83
#include <string>
#line 211
#include <vector>
#line 33

#line 455
#include "lazy-write/lazy-write.h"
#line 488
#include "line-reader/line-reader.h"
#line 139
#include "solid/require.h"

#line 970
bool write_raw { false };

#line 878
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
#line 491
static std::string line;
#line 616

#line 796
static inline bool line_is_wildcard(
	std::string &indent
) {
	auto idx = line.find("//" " ...");
	if (idx == std::string::npos) { return false; }
	indent = line.substr(0, idx);
	return true;
}
#line 671
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
#line 617
static bool starts_with(
	const std::string &base,
	const std::string &prefix
) {
	if (prefix.empty()) { return true; }
	return base.size() >= prefix.size() && base.substr(0, prefix.size()) == prefix;
}
#line 492
static Line_Reader_Pool reader;

static bool next() {
	return reader.next(line);
}

void err(const std::string& message) {
	std::cerr << reader.pos().file_name() << ':' << reader.pos().line() << ' ' <<
		message << '\n';
	require(false && "error occurred");
}

#line 407
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
#line 373
		static bool with_lines_(std::string name) {
			std::string ext { get_extension(name) };
			return ext == "h" || ext == "c" || ext == "cpp";
		}
#line 218
	public:
#line 378
		const bool with_lines;
#line 219
		File(const std::string &name):
#line 381
			with_lines { with_lines_(name) },
#line 220
			name { name }
		{ }
		const std::string name;
};
#line 236

#line 918
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
#line 287
std::ostream& write_file_to_stream(const File &f, std::ostream& out) {
#line 974
	bool skipping { false };
	std::string end_line { };
#line 339
	std::string name { "-" };
	int line { 1 };
#line 288
	for (const auto &l : f) {
#line 978
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
#line 342
		if (line != l.number() || name != l.file()) {
			// write line macro
#line 393
			if (f.with_lines) {
#line 360
				out << "#line " << l.number();
				if (name != l.file()) { out << " \"" << l.file() << "\""; }
				out.put('\n');
#line 396
			}
#line 344
			line = l.number();
			name = l.file();
		}
#line 289
		out << l.value(); out.put('\n');
#line 348
		++line;
#line 290
	}
#line 1003
	if (skipping) { err("no #endif for #if"); }
#line 291
	return out;
}
#line 460

inline void write_file(const File &f) {
	Lazy_Write out { f.name };
	write_file_to_stream(f, out);
}
#line 271
std::string write_file_to_string(const File &f) {
	std::ostringstream out;
	write_file_to_stream(f, out);
	return out.str();
}
#line 237
static std::map<std::string, File> pool;

#line 704
// patch helpers

#line 831
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
		if (line != "```" && cur->value() == line && ! is_super) { break; }
		++cur;
	}
	return true;
}
#line 706
static inline bool read_patch(File &file) {
	if (! next()) { return false; }
	auto cur { file.begin() };
	std::string indent;
	while (line != "```") {
		// handle code
#line 730
		if (line_is_wildcard(indent)) {
			// do wildcard
#line 779
			bool is_super { line.find("//" " ....") != std::string::npos };
			if (! do_wildcard(indent, file, cur, is_super)) {
				return false;
			}
#line 732
			continue;
		} else if (cur != file.end() && line == cur->value()) {
			// lines match
			++cur;
		} else {
			// insert line
#line 756
			cur = ++file.insert(
				cur, Line {
					line, reader.pos().file_name(), reader.pos().line()
				}
			);
#line 738
		}
#line 712
		if (! next()) { err("end of file in code block"); }
	}
	if (cur != file.end()) { err("incomplete patch"); }
	return next();
}

#line 68
static inline void run_tests() {
	// unit-tests
#line 861
	{ // find file name in line
		std::string l { "a line with [bla](bla.md) a link" };
		std::string got { link_in_line(l) };
		require(got == "bla.md");
	}
#line 810
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
#line 651
	{ // multiple valid filename candidates
		line = "xx `first` xx `2nd.x` xx `` xx `last.x` xx";
		std::string f { "out.c" };
		change_cur_file_name(f);
		require(f == "last.x");
	}
#line 636
	{ // multiple filename candidates
		line = "xx `first` xx `2nd.x` xx `` xx `last` xx";
		std::string f { "out.c" };
		change_cur_file_name(f);
		require(f == "2nd.x");
	}
#line 562
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
#line 537
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
#line 513
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
#line 436
	{ // different files
		File f { "out.c" };
		auto it = f.begin();
		it = ++f.insert(it, { "line 1", "-", 1 });
		it = ++f.insert(it, { "line 2", "other.md", 2 });
		auto c { write_file_to_string(f) };
		require(c == "line 1\n#line 2 \"other.md\"\nline 2\n");
	}
#line 420
	{ // not starting at one
		File f { "out.c" };
		auto it = f.begin();
		it = ++f.insert(it, { "line 1", "-", 4 });
		it = ++f.insert(it, { "line 2", "-", 5 });
		auto c { write_file_to_string(f) };
		require(c == "#line 4\nline 1\nline 2\n");
	}
#line 319
	{ // non-continuous file
		File f { "out.c" };
		auto it = f.begin();
		it = ++f.insert(it, { "line 1", "-", 1 });
		it = ++f.insert(it, { "line 2", "-", 10 });
		auto c { write_file_to_string(f) };
		require(c == "line 1\n#line 10\nline 2\n");
	}
#line 301
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
#line 1008
	if (argc >= 2 && argv[1] == std::string { "--raw" }) {
		write_raw = true; --argc; ++argv;
	}
#line 87
	if (argc == 2 && argv[1] == std::string { "--run-only-tests" }) {
		return EXIT_SUCCESS;
	}
#line 35
	// parse input
#line 586
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
#line 1039
			if (cur_file == "/dev/null") { pool.erase(f); }
#line 596
		} else {
			change_cur_file_name(cur_file);
#line 1022
			if (line == "<!-- MD-PATCHER EXIT -->") { break; }
#line 899
			auto sub { link_in_line(line) };
			if (sub.size() > 3 && sub.rfind(".md") == sub.size() - 3) {
				// normalize path
#line 941
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

#line 902
				reader.push_front(sub);
			}
#line 598
			if (! next()) { break; }
		}
	}
#line 36
	// write output
#line 473
	for (const auto &f: pool) {
		write_file(f.second);
	}
#line 37
	return EXIT_SUCCESS;
}
