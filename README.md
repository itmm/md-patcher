# md-patcher

`md-patcher` is a program, to extract code fragments out of Markdown files and
combine them to a compilable program. That is a successor of
[hex](https://github.com/itmm/hex) with a far clearer syntax, but also with
slightly smaller expressive power.

## General Structure

I use an easy structure for `md-patcher`: I read Markdown files and combines
the included code fragments to whole source code files. Then I write the
resulting files out. I use normal inline code fragments to specify the file
names of the generated files: each inline code fragment that contains at least
a period or slash changes the current file name.

Sometimes I am unsure, if `md-patcher` and I agree about the current file name.
So I add the file name as an inline code fragment directly before the code
fragment. As a benefit, it is also easier for the reader to see to with file
the fragment belongs.

**Caution**: `md-patcher` does not check, if the file name makes sense. Also
in `md-patcher` I will not create any directories that are missing in the path.

By the way: this Markdown file can be used to extract the full source code
of `md-patcher`. But for that I need a running version of `md-patcher`. So
the generated source code is also part of the repository to ease the
bootstrap process.

I do not need to create the files in one go. I can modify and extend fragments
while going through this document. So I can start with a birds eye view and
drill down later. At every step I hopefully have a buildable program.

I started `md-patcher.cpp` this way:

```c++
#include <cstdlib>

int main(int argc, const char *argv[]) {
	// parse input
	// write output
	return EXIT_SUCCESS;
}
```

I wrote the program in C++. More precisely in the C++20 standard. I believe
that this language is a good compromise between succinct formulation of
solutions and execution speed. For additional robustness I use some packets
from the `solid` namespace.

In contrast to C there are powerful tools in the C++ standard library.
Especially I warmly welcome the support for strings and containers. On the
other hand the generated program is fast without a lot of external dependencies.

I build the program with [CMake](https://www.cmake.org). The file
[CMakeLists.txt](./CMakeLists.txt) contains the configuration.

But there is one problem: `CMake` itself uses `md-patcher` to extract the
source code from this Markdown file. So I cannot use `CMake` to build the
first version of `md-patcher`. To solve this problem I added the small
script [bootstrap.sh](./bootstrap.sh) that directly builds `md-patcher` from
the source code files without running `md-patcher`. I can use the generated
`mdp` executable to make a full `CMake` build run.

OK, the program does not do much yet. But I can add functionality in small
steps. First I add code to enable unit-testing. So I can continue the project
in Test Driven Design (TDD): first write a test, see it fail, fix the test
and refactor. I document the results in this Markdown file.

I put all unit-tests in a function `run_tests` that I call in `md-patcher.cpp`
on every start:

```c++
// ...

static inline void run_tests() {
	// unit-tests
}

int main(int argc, const char *argv[]) {
	run_tests();
	// ...
}
```

If I pass the command line argument `--run-only-tests` I signal, that I only
want to run the unit-tsts and do not want to process any additional files:

```c++
#include <cstdlib>
#include <string>
// ...
int main(int argc, const char *argv[]) {
	run_tests();
	if (argc == 2 && argv[1] == std::string { "--run-only-tests" }) {
		return EXIT_SUCCESS;
	}
	// ...
}
```

This argument is used by `CMake` to run the test suite.

Please note the special comment `// ...`. These comments signal `md-patcher`
that it should jump over the next lines of the file until it finds a line
that matches the line following the comment. That allows me to specify the
position where the new code will be inserted. If the comment is indented it
can only jump over lines that have the same prefix.

After processing these three fragments I get the following code, that I
redirect to `/dev/null` to ignore it:

```c++
#include <cstdlib>
#include <string>

static inline void run_tests() {
	// unit-tests
}

int main(int argc, const char *argv[]) {
	run_tests();
	if (argc == 2 && argv[1] == std::string { "--run-only-tests" }) {
		return EXIT_SUCCESS;
	}
	// parse input
	// write output
	return EXIT_SUCCESS;
}
```

## The basic objects of `md-patcher`

In this section I describe the classes that I use to store the current state
of `md-patcher`. Basically that are `File`s that contain `Line`s.

I define a `Line` to be a string that also contains the name of the file from
which it was extracted and the line number in that file. These original files
are the Markdown files that are parsed by `md-patcher`. The file name and
line number are needed to sprinkle in `#line` macros in the generated sources.
If an error occurs during compilation, the compiler will refer to the markdown
file that contains the fragment in which the error occurred.

I start with a test in `md-patcher.cpp`:

```c++
// ...

#include "solid/require.h"

// ...
	// unit-tests
	{ // check Line attributes
		const Line line { "some-line", "some-file", 42 };
		require(line.value() == "some-line");
		require(line.file() == "some-file");
		require(line.number() == 42);
	}
// ...
```

I use the macro `require` from my project
[assert-problems](https://github.com/itmm/assert-problems) instead of `assert`.
`require` also works in release builds, has a more compact output and can be
caught in tests. I use it for unit-tests and also for the detection of
programming errors by checking parameters or loop invariants.

Here is my implementation of `Line`:

```c++
// ...

// ...

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
// ...
```

I would gladly declare the attributes of this class as `const`. But then I cannot use a
`std::vector` for storing them: elements can be moved around in an `insert` operation, and
you can't assign them if some attributes are `const`. So I use the more verbose variant
with non-`const` attributes and `public:` accessors.

Next I define a `File`. That is basically a vector of `Line`s plus a file path. Again I start
with a test:

```c++
// ...
	// unit-tests
	{ // check empty file
		File f { "out.cpp" };
		require(f.name == "out.cpp");
		require(f.begin() == f.end());
	}
// ...
```

`md-patcher` orders the unit-tests in the reverse direction: the second test is the first to
run. This is because the tests are inserted after the `// unit-tests` line. But it has the
nice side effect that the lastest test is run first.

But to achieve that, every test must start with a unique line. That is one reason why I start
each test with a small description of it.

This is my implementation of `File` in `md-patcher.cpp` to make the test pass:

```c++
// ...
#include <string>
#include <vector>
// ...
class Line {
	// ...
};

class File : public std::vector<Line> {
	public:
		File(const std::string &name):
			name { name }
		{ }
		const std::string name;
};
// ...
```

I collect all open files in a pool:

```c++
#include <cstdlib>
#include <map>
// ...
class File : public std::vector<Line> {
	// ...
};

static std::map<std::string, File> pool;

// ...
```

## Output

I am starting at the end of the process and look at the output. I write all lines from all
files out into the corresponding files. The first test case checks that an empty file can
be written. Instead of files I use strings to simplify the tests:

```c++
// ...
	// unit-tests
	{ // write emtpy file
		const File f { "out.c" };
		auto c { write_file_to_string(f) };
		require(c == "");
	}
// ...
```

I define the missing function `write_file_to_string` via another function that writes a `File`
into a stream:

```c++
// ...
#include <map>
#include <sstream>
// ...
class File : public std::vector<Line> {
	// ...
};

std::string write_file_to_string(const File &f) {
	std::ostringstream out;
	return write_file_to_stream(f, out).str();
}
// ...
```

The function `write_file_to_stream` uses a template for the stream type. I need only a small
subset of `std::ostream` and I will use a special stream that only modifies a file, if its
content changes. But this `Lazy_Stream` does implement only a small set of methods:

```c++
// ...
class File : public std::vector<Line> {
	// ...
};

template<typename ST>
ST &write_file_to_stream(const File &f, ST &out) {
	for (const auto &l : f) {
		out << l.value(); out.put('\n');
	}
	return out;
}
// ...
```

The next test checks that the correct content is written into non-empty files:

```c++
// ...
	// unit-tests
	{ // copy simple file
		File f { "out.c" };
		auto it = f.begin();
		it = f.insert(it, { "line 1", "-", 1 });
		it = f.insert(it, { "line 2", "-", 2 });
		auto c { write_file_to_string(f) };
		require(c == "line 1\nline 2\n");
	}
// ...
```

I need a method to insert lines and keep the iterator intact:

```c++
// ...
class File : public std::vector<Line> {
	public:
		iterator insert(iterator pos, const Line &line) {
			auto p { pos - begin() };
			std::vector<Line>::insert(pos, line);
			return begin() + (p + 1);
		}	
		// ...
};
// ...
```

The lines in a file can be non-continuous. In this case I need to insert a `#line` macro
to adjust the line counter of the compiler to the refer to the correct source line. First
I write a test for this scenario:

```c++
// ...
	// unit-tests
	{ // non-continuous file
		File f { "out.c" };
		auto it = f.begin();
		it = f.insert(it, { "line 1", "-", 1 });
		it = f.insert(it, { "line 2", "-", 10 });
		auto c { write_file_to_string(f) };
		require(c == "line 1\n#line 10\nline 2\n");
	}
// ...
```

I need to know, when I must add a `#line` macro. So I count the lines in parallel. If the next
line number is not the expected one, a `#line` macro must be inserted:

```c++
// ...
template<typename ST>
ST &write_file_to_stream(const File &f, ST &out) {
	std::string name { "-" };
	int line { 1 };
	for (const auto &l : f) {
		if (line != l.number() || name != l.file()) {
			// write line macro
			line = l.number();
			name = l.file();
		}
		out << l.value(); out.put('\n');
		++line;
	}
	return out;
}
// ...
```

While writing the macro I need a function to write a number. My stream class does not provide
this method.

```c++
// ...
			// write line macro
				out << "#line ";
				put_num(out, l.number());
				if (name != l.file()) {
					out.put(' ');
					out.put('"');
					out << l.file();
					out.put('"');
				}
				out.put('\n');
// ...
```

The helper function works recursively to write the number:

```c++
// ...
template<typename ST>
void recursive_put_num(ST &s, int num) {
	if (num) {
		recursive_put_num(s, num / 10);
		s.put((num % 10) + '0');
	}
}

template<typename ST>
void put_num(ST &s, int num) {
	if (num == 0) {
		s.put('0');
	}
	else {
		if (num < 0) {
			s.put('-');
			if (num < -9) { recursive_put_num(s, num / -10); }
			s.put('0' - (num % 10));
		} else {
			recursive_put_num(s, num);
		}
	}
}

template<typename ST>
// ...
```

Just for completeness here I wrote some tests for the `get_num` function:

```c++
// ...
	// unit-tests
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
// ...
```

I cannot insert `#line` in every file. Only can C/C++ files can contain this macro. So I can
only insert them in files whose path ends in `.h`, `.c`, or `.cpp`. In `md-patcher.cpp` I
set a flag in the constructor that reflects the ability to insert `#line` macros:

```c++
// ...
class File : public std::vector<Line> {
		static bool with_lines_(std::string name) {
			std::string ext { get_extension(name) };
			return ext == "h" || ext == "c" || ext == "cpp";
		}
	public:
		const bool with_lines;
		// ...
		File(const std::string &name):
			with_lines { with_lines_(name) },
			name { name }
		// ...
};
// ...
```

I only write `#line` macros, if the flag is set:

```c++
// ...
			// write line macro
			if (f.with_lines) {
				// ...
				out.put('\n');
			}
// ...
```

I have to add the function to extract the extension:

```c++
// ...

// ...

std::string get_extension(std::string path) {
	auto got { path.rfind('.') };
	if (got == std::string::npos) { return std::string { }; }
	return path.substr(got + 1);
}
// ...
```

I must insert a `#line` macro, if the first line of a file is not `1`:

```c++
// ...
	// unit-tests
	{ // not starting at one
		File f { "out.c" };
		auto it = f.begin();
		it = f.insert(it, { "line 1", "-", 4 });
		it = f.insert(it, { "line 2", "-", 5 });
		auto c { write_file_to_string(f) };
		require(c == "#line 4\nline 1\nline 2\n");
	}
// ...
```

Also, I must insert a `#line` macro if the input file changes:

```c++
// ...
	// unit-tests
	{ // different files
		File f { "out.c" };
		auto it = f.begin();
		it = f.insert(it, { "line 1", "-", 1 });
		it = f.insert(it, { "line 2", "other.md", 2 });
		auto c { write_file_to_string(f) };
		require(c == "line 1\n#line 2 \"other.md\"\nline 2\n");
	}
// ...
```

For the output I use the library [`lazy-write`](https://github.com/itmm/lazy-write). It only
writes to file if it changes. So if the generated file is the same one as the one already
present, the existing file will not be touched. That eases the interplay with tools like
'Make'. I use the stream class `Lazy_Write` to write the file out:

```c++
// ...

#include "lazy-write/lazy-write.h"
// ...
ST &write_file_to_stream(const File &f, ST &out) {
	// ...
}

inline void write_file(const File &f) {
	Lazy_Write out { f.name };
	write_file_to_stream(f, out);
}
// ...
```

I can now complete the output:

```c++
// ...
	// write output
	for (const auto &f: pool) {
		write_file(f.second);
	}
// ...
```

## Reading the Input

For reading the input I use the library [`line-reader`](https://github.com/itmm/line-reader).
The class `Line_Reader_Pool` contains a list of open files and keeps the paths and line
numbers for each of it:

```c++
// ...
#include "lazy-write/lazy-write.h"
#include "line-reader/line-reader.h"
// ...

static std::string line;
static Line_Reader_Pool reader;

static bool next() {
	return reader.next(line);
}

std::ostream &err_pos() {
	return std::cerr << reader.pos().file_name() <<
		':' << reader.pos().line() << ' ';
}

// ...
```

The method `next` returns the next line and `pos` returns the information about this line. In
the following tests I show how to handle it. First I have a test with one file:

```c++
// ...
	// unit-tests
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
// ...
```

I can also queue multiple files. As soon as one file ends, the content will be fetched from
the next one:

```c++
// ...
	// unit-tests
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
// ...
```

I can also include `#line` macros in the input:

```c++
// ...
	// unit-tests
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
// ...
```

So I can add the reading of the input in the `main` function of `md-patcher.cpp`. So I still
have to implement the called functions:

```c++
// ...
	// parse input
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
			if (! next()) { break; }
		}
	}
// ...
```

I read lines until I reach the end. I do not parse code blocks without a syntax specification.
That is a small trick to uncomment blocks during the debugging phase.

In the function `starts_with` in `md-patcher.cpp` I check, if a string starts with the
specified sequence of chars:

```c++
// ...

// ...

static std::string line;

static bool starts_with(
	const std::string &base,
	const std::string &prefix
) {
	if (prefix.empty()) { return true; }
	return base.size() >= prefix.size() &&
		base.substr(0, prefix.size()) == prefix;
}
// ...
```

Also I check if the file paths are found correctly. The characters between two backticks are
only a file path, if they contain at least one slash or period. The last candidate in a line
is the new file path.

First I test only one matching name:

```c++
// ...
	// unit-tests
	{ // multiple filename candidates
		line = "xx `first` xx `2nd.x` xx `` xx `last` xx";
		std::string f { "out.c" };
		change_cur_file_name(f);
		require(f == "2nd.x");
	}
// ...
```

But I also test that the last candidate is chosen:


```c++
// ...
	// unit-tests
	{ // multiple valid filename candidates
		line = "xx `first` xx `2nd.x` xx `` xx `last.x` xx";
		std::string f { "out.c" };
		change_cur_file_name(f);
		require(f == "last.x");
	}
// ...
```

Die Umsetzung geht einfach die Kandidaten durch. Der letzte Kandidat steht nach
Beenden der Funktion im Argument.

```c++
// ...

// ...

// ...
static std::string line;

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
// ...
```

## Programmcode extrahieren

Ich hoffe, Sie haben so wie ich Spaß an der schrittweisen Definition von
Programmen gefunden.

Ursprünglich hatte ich ein deutlich komplizierteres Programm `hex` geschrieben.
Aber das eignet sich nicht so gut, um Programme zu dokumentieren. Es war mit
seiner eigenen Syntax zu kompliziert. Für eine andere Programmiersprache `xtx`
habe ich nach einer einfacheren Lösung gesucht. Und `md-patcher` ist das
Ergebnis.

Die Funktion `read_patch` in `md-patcher.cpp` liest das Fragment Zeile für
Zeile, während es einen Iterator auf die bisherigen Zeilen der Datei hält:

```c++
// ...
static std::map<std::string, File> pool;

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

static inline bool read_patch(File &file) {
	if (! next()) { return false; }
	auto cur { file.begin() };
	std::string indent;
	while (line != "```") {
		// handle code
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
// ...
```

Es gibt folgende Fälle beim Code-Parsen zu unterscheiden:

```c++
// ...
static inline bool read_patch(File &file) {
	// ...
	while (line != "```") {
		// handle code
		if (line_is_wildcard(indent)) {
			// do wildcard
			continue;
		} else if (cur != file.end() && line == cur->value()) {
			++cur;
		} else {
			// insert line
		}
		// ...
	}
	// ...
}
// ...
```

Beim Einfügen einer Zeile muss darauf geachtet werden, dass danach der Iterator
nicht mehr gültig sein muss. Es muss also über den Index gegangen werden:

```c++
// ...
static inline bool read_patch(File &file) {
	// ...
	while (line != "```") {
		// ...
			// insert line
			cur = insert_before(line, cur, file);
		// ...
	}
	// ...
}
// ...
```

Bei der Wildcard-Erkennung wird ein `ident` ermittelt, welcher der Funktion in
`md-patcher.cpp` mitgegeben wird, um ein vorzeitiges Ende des Kopierens zu
erkennen:

```c++
// ...
static inline bool read_patch(File &file) {
	// ...
	while (line != "```") {
		// ...
			// do wildcard
			if (! do_wildcard(indent, file, cur)) {
				return false;
			}
		// ...
	}
	// ...
}
// ...
```

Ob der Füll-Kommentar vorhanden ist, ermittelt die folgende Funktion in
`md-patcher.cpp`:

```c++
// ...
static std::string line;

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
// ...
```

Damit kann zu guter Letzt die Füll-Funktion angegeben werden:

```c++
// ...
// patch helpers

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
// ...
```

## Includes verarbeiten

Wenn Markdown-Dateien aus der aktuellen Datei heraus verlinkt werden, dann
sollen diese auch mit prozessiert werden. Dadurch müssen nicht bei neuen Dateien
ständig die Makefiles angepasst werden. Dafür soll es eine Hilfsfunktion geben,
die Links aus einer Zeile extrahiert:

```c++
// ...
	// unit-tests
	{ // find file name in line
		std::string l { "a line with [bla](bla.md) a link" };
		std::string got { link_in_line(l) };
		require(got == "bla.md");
	}
// ...
```

Aber diese Funktion gibt es noch nicht. Hier ist die Implementierung:

```c++
// ...

// ...

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
// ...
```

Nun muss nur noch geprüft werden, ob ein passender Link existiert. Falls ja,
wird die Datei ebenfalls bearbeitet.

```c++
// ...
			change_cur_file_name(cur_file);
			auto sub { link_in_line(line) };
			if (
				sub.size() > 3 &&
				sub.rfind(".md") == sub.size() - 3
			) {
				// normalize path
				reader.push_front(sub);
			}
// ...
```

Für die Normalisierung wird der Pfad erst einmal in seine Komponenten zerlegt:

```c++
// ...
class File : public std::vector<Line> {
// ...
};

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

// ...
				// normalize path
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

// ...
```

Als weitere Optimierung sollen keine Teile ausgegeben werden, die mit `#if 0`
auskommentiert sind. Mit dem Kommandozeilen-Argument `--raw` kann ich diese
Optimierung jedoch unterbinden:

```c++
// ...

// ...

bool write_raw { false };

// ...
ST &write_file_to_stream(const File &f, ST &out) {
	bool skipping { false };
	std::string end_line { };
	// ...
	for (const auto &l : f) {
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
		// ...
	}
	require(! skipping);
	// ...
}
// ...
	run_tests();
	if (argc >= 2 && argv[1] == std::string { "--raw" }) {
		write_raw = true; --argc; ++argv;
	}
// ...
```

Mit einem speziellen Kommentar im Markdown kann ich die Generierung vorzeitig
abbrechen. Das ist beim Debuggen hilfreich:

```c++
// ...
			change_cur_file_name(cur_file);
			if (line == "<!-- MD-PATCHER EXIT -->") { break; }
// ...
```

Damit ist der gesamte Quellcode beschrieben.

Und aus dieser Markdown-Datei wurde mit `md-patcher` das Programm selbst
extrahiert.
