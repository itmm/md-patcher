#line 33 "README.md"
#include <cstdlib>
#line 226
#include <map>
#line 331
#include <ostream>
#line 260
#include <sstream>
#line 78
#include <string>
#line 206
#include <vector>
#line 34

#line 450
#include "lazy-write/lazy-write.h"
#line 483
#include "line-reader/line-reader.h"
#line 134
#include "solid/require.h"

#line 965
bool write_raw { false };

#line 873
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
#line 486
static std::string line;
#line 611

#line 791
static inline bool line_is_wildcard(
	std::string &indent
) {
	auto idx = line.find("//" " ...");
	if (idx == std::string::npos) { return false; }
	indent = line.substr(0, idx);
	return true;
}
#line 666
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
#line 612
static bool starts_with(
	const std::string &base,
	const std::string &prefix
) {
	if (prefix.empty()) { return true; }
	return base.size() >= prefix.size() && base.substr(0, prefix.size()) == prefix;
}
#line 487
static Line_Reader_Pool reader;

static bool next() {
	return reader.next(line);
}

void err(const std::string& message) {
	std::cerr << reader.pos().file_name() << ':' << reader.pos().line() << ' ' <<
		message << '\n';
	require(false && "error occurred");
}

#line 402
std::string get_extension(std::string path) {
	auto got { path.rfind('.') };
	if (got == std::string::npos) { return std::string { }; }
	return path.substr(got + 1);
}
#line 160
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
#line 211

class File : public std::vector<Line> {
#line 368
		static bool with_lines_(std::string name) {
			std::string ext { get_extension(name) };
			return ext == "h" || ext == "c" || ext == "cpp";
		}
#line 213
	public:
#line 373
		const bool with_lines;
#line 214
		File(const std::string &name):
#line 376
			with_lines { with_lines_(name) },
#line 215
			name { name }
		{ }
		const std::string name;
};
#line 231

#line 913
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
#line 282
std::ostream& write_file_to_stream(const File &f, std::ostream& out) {
#line 969
	bool skipping { false };
	std::string end_line { };
#line 334
	std::string name { "-" };
	int line { 1 };
#line 283
	for (const auto &l : f) {
#line 973
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
#line 337
		if (line != l.number() || name != l.file()) {
			// write line macro
#line 388
			if (f.with_lines) {
#line 355
				out << "#line " << l.number();
				if (name != l.file()) { out << " \"" << l.file() << "\""; }
				out.put('\n');
#line 391
			}
#line 339
			line = l.number();
			name = l.file();
		}
#line 284
		out << l.value(); out.put('\n');
#line 343
		++line;
#line 285
	}
#line 998
	if (skipping) { err("no #endif for #if"); }
#line 286
	return out;
}
#line 455

inline void write_file(const File &f) {
	Lazy_Write out { f.name };
	write_file_to_stream(f, out);
}
#line 266
std::string write_file_to_string(const File &f) {
	std::ostringstream out;
	write_file_to_stream(f, out);
	return out.str();
}
#line 232
static std::map<std::string, File> pool;

#line 699
// patch helpers

#line 826
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
#line 701
static inline bool read_patch(File &file) {
	if (! next()) { return false; }
	auto cur { file.begin() };
	std::string indent;
	while (line != "```") {
		// handle code
#line 725
		if (line_is_wildcard(indent)) {
			// do wildcard
#line 774
			bool is_super { line.find("//" " ....") != std::string::npos };
			if (! do_wildcard(indent, file, cur, is_super)) {
				return false;
			}
#line 727
			continue;
		} else if (cur != file.end() && line == cur->value()) {
			// lines match
			++cur;
		} else {
			// insert line
#line 751
			cur = ++file.insert(
				cur, Line {
					line, reader.pos().file_name(), reader.pos().line()
				}
			);
#line 733
		}
#line 707
		if (! next()) { err("end of file in code block"); }
	}
	if (cur != file.end()) { err("incomplete patch"); }
	return next();
}

#line 63
static inline void run_tests() {
	// unit-tests
#line 856
	{ // find file name in line
		std::string l { "a line with [bla](bla.md) a link" };
		std::string got { link_in_line(l) };
		require(got == "bla.md");
	}
#line 805
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
#line 646
	{ // multiple valid filename candidates
		line = "xx `first` xx `2nd.x` xx `` xx `last.x` xx";
		std::string f { "out.c" };
		change_cur_file_name(f);
		require(f == "last.x");
	}
#line 631
	{ // multiple filename candidates
		line = "xx `first` xx `2nd.x` xx `` xx `last` xx";
		std::string f { "out.c" };
		change_cur_file_name(f);
		require(f == "2nd.x");
	}
#line 557
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
#line 532
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
#line 508
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
#line 431
	{ // different files
		File f { "out.c" };
		auto it = f.begin();
		it = ++f.insert(it, { "line 1", "-", 1 });
		it = ++f.insert(it, { "line 2", "other.md", 2 });
		auto c { write_file_to_string(f) };
		require(c == "line 1\n#line 2 \"other.md\"\nline 2\n");
	}
#line 415
	{ // not starting at one
		File f { "out.c" };
		auto it = f.begin();
		it = ++f.insert(it, { "line 1", "-", 4 });
		it = ++f.insert(it, { "line 2", "-", 5 });
		auto c { write_file_to_string(f) };
		require(c == "#line 4\nline 1\nline 2\n");
	}
#line 314
	{ // non-continuous file
		File f { "out.c" };
		auto it = f.begin();
		it = ++f.insert(it, { "line 1", "-", 1 });
		it = ++f.insert(it, { "line 2", "-", 10 });
		auto c { write_file_to_string(f) };
		require(c == "line 1\n#line 10\nline 2\n");
	}
#line 296
	{ // copy simple file
		File f { "out.c" };
		auto it = f.begin();
		it = ++f.insert(it, { "line 1", "-", 1 });
		it = ++f.insert(it, { "line 2", "-", 2 });
		auto c { write_file_to_string(f) };
		require(c == "line 1\nline 2\n");
	}
#line 246
	{ // write emtpy file
		const File f { "out.c" };
		auto c { write_file_to_string(f) };
		require(c == "");
	}
#line 186
	{ // check empty file
		File f { "out.cpp" };
		require(f.name == "out.cpp");
		require(f.begin() == f.end());
	}
#line 138
	{ // check Line attributes
		const Line line { "some-line", "some-file", 42 };
		require(line.value() == "some-line");
		require(line.file() == "some-file");
		require(line.number() == 42);
	}
#line 65
}

#line 35
int main(int argc, const char *argv[]) {
#line 68
	run_tests();
#line 1003
	if (argc >= 2 && argv[1] == std::string { "--raw" }) {
		write_raw = true; --argc; ++argv;
	}
#line 82
	if (argc == 2 && argv[1] == std::string { "--run-only-tests" }) {
		return EXIT_SUCCESS;
	}
#line 36
	// parse input
#line 581
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
#line 1034
			if (cur_file == "/dev/null") { pool.erase(f); }
#line 591
		} else {
			change_cur_file_name(cur_file);
#line 1017
			if (line == "<!-- MD-PATCHER EXIT -->") { break; }
#line 894
			auto sub { link_in_line(line) };
			if (sub.size() > 3 && sub.rfind(".md") == sub.size() - 3) {
				// normalize path
#line 936
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

#line 897
				reader.push_front(sub);
			}
#line 593
			if (! next()) { break; }
		}
	}
#line 37
	// write output
#line 468
	for (const auto &f: pool) {
		write_file(f.second);
	}
#line 38
	return EXIT_SUCCESS;
}
