#line 34
#line 79
#line 148
#line 148
#line 148
#line 148
#line 148
#line 148
#line 148
#line 148
#line 148
#line 148
#line 148
#include "lazy-write.h"
#line 79
#include <map>
#include <vector>
#include <string>
#line 208
#include <iostream>
#include "line-reader.h"
#line 152

#line 211
static std::string line;
#line 273
#line 273
#line 273

#line 300
#line 467
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
#line 300
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
#line 274
static bool starts_with(
	const std::string &base,
	const std::string &prefix
) {
	if (prefix.empty()) { return true; }
	return base.size() >= prefix.size() &&
		base.substr(0, prefix.size()) == prefix;
}
#line 156
static Line_Reader reader { "", std::cin };

static bool next() {
	return reader.next(line);
}

std::ostream &err_pos() {
	return std::cerr << reader.pos().file_name() <<
		':' << reader.pos().line() << ' ';
}

// next defined
#line 153
using Lines = std::vector<std::string>;
using Files = std::map<std::string, Lines>;
static Files pool;
#line 34
#line 341

static Lines::iterator insert_before(
	const std::string &ins, Lines::iterator cur,
	Lines &lines
) {
	auto p { cur - lines.begin() };
	lines.insert(cur, ins);
	return lines.begin() + (p + 1);
}

// patch helpers

#line 487
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
#line 217
static inline bool read_patch(Lines &lines) {
	if (! next()) { return false; }
	Lines::iterator cur { lines.begin() };
	File_Position pos { "", 0 };
	File_Position old_pos { pos };
	std::string indent;
	while (line != "```") {
		// handle code
#line 383
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
#line 450
			if (! do_wildcard(
				indent, lines, cur, pos, old_pos
			)) { return false; }
#line 225
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
#line 424
			std::string line_macro {
				pos.line_macro(reader.pos())
			};
			if (! line_macro.empty()) {
				cur = insert_before(line_macro, cur, lines);
			}
			cur = insert_before(line, cur, lines);
			++pos;
#line 237
		}
#line 215
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
#line 34
#line 34
#line 34
#line 34
int main() {
	// parse input
#line 245
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
#line 173
	// write output
#line 153
	for (const auto &f: pool) {
		Lazy_Write out(f.first);
		for (const auto &l: f.second) {
			out << l; out.put('\n');
		}
	}
#line 89
	return 0;
}
