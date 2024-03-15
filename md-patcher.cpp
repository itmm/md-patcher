#line 36 "README.md"
#include <cstdlib>
#line 245
#include <map>
#line 286
#include <sstream>
#line 90
#include <string>
#line 222
#include <vector>
#line 37

#line 535
#include "lazy-write/lazy-write.h"
#line 569
#include "line-reader/line-reader.h"
#line 148
#include "solid/require.h"

#line 1035
bool write_raw { false };

#line 947
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
#line 572
static std::string line;
#line 696

#line 883
static inline bool line_is_wildcard(
	std::string &indent
) {
	auto idx = line.find("//" " ...");
	if (idx == std::string::npos) {
		return false;
	}
	indent = line.substr(0, idx);
	return true;
}
#line 742
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
		) {
			file = got;
		}
		start = end + 1;
	}
}
#line 697
static bool starts_with(
	const std::string &base,
	const std::string &prefix
) {
	if (prefix.empty()) { return true; }
	return base.size() >= prefix.size() &&
		base.substr(0, prefix.size()) == prefix;
}
#line 573
static Line_Reader_Pool reader;

static bool next() {
	return reader.next(line);
}

std::ostream &err_pos() {
	return std::cerr << reader.pos().file_name() <<
		':' << reader.pos().line() << ' ';
}

#line 487
std::string get_extension(std::string path) {
	auto got { path.rfind('.') };
	if (got == std::string::npos) { return std::string { }; }
	return path.substr(got + 1);
}
#line 174
class Line {
		std::string value_;
		std::string file_;
		int number_;
	public:
		Line(
			const std::string &value, const std::string &file,
			int number
		):
			value_ { value }, file_ { file }, number_ { number }
		{ }
		const std::string &value() const { return value_; }
		const std::string &file() const { return file_; }
		int number() const { return number_; }
};
#line 227

class File : public std::vector<Line> {
#line 447
		const bool with_lines_;
		static bool with_lines(std::string name) {
			std::string ext { get_extension(name) };
			return ext == "h" || ext == "c" || ext == "cpp";
		}
#line 229
		const std::string name_;
	public:
#line 454
		bool with_lines() const { return with_lines_; }
#line 346
		iterator insert(iterator pos, const Line &line) {
			auto p { pos - begin() };
			std::vector<Line>::insert(pos, line);
			return begin() + (p + 1);
		}	
#line 231
		File(const std::string &name):
#line 457
			with_lines_ { with_lines(name) },
#line 232
			name_ { name }
		{
			// init file attributes
		}
		const std::string& name() const { return name_; }
};
#line 250

#line 987
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

#line 308
template<typename ST>
#line 422
void put_num(ST &s, int num) {
	if (num) {
		put_num(s, num / 10);
		s.put((num % 10) + '0');
	}
}
template<typename ST>
#line 309
ST &write_file_to_stream(const File &f, ST &out) {
#line 1039
	bool skipping { false };
	std::string end_line { };
#line 384
	auto name { f.name() };
	int line { 1 };
#line 310
	for (const auto &l : f) {
#line 1043
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
#line 387
		if (line != l.number() || name != l.file()) {
			// write line macro
#line 472
			if (f.with_lines()) {
#line 405
				out << "#line ";
				put_num(out, l.number());
				if (name != l.file()) {
					out.put(' ');
					out.put('"');
					out << l.file();
					out.put('"');
				}
				out.put('\n');
#line 475
			}
#line 389
			line = l.number();
			name = l.file();
		}
#line 311
		out << l.value(); out.put('\n');
#line 393
		++line;
#line 312
	}
#line 1068
	require(! skipping);
#line 313
	return out;
}
#line 540

inline void write_file(const File &f) {
	Lazy_Write out { f.name() };
	write_file_to_stream(f, out);
}
#line 292
std::string write_file_to_string(const File &f) {
	std::ostringstream out;
	return write_file_to_stream(f, out).str();
}
#line 251
static std::map<std::string, File> pool;

#line 780
template<typename IT>
static IT insert_before(
	const std::string &ins, IT cur,
	File &file
) {
	auto p { cur - file.begin() };
	file.insert(cur, Line {
		ins, reader.pos().file_name(), reader.pos().line()
	});
	return file.begin() + (p + 1);
}

// patch helpers

#line 902
template<typename IT>
static inline bool do_wildcard(
	const std::string &indent,
	File &file,
	IT &cur
) {
	if (! next()) {
		err_pos() << "end of file after wildcard\n";
		return false;
	}
	while (cur != file.end()) { 
		if (! starts_with(cur->value(), indent)) { break; }
		if (line != "```" && cur->value() == line) { break; }
		++cur;
	}
	return true;
}
#line 794
static inline bool read_patch(File &file) {
	if (! next()) { return false; }
	auto cur { file.begin() };
	std::string indent;
	while (line != "```") {
		// handle code
#line 822
		if (line_is_wildcard(indent)) {
			// do wildcard
#line 866
			if (! do_wildcard(indent, file, cur)) {
				return false;
			}
#line 824
			continue;
		} else if (cur != file.end() && line == cur->value()) {
			++cur;
		} else {
			// insert line
#line 847
			cur = insert_before(line, cur, file);
#line 829
		}
#line 800
		if (! next()) {
			err_pos() << "end of file in code block\n";
			return false;
		}
	}
	if (cur != file.end()) {
		err_pos() << "incomplete patch\n";
		return false;
	}
	return next();
}
#line 75
static inline void run_tests() {
	// unit-tests
#line 932
	{ // find file name in line
		std::string l { "a line with [bla](bla.md) a link" };
		std::string got { link_in_line(l) };
		require(got == "bla.md");
	}
#line 722
	{ // multiple filename candidates
		line = "xx `first` xx `2nd.x` xx `` xx `last` xx";
		std::string f { "bla" };
		change_cur_file_name(f);
		require(f == "2nd.x");
	}
#line 641
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
#line 616
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
#line 593
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
#line 516
	{ // different files
		File f { "out.c" };
		auto it = f.begin();
		it = f.insert(it, { "line 1", "out.c", 1 });
		it = f.insert(it, { "line 2", "other.c", 2 });
		auto c { write_file_to_string(f) };
		require(c == "line 1\n#line 2 \"other.c\"\nline 2\n");
	}
#line 500
	{ // not starting at one
		File f { "out.c" };
		auto it = f.begin();
		it = f.insert(it, { "line 1", "out.c", 4 });
		it = f.insert(it, { "line 2", "out.c", 5 });
		auto c { write_file_to_string(f) };
		require(c == "#line 4\nline 1\nline 2\n");
	}
#line 366
	{ // non-continuous file
		File f { "out.c" };
		auto it = f.begin();
		it = f.insert(it, { "line 1", "out.c", 1 });
		it = f.insert(it, { "line 2", "out.c", 10 });
		auto c { write_file_to_string(f) };
		require(c == "line 1\n#line 10\nline 2\n");
	}
#line 327
	{ // copy simple file
		File f { "out.c" };
		auto it = f.begin();
		it = f.insert(it, { "line 1", "out.c", 1 });
		it = f.insert(it, { "line 2", "out.c", 2 });
		auto c { write_file_to_string(f) };
		require(c == "line 1\nline 2\n");
	}
#line 271
	{ // write emtpy file
		const File f { "out.c" };
		auto c { write_file_to_string(f) };
		require(c == "");
	}
#line 203
	{ // check empty file
		File f { "out.cpp" };
		require(f.name() == "out.cpp");
		require(f.begin() == f.end());
	}
#line 152
	{ // check Line attributes
		const Line line { "some-line", "some-file", 42 };
		require(line.value() == "some-line");
		require(line.file() == "some-file");
		require(line.number() == 42);
	}
#line 77
}

#line 38
int main(int argc, const char *argv[]) {
#line 80
	run_tests();
#line 1073
	if (argc >= 2 && argv[1] == std::string { "--raw" }) {
		write_raw = true; --argc; ++argv;
	}
#line 94
	if (argc == 2 && argv[1] == std::string { "--run-only-tests" }) {
		return EXIT_SUCCESS;
	}
#line 39
	// parse input
#line 665
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
#line 1085
			if (line == "<!-- MD-PATCHER EXIT -->") { break; }
#line 968
			auto sub { link_in_line(line) };
			if (
				sub.size() > 3 &&
				sub.rfind(".md") == sub.size() - 3
			) {
				// normalize path
#line 1004
				std::vector<std::string> parts;
				if (! sub.empty() && sub[0] != '/') {
					push_parts(parts, cur_file);
					if (!parts.empty()) {
						parts.pop_back();
					}
				}
				push_parts(parts, sub);
				std::ostringstream out;
				bool first { true };
				for (auto part : parts) {
					if (! first) {
						out << '/';
					}
					out << part;
					first = false;
				}
				sub = out.str();

#line 974
				reader.push_front(sub);
			}
#line 677
			if (! next()) { break; }
		}
	}
#line 40
	// write output
#line 553
	for (const auto &f: pool) {
		write_file(f.second);
	}
#line 41
	return EXIT_SUCCESS;
}
