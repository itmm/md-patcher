#line 35 "README.md"
#line 92 "README.md"
#line 151 "README.md"
#line 151 "README.md"
#line 151 "README.md"
#line 151 "README.md"
#line 151 "README.md"
#line 270 "README.md"
#line 270 "README.md"
#line 270 "README.md"
#line 270 "README.md"
#line 270 "README.md"
#line 270 "README.md"
#line 270 "README.md"
#line 270 "README.md"
#line 270 "README.md"
#line 270 "README.md"
#line 270 "README.md"
#include "lazy-write.h"
#line 151
#include <cassert>
#line 92
#include <string>
#line 168
#line 331
#include <iostream>
#include "line-reader.h"

static std::string line;
#line 402
#line 402
#line 402

#line 429
#line 598
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
#line 429
static void change_file(std::string &file) {
	const auto last_idx { line.rfind('`') };
	if (last_idx != std::string::npos && last_idx > 0) {
		const auto start_idx { line.rfind(
			'`', last_idx - 1
		) + 1 };
		if (start_idx != std::string::npos &&
			start_idx < last_idx
		) {
			// TODO: only change if contains '.' or '/'
			file = line.substr(
				start_idx, last_idx - start_idx
			);
		}
	}
}
#line 403
static bool starts_with(
	const std::string &base,
	const std::string &prefix
) {
	if (prefix.empty()) { return true; }
	return base.size() >= prefix.size() &&
		base.substr(0, prefix.size()) == prefix;
}
#line 277
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
		auto insert(std::vector<Line>::iterator pos, const Line &line) {
			return lines_.insert(pos, line);
		}	
};
#line 153
#line 234
#include <map>

using Lines = std::vector<std::string>;
using Files = std::map<std::string, File>;
static Files pool;
#line 153
#line 471

template<typename IT>
static IT insert_before(
	const std::string &ins, IT cur,
	File &file
) {
	auto p { cur - file.begin() };
	file.insert(cur, Line { ins, reader.pos().file_name(), reader.pos().line() });
	return file.begin() + (p + 1);
}

// patch helpers

#line 618
template<typename IT>
static inline bool do_wildcard(
	const std::string &indent,
	File &file,
	IT &cur,
	File_Position &pos,
	File_Position &old_pos
) {
	if (! next()) {
		err_pos() << "end of file after wildcard\n";
		return false;
	}
	while (cur != file.end()) { 
		std::string macro {
			pos.line_macro(old_pos)
		};
		if (! macro.empty()) {
			cur = insert_before(macro, cur, file);
		}
		File_Position fp { pos.parse_line_macro(cur->value()) };
		if (fp) {
			++cur;
			continue;
		}
		if (! starts_with(cur->value(), indent)) { break; }
		if (line != "```" && cur->value() == line) { break; }
		++cur;
		++pos;
		old_pos = pos;
	}
	return true;
}
#line 372
static inline bool read_patch(File &file) {
	if (! next()) { return false; }
	auto cur { file.begin() };
	File_Position pos { "", 0 };
	File_Position old_pos { pos };
	std::string indent;
	while (line != "```") {
		// handle code
#line 514
		if (cur != file.end()) {
			File_Position xp { old_pos.parse_line_macro(cur->value()) };
			if (xp) {
				old_pos = xp;
				++cur;
				continue;
			}
		}
		if (line_is_wildcard(indent)) {
			// do wildcard
#line 581
			if (! do_wildcard(
				indent, file, cur, pos, old_pos
			)) { return false; }
#line 380
			continue;
		} else if (cur != file.end() && line == cur->value()) {
			std::string line_macro {
				pos.line_macro(old_pos)
			};
			if (! line_macro.empty()) {
				cur = insert_before(line_macro, cur, file);
			}
			++cur;
			++pos; old_pos = pos;
		} else {
			// insert line
#line 555
			std::string line_macro {
				pos.line_macro(reader.pos())
			};
			if (! line_macro.empty()) {
				cur = insert_before(line_macro, cur, file);
			}
			cur = insert_before(line, cur, file);
			++pos;
#line 392
		}
#line 370
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
#line 369
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
#line 102
	// write output
#line 275
	for (const auto &f: pool) {
		Lazy_Write out(f.first);
		for (const auto &l: f.second) {
			// TODO: add #line statements
			out << l.value(); out.put('\n');
		}
	}
#line 103
	return 0;
}
