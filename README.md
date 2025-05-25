<!-- vim: set spelllang=en noexpandtab: -->
# md-patcher

`md-patcher` is a program, to extract code fragments out of Markdown files and
combine them to full source code files.  It is a successor of
[hex](https://github.com/itmm/hex) with a far clearer syntax, but also with
slightly smaller expressive power.

The main feature is, that your source code can be described in far greater
detail in the Markdown files. Also it allows to develop the program and not
present only the final stage.


## General Structure

I use an easy structure for `md-patcher`: I read Markdown files and combines
the included code fragments to whole source code files. Then I write the
resulting files out. I use normal inline code fragments to specify the file
names of the generated files: each inline code fragment that contains at least
a period or slash changes the current file name.

Sometimes I am unsure, if `md-patcher` and I agree about the current file name.
So I add the file name as an inline code fragment directly before the code
fragment. As a benefit, it is
also easier for the reader to see to with file the fragment belongs.

**Caution**: `md-patcher` does not check, if the file name makes sense. Also
in `md-patcher` I will not create any directories that are missing in the path.

By the way: this Markdown file can be used to extract the full source code of
`md-patcher`. But for that I need a running version of `md-patcher`. So the
generated source code is also part of the repository to ease the bootstrap
process.

I do not need to create the files in one go. I can modify and extend fragments
while going through this document. So I can start with a birds eye view
and drill down later. At every step I hopefully have a buildable program.

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
other hand the generated program is fast without a lot of external
dependencies.

I build the program with [GNU Make](https://www.gnu.org/software/make/). The
file [Makefile](./Makefile) contains the configuration.

OK, the program does not do much yet. But I can add functionality in small
steps. First I add code to enable unit-testing. So I can continue the project
in Test Driven Design (TDD): first write a test, see it fail, fix the test and
refactor (or: remove duplication). I document the results in this Markdown
file.

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
// ...
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

This argument is used by `GNU Make` to run the test suite.

Please note the special comment `// ...`. These comments signal `md-patcher`
that it should jump over the next lines of the file until it finds a line that
matches the line following the comment. That allows me to specify the position
where the new code will be inserted.

If the comment is indented it can only jump over lines that have the same
prefix.

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

You see that it is a well structured C++ file and a great starting point for
the program. I actually lied a bit and removed a lot of `#line` macros. These
are inserted, so the compiler can refer to the position in the Markdown file
instead of the generated source code. So you can stay in the Markdown file and
fix warnings and errors directly there.


## The basic objects of `md-patcher`

In this section I describe the classes that I use to store the current state of
`md-patcher`. Basically that are `File`s that contain `Line`s.

I define a `Line` to be a string that also contains the name of the file from
which it was extracted and the line number in that file. These original files
are the Markdown files that are parsed by `md-patcher`. The file name and line
number are needed to sprinkle in `#line` macros in the generated sources.

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
[solid-require](https://github.com/itmm/solid-require) instead of `assert`.
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
		Line(const std::string &value, const std::string &file, int number):
			value_ { value }, file_ { file }, number_ { number }
		{ }
		const std::string &value() const { return value_; }
		const std::string &file() const { return file_; }
		int number() const { return number_; }
};
// ...
```

I would gladly declare the attributes of this class as `const`. But then I
cannot use a `std::vector` for storing them: elements can be moved around in an
`insert` operation, and you can't assign them if some attributes are `const`.
So I use the more verbose variant with non-`const` attributes and `public:`
accessors. But apart from the automatically generated `operator=` no method
will change any attributes.

Next I define a `File`. That is basically a vector of `Line`s plus a file path.
Again I start with a test:

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

`md-patcher` orders the unit-tests in the reverse direction: the second test is
the first to run. This is because the tests are inserted after the
`// unit-tests` line. But it has the nice side effect that the lastest test is
run first.

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

I am starting at the end of the process and look at the output. I write all
lines from all files out into the corresponding files. The first test case
checks that an empty file can be written. Instead of files I use strings to
simplify and speed up the tests:

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

I define the missing function `write_file_to_string` via another function that
writes a `File` into a stream:

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
	write_file_to_stream(f, out);
	return out.str();
}
// ...
```

The function `write_file_to_stream` uses a `std::ostream` to write a `File`:

```c++
// ...
class File : public std::vector<Line> {
	// ...
};

std::ostream& write_file_to_stream(const File &f, std::ostream& out) {
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
		it = ++f.insert(it, { "line 1", "-", 1 });
		it = ++f.insert(it, { "line 2", "-", 2 });
		auto c { write_file_to_string(f) };
		require(c == "line 1\nline 2\n");
	}
// ...
```

The lines in a file can be non-continuous. In this case I need to insert a
`#line` macro to adjust the line counter of the compiler to the refer to the
correct source line. First I write a test for this scenario:

```c++
// ...
	// unit-tests
	{ // non-continuous file
		File f { "out.c" };
		auto it = f.begin();
		it = ++f.insert(it, { "line 1", "-", 1 });
		it = ++f.insert(it, { "line 2", "-", 10 });
		auto c { write_file_to_string(f) };
		require(c == "line 1\n#line 10\nline 2\n");
	}
// ...
```

I need to know, when I must add a `#line` macro. So I count the lines in
parallel. If the next line number is not the expected one, a `#line` macro must
be inserted:

```c++
// ...
#include <map>
#include <ostream>
// ...
std::ostream& write_file_to_stream(const File &f, std::ostream& out) {
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

Here I write the macro:

```c++
// ...
			// write line macro
				out << "#line " << l.number();
				if (name != l.file()) { out << " \"" << l.file() << "\""; }
				out.put('\n');
// ...
```

I cannot insert `#line` in every file. Only can C/C++ files can contain this
macro. So I can only insert them in files whose path ends in `.h`, `.c`, or
`.cpp`. In `md-patcher.cpp` I set a flag in the constructor that reflects the
ability to insert `#line` macros. First a test:

```c++
// ...
	// unit-tests
	{ // check that file extension is honoured
		File fc { "out.c" };
		require(fc.with_lines);
		File fh { "out.h" };
		require(fh.with_lines);
		File fcpp { "out.cpp" };
		require(fcpp.with_lines);
		File no { "out_c" };
		require(! no.with_lines);
		File fjava { "out.java" };
		require(! fjava.with_lines);
		File fm { "Makefile" };
		require(! fm.with_lines);
	}
// ...
```

And here is the implementation to make the test pass:

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
		it = ++f.insert(it, { "line 1", "-", 4 });
		it = ++f.insert(it, { "line 2", "-", 5 });
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
		it = ++f.insert(it, { "line 1", "-", 1 });
		it = ++f.insert(it, { "line 2", "other.md", 2 });
		auto c { write_file_to_string(f) };
		require(c == "line 1\n#line 2 \"other.md\"\nline 2\n");
	}
// ...
```

For the output I use the library
[`lazy-write`](https://github.com/itmm/lazy-write). It only writes to file if
it changes. So if the generated file is the same one as the one already
present, the existing file will not be touched. That eases the interplay with
tools like 'GNU Make'. I use the stream class `Lazy_Write` to write the file
out:

```c++
// ...

#include "lazy-write/lazy-write.h"
// ...
std::ostream& write_file_to_stream(const File &f, std::ostream& out) {
	// ...
}

inline void write_file(const File &f) {
	Lazy_Write out { f.name };
	write_file_to_stream(f, out);
}
// ...
```

Note that `Lazy_Write` is a subtype of `std::ostream`.

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

For reading the input I use the library
[`line-reader`](https://github.com/itmm/line-reader).  The class
`Line_Reader_Pool` contains a list of open files and keeps the paths and line
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

void err(const std::string& message) {
	std::cerr << reader.pos().file_name() << ':' << reader.pos().line() <<
		' ' << message << '\n';
	require(false && "error occurred");
}

// ...
```

The method `next` returns the next line and `pos` returns the information
about this line. In the following tests I show how to handle it. First I have a 
test with one file:

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

I can also queue multiple files. As soon as one file ends, the content will be
fetched from the next one:

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

So I can add the reading of the input in the `main` function of
`md-patcher.cpp`. I still have to implement the called functions `starts_with`,
`read_patch`, and `change_cur_file_name`:

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
			cur_file = change_cur_file_name(cur_file);
			if (! next()) { break; }
		}
	}
// ...
```

I read lines until I reach the end. I do not parse code blocks without a
syntax specification.  That is a small trick to uncomment blocks during the
debugging phase.

In the function `starts_with` I check, if a string starts with the specified
sequence of chars:

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

Also I check if the file paths are found correctly. The characters between two
backticks are only a file path, if they contain at least one slash or period.
The last candidate in a line is the new file path.

First I test only one matching name:

```c++
// ...
	// unit-tests
	{ // no file name change in empty line
		line = "";
		std::string f { change_cur_file_name("out.c") };
		require(f == "out.c");
	}
	{ // no file name change in simple line
		line = "abc x.cpp";
		std::string f { change_cur_file_name("out.c") };
		require(f == "out.c");
	}
	{ // no file name change in non-path snippets
		line = "a `Makefile` b";
		std::string f { change_cur_file_name("out.c") };
		require(f == "out.c");
	}
	{ // multiple filename candidates
		line = "xx `first` xx `2nd.x` xx `` xx `last` xx";
		std::string f { change_cur_file_name("out.c") };
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
		std::string f { change_cur_file_name("out.c") };
		require(f == "last.x");
	}
// ...
```

In the function `change_cur_file_name` I walk through all candidates and keep
the last one as the final result:

```c++
// ...

// ...

// ...
static std::string line;

static std::string change_cur_file_name(const std::string &file) {
	std::string result { file };
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
		) { result = got; }
		start = end + 1;
	}
	return result;
}
// ...
```


## Extract Program Code

I hope you enjoy the piecewise definition of the program.

Before I wrote [`hex`](https://github.com/itmm/hex). It also allows the
extraction and concatenation of code fragments. But it uses are more complex
syntax. `md-patcher` is far easier by using the `// ...` comment.

In the function `read-patch` in `md-patcher.cpp` I read a code fragment line by
line. During that time I have a matching iterator to the previously parsed
lines of the current file.

But first add a test:

```c++
// ...
	// unit-tests
	{ // parse simple code block
		std::istringstream in { "```c\nint a = 2;\nreturn a;\n```\n" };
		reader.push_front("x.md", in);
		next();
		require(line == "```c");
		File file { "out.c" };
		require(! read_patch(file));
		require(file.size() == 2);
		require(file[0].value() == "int a = 2;");
		require(file[1].value() == "return a;");
		reader = Line_Reader_Pool { };
	}
// ...
```

Here is the implementation:

```c++
// ...
static std::map<std::string, File> pool;

// patch helpers

static inline bool read_patch(File &file) {
	if (! next()) { return false; }
	auto cur { file.begin() };
	std::string indent;
	while (line != "```") {
		// handle code
		if (! next()) { err("end of file in code block"); }
	}
	if (cur != file.end()) { err("incomplete patch"); }
	return next();
}

// ...
```

There are two cases to differentiate while parsing code: processing a
wildcard line, and inserting a new line into the code file:

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
			// lines match
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

Due to my special `insert` implememntation in `File`, the returned iterator is valid after
the insertion:

```c++
// ...
static inline bool read_patch(File &file) {
	// ...
	while (line != "```") {
		// ...
			// insert line
			cur = ++file.insert(
				cur, Line {
					line, reader.pos().file_name(), reader.pos().line()
				}
			);
		// ...
	}
	// ...
}
// ...
```

When I recognize a wildcard comment, I store the prefix before the comment in the variable
`indent`. That variable I pass to the `do_wildcard` function to recognize premature
termination of the wildcard skipping:

```c++
// ...
static inline bool read_patch(File &file) {
	// ...
	while (line != "```") {
		// ...
			// do wildcard
			bool is_super { line.find("//" " ....") != std::string::npos };
			if (! do_wildcard(indent, file, cur, is_super)) {
				return false;
			}
		// ...
	}
	// ...
}
// ...
```

In the following function I recognize, if a wildcard comment is present:

```c++
// ...
static std::string line;

static inline bool line_is_wildcard(
	std::string &indent
) {
	auto idx = line.find("//" " ...");
	if (idx == std::string::npos) { return false; }
	indent = line.substr(0, idx);
	return true;
}
// ...
```

```c++
// ...
	// unit-tests
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
	IT &cur,
	bool is_super
) {
	if (! next()) {
		err("end of file after wildcard\n");
		return false;
	}
	while (cur != file.end()) { 
		if (! starts_with(cur->value(), indent)) { break; }
		if (line != "```" && cur->value() == line && ! is_super) {
			++cur;
			next();
			break;
		}
		++cur;
	}
	return true;
}
// ...
```

## Handle Includes

If I link from one Markdown file to another Markdown file, I also want to process the referred
file. That simplifies the call of `md-patcher`. I will write a function, that extracts a link
from the Markdown line:

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

Only one link can be extracted! So you have to split a line with multiple links.

I implement the function in the following way:

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

I have to check that the link points to a Markdown file. If so, I redirect the input to
continue from this file:

```c++
// ...
			cur_file = change_cur_file_name(cur_file);
			auto sub { link_in_line(line) };
			if (sub.size() > 3 && sub.rfind(".md") == sub.size() - 3) {
				// normalize path
				reader.push_front(sub);
			}
// ...
```

### Normalizing Paths

To normalize a file path, I split it into its components first. Then I ignore `.` components
and remove a component for each `..` that occurs. This is the change in `md-patcher.cpp`:

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
```

If I run into a relative path, I take the path of the current file and add the relative
components:

```c++
// ...
				// normalize path
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

// ...
```

## Writing `--raw` Output

I normally don't write any parts that are commented out with `#if 0`. But I sometimes need
these blocks during debugging. So I add an command line argument `--raw`. If this is present,
I keep the blocks in the output:

```c++
// ...

// ...

bool write_raw { false };

// ...
std::ostream& write_file_to_stream(const File &f, std::ostream& out) {
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
	if (skipping) { err("no #endif for #if"); }
	// ...
}
// ...
	run_tests();
	if (argc >= 2 && argv[1] == std::string { "--raw" }) {
		write_raw = true; --argc; ++argv;
	}
// ...
```

## Early Exit Comment

Also I add a special comment, that stops the processing of a file. With that comment I can
find errors during debugging of the Markdown document:

```c++
// ...
			cur_file = change_cur_file_name(cur_file);
			if (line == "<!-- MD-PATCHER EXIT -->") { break; }
// ...
```

That is the complete source code. You can use this Markdown file to extract the source code
with `md-patcher` and build `md-patcher` from scratch. Have fun!

## Don't write to `/dev/null`

I add one change to not keep any lines that are written to `/dev/null`. That way, multiple
writes to that file do not need to add comments to skip over existing lines.

The change in `md-patcher.cpp` is minute: I just remove the file after processing it:

```c++
// ...
			if (! read_patch(f->second)) { break; }
			if (cur_file == "/dev/null") { pool.erase(f); }
// ...
```
