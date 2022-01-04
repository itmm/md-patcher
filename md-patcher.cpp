#line 150 "README.md"
#include <cassert>
#line 91
#include <string>
#line 479
#include <iostream>
#include "line-reader.h"

static std::string line;
#line 548

#line 724
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
#line 575
static void change_file(std::string &file) {
	const auto last_idx { line.rfind('`') };
	if (last_idx != std::string::npos && last_idx > 0) {
		const auto start_idx { line.rfind(
			'`', last_idx - 1
		) + 1 };
		if (start_idx != std::string::npos &&
			start_idx < last_idx
		) {
			auto got { line.substr(
				start_idx, last_idx - start_idx
			) };
			if (got.find('.') != std::string::npos || got.find('/') != std::string::npos) {
				file = got;
			}
		}
	}
}
#line 549
static bool starts_with(
	const std::string &base,
	const std::string &prefix
) {
	if (prefix.empty()) { return true; }
	return base.size() >= prefix.size() &&
		base.substr(0, prefix.size()) == prefix;
}
#line 483
static Line_Reader_Pool reader;

static bool next() {
	return reader.next(line);
}

std::ostream &err_pos() {
	return std::cerr << reader.pos().file_name() <<
		':' << reader.pos().line() << ' ';
}

// next defined
#line 167
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
#line 206
#include <vector>
class File {
		std::string name_;
		using Lines = std::vector<Line>;
		Lines lines_;
	public:
		File(const std::string &name): name_ { name } { }
		const std::string &name() const { return name_; }
		auto begin() { return lines_.begin(); }
#line 318
		using iterator = Lines::iterator;
		iterator insert(iterator pos, const Line &line) {
			auto p { pos - begin() };
			lines_.insert(pos, line);
			return begin() + (p + 1);
		}	
#line 215
		auto begin() const { return lines_.begin(); }
		auto end() { return lines_.end(); }
		auto end() const { return lines_.end(); }
};
#line 282
template<typename ST>
#line 395
void put_num(ST &s, int num) {
	if (num) {
		put_num(s, num / 10);
		s.put((num % 10) + '0');
	}
}
template<typename ST>
#line 283
ST &write_file_to_stream(const File &f, ST &out) {
#line 355
	auto name { f.name() };
	int line { 1 };
#line 284
	for (const auto &l : f) {
#line 358
		if (line != l.number() || name != l.file()) {
			// write line macro
#line 375
			out << "#line ";
			put_num(out, l.number());
			if (name != l.file()) {
				out.put(' ');
				out.put('"');
				out << l.file();
				out.put('"');
			}
			out.put('\n');
			line = l.number();
			name = l.file();
#line 360
		}
#line 285
		out << l.value(); out.put('\n');
#line 362
		++line;
#line 286
	}
	return out;
}
#line 450
#include "lazy-write.h"

inline void write_file(const File &f) {
	Lazy_Write out { f.name() };
	write_file_to_stream(f, out);
}
#line 265
#include <sstream>
std::string write_file_to_string(const File &f) {
	std::ostringstream out;
	return write_file_to_stream(f, out).str();
}
#line 230
#include <map>

static std::map<std::string, File> pool;
#line 619

template<typename IT>
static IT insert_before(
	const std::string &ins, IT cur,
	File &file
) {
	auto p { cur - file.begin() };
	file.insert(cur, Line {
		ins, reader.pos().file_name(), reader.pos().line() - 1
	});
	return file.begin() + (p + 1);
}

// patch helpers

#line 744
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
#line 634
static inline bool read_patch(File &file) {
	if (! next()) { return false; }
	auto cur { file.begin() };
	std::string indent;
	while (line != "```") {
		// handle code
#line 662
		if (line_is_wildcard(indent)) {
			// do wildcard
#line 707
			if (! do_wildcard(indent, file, cur)) {
				return false;
			}
#line 664
			continue;
		} else if (cur != file.end() && line == cur->value()) {
			++cur;
		} else {
			// insert line
#line 688
			cur = insert_before(line, cur, file);
#line 669
		}
#line 640
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
#line 92
static inline void run_tests() {
	// unit-tests
#line 429
	{ // different files
		File f { "out.txt" };
		auto it = f.begin();
		it = f.insert(it, { "line 1", "out.txt", 1 });
		it = f.insert(it, { "line 2", "other.txt", 2 });
		auto c { write_file_to_string(f) };
		assert(c == "line 1\n#line 2 \"other.txt\"\nline 2\n");
	}
#line 412
	{ // not starting at one
		File f { "out.txt" };
		auto it = f.begin();
		it = f.insert(it, { "line 1", "out.txt", 4 });
		it = f.insert(it, { "line 2", "out.txt", 5 });
		auto c { write_file_to_string(f) };
		assert(c == "#line 4\nline 1\nline 2\n");
	}
#line 337
	{ // non-continuous file
		File f { "out.txt" };
		auto it = f.begin();
		it = f.insert(it, { "line 1", "out.txt", 1 });
		it = f.insert(it, { "line 2", "out.txt", 10 });
		auto c { write_file_to_string(f) };
		assert(c == "line 1\n#line 10\nline 2\n");
	}
#line 297
	{ // copy simple file
		File f { "out.txt" };
		auto it = f.begin();
		it = f.insert(it, { "line 1", "out.txt", 1 });
		it = f.insert(it, { "line 2", "out.txt", 2 });
		auto c { write_file_to_string(f) };
		assert(c == "line 1\nline 2\n");
	}
#line 248
	{ // write emtpy file
		const File f { "out.txt" };
		auto c { write_file_to_string(f) };
		assert(c == "");
	}
#line 190
	{ // check empty file
		File f { "out.cpp" };
		assert(f.name() == "out.cpp");
		assert(f.begin() == f.end());
	}
#line 153
	{ // check Line attributes
		const Line line { "some-line", "some-file", 42 };
		assert(line.value() == "some-line");
		assert(line.file() == "some-file");
		assert(line.number() == 42);
	}
#line 94
}
#line 34
int main(int argc, const char *argv[]) {
#line 96
	run_tests();
	if (argc == 2 && argv[1] == std::string { "--run-only-tests" }) {
		return 0;
	}
#line 35
	// parse input
#line 515
	reader.populate(argc, argv);
	std::string cur_file { "out.txt" };
	if (next()) for (;;) {
		if (starts_with(line, "```") &&
			line.length() > 3
		) {
			auto f { pool.find(cur_file) };
			if (f == pool.end()) {
				pool.insert({cur_file, File { cur_file }});
				f = pool.find(cur_file);
			}
			if (! read_patch(f->second)) { break; }
		} else {
			change_file(cur_file);
			if (! next()) { break; }
		}
	}
#line 36
	// write output
#line 462
	for (const auto &f: pool) {
		write_file(f.second);
	}
#line 37
	return 0;
}
