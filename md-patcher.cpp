#line 35 "README.md"
#line 92 "README.md"
#line 151 "README.md"
#line 151 "README.md"
#line 151 "README.md"
#line 151 "README.md"
#line 151 "README.md"
#line 267 "README.md"
#line 267 "README.md"
#line 267 "README.md"
#line 267 "README.md"
#line 267 "README.md"
#line 267 "README.md"
#line 267 "README.md"
#line 267 "README.md"
#line 267 "README.md"
#line 267 "README.md"
#line 267 "README.md"
#include "lazy-write.h"
#line 151
#include <cassert>
#line 92
#include <string>
#line 168
#line 327
#include <iostream>
#include "line-reader.h"

static std::string line;
#line 393
#line 393
#line 393

#line 420
#line 587
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
#line 420
static void change_file(std::string &file) {
	const auto last_idx { line.rfind('`') };
	if (last_idx != std::string::npos && last_idx > 0) {
		const auto start_idx { line.rfind(
			'`', last_idx - 1
		) + 1 };
		if (start_idx != std::string::npos &&
			start_idx < last_idx
		) {
			file = line.substr(
				start_idx, last_idx - start_idx
			);
		}
	}
}
#line 394
static bool starts_with(
	const std::string &base,
	const std::string &prefix
) {
	if (prefix.empty()) { return true; }
	return base.size() >= prefix.size() &&
		base.substr(0, prefix.size()) == prefix;
}
#line 274
static Line_Reader reader { "", std::cin };

static bool next() {
	return reader.next(line);
}

std::ostream &err_pos() {
	return std::cerr << reader.pos().file_name() <<
		':' << reader.pos().line() << ' ';
}

// next defined
#line 168
class Line {
		const std::string value_;
		const std::string file_;
		const int number_;
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
#line 153
#line 207
#include <vector>
class File {
		std::string name_;
		std::vector<Line> lines_;
	public:
		File(const std::string &name): name_ { name } { }
		const std::string &name() const { return name_; }
		auto begin() { return lines_.begin(); }
		auto begin() const { return lines_.begin(); }
		auto end() { return lines_.end(); }
		auto end() const { return lines_.end(); }
};
#line 153
#line 231
#include <map>

using Lines = std::vector<std::string>;
using Files = std::map<std::string, Lines>;
static Files pool;
#line 153
#line 461

static Lines::iterator insert_before(
	const std::string &ins, Lines::iterator cur,
	Lines &lines
) {
	auto p { cur - lines.begin() };
	lines.insert(cur, ins);
	return lines.begin() + (p + 1);
}

// patch helpers

#line 607
static inline bool do_wildcard(
	const std::string &indent,
	Lines &lines,
	Lines::iterator &cur,
	File_Position &pos,
	File_Position &old_pos
) {
	if (! next()) {
		err_pos() << "end of file after wildcard\n";
		return false;
	}
	while (cur != lines.end()) { 
		std::string macro {
			pos.line_macro(old_pos)
		};
		if (! macro.empty()) {
			cur = insert_before(macro, cur, lines);
		}
		File_Position fp { pos.parse_line_macro(*cur) };
		if (fp) {
			++cur;
			continue;
		}
		if (! starts_with(*cur, indent)) { break; }
		if (line != "```" && *cur == line) { break; }
		++cur;
		++pos;
		old_pos = pos;
	}
	return true;
}
#line 364
static inline bool read_patch(Lines &lines) {
	if (! next()) { return false; }
	Lines::iterator cur { lines.begin() };
	File_Position pos { "", 0 };
	File_Position old_pos { pos };
	std::string indent;
	while (line != "```") {
		// handle code
#line 503
		if (cur != lines.end()) {
			File_Position xp { old_pos.parse_line_macro(*cur) };
			if (xp) {
				old_pos = xp;
				++cur;
				continue;
			}
		}
		if (line_is_wildcard(indent)) {
			// do wildcard
#line 570
			if (! do_wildcard(
				indent, lines, cur, pos, old_pos
			)) { return false; }
#line 372
			continue;
		} else if (cur != lines.end() && line == *cur) {
			std::string line_macro {
				pos.line_macro(old_pos)
			};
			if (! line_macro.empty()) {
				cur = insert_before(line_macro, cur, lines);
			}
			++cur;
			++pos; old_pos = pos;
		} else {
			// insert line
#line 544
			std::string line_macro {
				pos.line_macro(reader.pos())
			};
			if (! line_macro.empty()) {
				cur = insert_before(line_macro, cur, lines);
			}
			cur = insert_before(line, cur, lines);
			++pos;
#line 384
		}
#line 362
		if (! next()) {
			err_pos() << "end of file in code block\n";
			return false;
		}
	}
	if (cur != lines.end()) {
		err_pos() << "incomplete patch\n";
		return false;
	}
	return next();
}
#line 153
#line 153
#line 153
#line 153
static inline void run_tests() {
	// unit-tests
#line 154
#line 191
	{ // check empty file
		File f { "out.cpp" };
		assert(f.name() == "out.cpp");
		assert(f.begin() == f.end());
	}
#line 154
	{ // check Line attributes
		const Line line { "some-line", "some-file", 42 };
		assert(line.value() == "some-line");
		assert(line.file() == "some-file");
		assert(line.number() == 42);
	}
#line 95
}
#line 35
int main(int argc, const char *argv[]) {
#line 97
#line 97
#line 97
	run_tests();
	if (argc == 2 && argv[1] == std::string { "--run-only-tests" }) {
		return 0;
	}
#line 36
	// parse input
#line 365
	std::string cur_file { "out.txt" };
	if (next()) for (;;) {
		if (starts_with(line, "```") &&
			line.length() > 3
		) {
			if (! read_patch(pool[cur_file])) { break; }
		} else {
			change_file(cur_file);
			if (! next()) { break; }
		}
	}
#line 102
	// write output
#line 272
	for (const auto &f: pool) {
		Lazy_Write out(f.first);
		for (const auto &l: f.second) {
			out << l; out.put('\n');
		}
	}
#line 103
	return 0;
}
