#include <fstream>
#include <map>
#include <vector>
#include <string>
#include <iostream>

static std::string line;

bool line_is_wildcard(std::string &indent) {
	auto idx = line.find("//" " ...");
	if (idx == std::string::npos) {
		return false;
	}
	indent = line.substr(0, idx);
	return true;
}
void change_file(std::string &file) {
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
bool starts_with(
	const std::string &base,
	const std::string &prefix
) {
	return base.size() >= prefix.size() &&
		base.substr(0, prefix.size()) == prefix;
}
static int line_nr { 0 };

bool next() {
	return std::getline(std::cin, line) && ++line_nr;
}

// next defined
using Lines = std::vector<std::string>;
using Files = std::map<std::string, Lines>;
static Files pool;

bool do_wildcard(
	const std::string &indent,
	Lines::iterator &cur,
	const Lines::const_iterator &end
) {
	if (! next()) {
		std::cerr << "end of file after wildcard\n";
		return false;
	}
	while (cur != end &&
		(line == "```" || *cur != line) &&
		starts_with(*cur, indent)
	) { ++cur; }
	return true;
}
bool read_patch(Lines &lines) {
	if (!next()) { return false; }
	Lines::iterator cur { lines.begin() };
	std::string indent;
	while (line != "```") {
		// handle code
		if (line_is_wildcard(indent)) {
			// do wildcard
			if (! do_wildcard(
				indent, cur, lines.end()
			)) { return false; }
			continue;
		} else if (cur != lines.end() && line == *cur) {
			++cur;
		} else {
			// insert line
			auto pos = cur - lines.begin();
			lines.insert(cur, line);
			cur = lines.begin() + (pos + 1);
		}
		if (! next()) {
			std::cerr << "end of file in code block\n";
			return false;
		}
	}
	return next();
}
int main() {
	// parse input
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
	// write output
	for (const auto &f: pool) {
		std::ofstream out(f.first.c_str());
		for (const auto &l: f.second) {
			out << l << '\n';
		}
	}
	return 0;
}
