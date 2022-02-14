# md-patcher

Ein Programm, um aus Code-Fragmenten einer Markdown-Datei
kompilierbare Programme zu extrahieren.

## Genereller Aufbau

`md-patcher` ist sehr einfach gehalten.
Es liest Markdown-Dateien und probiert,
darin enthaltene Code-Fragmente zu Dateien zusammenzusetzen.
Diese werden dann herausgeschrieben.
Den Datei-Namen nimmt `md-patcher` aus den normalen
Text-Blöcken:
Jedes Inline-Codefragment
(das mindestens einen Slash oder Punkt enthält)
ändert den Namen der aktuellen Datei.
Bevor ein relevantes Code-Fragment kommt,
sollte die Datei also zum Beispiel mit `md-patcher.cpp`
auf einen sinnvollen Namen gesetzt werden.
Achtung: Das Programm prüft nicht, ob der Name sinnvoll ist.
Zur Sicherheit sollte immer direkt vor einem Code-Block
der Name der Datei angegeben werden.
Dies erleichtert zusätzlich dem Leser,
den Zusammenhang der Code-Blöcke im Kopf zu behalten.

Die Datei muss nicht auf einen Schlag angegeben werden.
Es können Fragmente angegeben werden,
die im Laufe des Dokumentes erweitert werden können.
Ein Anfang von `md-patcher.cpp` könnte zum Beispiel so
aussehen:

```c++
int main(int argc, const char *argv[]) {
	// parse input
	// write output
	return 0;
}
```

Das Programm ist in C++ geschrieben.
Genauer: dem C++17 Standard.
Ich denke, diese Sprache erlaubt eine vernünftige Balance
zwischen Kompaktheit und Ausführungsgeschwindigkeit.
Im Gegensatz zu C gibt es schon mächtige Werkzeuge für den
Umgang mit Containern in der Standard-Bibliothek.
Und trotzdem ist das erzeugte Programm schnell und hat wenig externe
Abhängigkeiten..

Das gerade angegebene Fragment definiert nur eine Funktion `main`.
Dies ist die zentrale Funktion,
die aufgerufen wird,
wenn wir das Programm starten.
Die Zeichen `//` leiten einen Kommentar ein.
Alles was hinter den beiden Zeichen steht,
wird vom Compiler ignoriert.

Das Programm kann mit `cmake` gebaut werden.
Die entsprechende Konfigurationsdatei liegt in
[CMakeLists.txt](./CMakeLists.txt).

Ein Problem existiert mit `cmake` leider:
Es verwendet selbst `md-patcher`, um die Sourcen aus diesem Dokument
zu extrahieren.
Die generierten Sourcen sind zwar ebenfalls Teil dieses Repositories,
aber ohne `md-patcher` kann `cmake` die Sourcen nicht bauen.
Daher gibt es noch ein kleines Skript
[bootstrap.sh](./bootstrap.sh),
das aus den bestehenden Sourcen direkt `md-patcher` baut,
ohne irgendwelche Abhängigkeiten zu prüfen.

Zugegeben, das Programm macht noch nicht sehr viel.
Aber wir können es jetzt Stück für Stück erweitern.
Zuerst wird erst einmal Code integriert, um Unit-Tests auszuführen.
Damit haben wir die Möglichkeit, das Programm nach TDD zu entwickeln.
Dazu gibt es eine Funktion `run_tests` in `md-patcher.cpp`,
die bei jedem Start ausgeführt wird:

```c++
#include <string>
static inline void run_tests() {
	// unit-tests
}
int main(int argc, const char *argv[]) {
	run_tests();
	if (argc == 2 && argv[1] == std::string { "--run-only-tests" }) {
		return 0;
	}
	// ...
}
```

Wenn nur die Unit-Tests ausgeführt werden sollen, so kann das mit dem
Kommandozeilen-Argument `--run-only-tests` angegeben werden.

Wichtig sind die Zeilen mit dem Füll-Kommentar `// ...`.
Genau diese Zeilen erkennt `md-patcher` und fügt das bisher
bestehende Programm ein.

`md-patcher` probiert,
das bestehende Programm mit dem Fragment zusammenzuführen.
Dabei nutzt es zum einen identische Zeilen als
Verankerung der beiden Teile miteinander.
Zum anderen wird mit den Füll-Kommentaren angezeigt,
an welcher Stelle im Fragment der bestehende Code
eingesetzt werden kann.

Der resultierende Code
(dessen Ausgabe jetzt nach `/dev/null` geschrieben wird),
ist also:

```c++
static inline void run_tests() {
	// unit-tests
}
int main(int argc, const char *argv[]) {
	run_tests();
	if (argc == 2 && argv[1] == std::string { "--run-only-tests" }) {
		return 0;
	}
	// parse input
	// write output
	return 0;
}
```

Wenn Füll-Kommentare mit Tabs eingerückt sind,
dann können sie nur durch Code ersetzt werden,
der ebenfalls mindestens so tief eingerückt ist.

Als erste Struktur definieren wir eine Zeile.
Sie enthält nicht nur die gelesenen Zeichen, sondern
zusätzlich den Namen der Quell-Datei und die Zeile in der Quell-Datei.
Diese Informationen werden später benötigt, um die die richtigen `#line`
Anweisungen zu generieren.
Definieren wir zuerst einen Test in `md-patcher.cpp`:

```c++
#include <cassert>
// ...
	// unit-tests
	{ // check Line attributes
		const Line line { "some-line", "some-file", 42 };
		assert(line.value() == "some-line");
		assert(line.file() == "some-file");
		assert(line.number() == 42);
	}
// ...
```

Hier die entsprechende Struktur:

```c++
// ...
#include <string>
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

Die Attribute sind nicht `const`, da `Line`-Instanzen einander zugewiesen
werden können.

Eine Ausgabe-Datei hat selber auch einen Namen und beliebig viele Zeilen:

```c++
// ...
	// unit-tests
	{ // check empty file
		File f { "out.cpp" };
		assert(f.name() == "out.cpp");
		assert(f.begin() == f.end());
	}
// ...
```

Hier ist einfache Implementierung der Klasse, um den Unit-Test zum
Laufen zu bekommen:

```c++
// ...
class Line {
	// ...
};
#include <vector>
class File {
		const std::string name_;
		using Lines = std::vector<Line>;
		Lines lines_;
	public:
		File(const std::string &name): name_ { name } { }
		const std::string &name() const { return name_; }
		auto begin() { return lines_.begin(); }
		auto begin() const { return lines_.begin(); }
		auto end() { return lines_.end(); }
		auto end() const { return lines_.end(); }
};
// ...
```

Damit können wir einen Pool an Dateien offen halten,
deren Inhalt generiert wird:

```c++
// ...
class File {
	// ...
};
#include <map>

static std::map<std::string, File> pool;
// ...
```

## Ausgabe

Zäumen wir das Pferd von hinten auf und betrachten zuerst
die Ausgabe.
Es werden einfach alle Zeilen in die entsprechenden
Dateien geschrieben.
Bisher gibt es zwar noch keine Zeilen.
Dem entsprechend ist das Ergebnis leer.

```c++
// ...
	// unit-tests
	{ // write emtpy file
		const File f { "out.txt" };
		auto c { write_file_to_string(f) };
		assert(c == "");
	}
// ...
```

Momentan gibt es noch gar nicht die Methode, die ein `File` in einen
`std::string` schreibt.
Daher wird diese erst einmal definiert:

```c++
// ...
class File {
	// ...
};
#include <sstream>
std::string write_file_to_string(const File &f) {
	std::ostringstream out;
	return write_file_to_stream(f, out).str();
}
// ...
```

Aber auch die verwendete Methode gibt es noch nicht.
Diese wird als `template` implementiert,
da nicht jeder Stream einen vollen `std::ostream` implementiert:

```c++
// ...
class File {
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

Die Parameter werden als `template` implementiert.
Später verwenden wir anstatt Streams einen `Lazy_Writer`,
der Dateien nur schreibt, wenn sie sich auch verändert haben.
Dadurch bleiben die Änderungszeitstempel von Dateien erhalten, die
sich nicht verändert haben.

Der nächste Test haucht den Dateien Inhalt ein:

```c++
// ...
	// unit-tests
	{ // copy simple file
		File f { "out.txt" };
		auto it = f.begin();
		it = f.insert(it, { "line 1", "out.txt", 1 });
		it = f.insert(it, { "line 2", "out.txt", 2 });
		auto c { write_file_to_string(f) };
		assert(c == "line 1\nline 2\n");
	}
// ...
```

Dazu muss aber zuerst die Methode zum Einfügen von Zeilen implementiert
werden:

```c++
// ...
class File {
	// ...
	public:
		// ...
		auto begin() { return lines_.begin(); }
		using iterator = Lines::iterator;
		iterator insert(iterator pos, const Line &line) {
			auto p { pos - begin() };
			lines_.insert(pos, line);
			return begin() + (p + 1);
		}	
		// ...
};
// ...
```

Da normale Iteratoren beim `insert` ihre Gültigkeit verlieren können,
wird die Index-Position gesichert und daraus nach dem `insert` wieder
ein gültiger Iterator erzeugt.

Interessant wird es, wenn die Zeilen in der Ursprungs-Datei nicht fortlaufend
sortiert waren.
In diesem Fall muss ein spezielles `#line` Makro generiert werden:


```c++
// ...
	// unit-tests
	{ // non-continuous file
		File f { "out.txt" };
		auto it = f.begin();
		it = f.insert(it, { "line 1", "out.txt", 1 });
		it = f.insert(it, { "line 2", "out.txt", 10 });
		auto c { write_file_to_string(f) };
		assert(c == "line 1\n#line 10\nline 2\n");
	}
// ...
```

Beim Schreiben muss daher die aktuelle Datei und Zeilen-Nummer mit
gespeichert und passend aktualisiert werden:

```c++
// ...
template<typename ST>
ST &write_file_to_stream(const File &f, ST &out) {
	auto name { f.name() };
	int line { 1 };
	for (const auto &l : f) {
		if (line != l.number() || name != l.file()) {
			// write line macro
		}
		out << l.value(); out.put('\n');
		++line;
	}
	return out;
}
// ...
```

Bei der Ausgabe wird eine Hilfsmethode benötigt, um positive Zahlen
auszugeben:

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
			line = l.number();
			name = l.file();
// ...
```

Diese Funktion muss auch noch definiert werden:


```c++
// ...
template<typename ST>
void put_num(ST &s, int num) {
	if (num) {
		put_num(s, num / 10);
		s.put((num % 10) + '0');
	}
}
template<typename ST>
// ...
```

Wir können auch testen, was passiert wenn die gleich die erste Zeile falsch
ist:


```c++
// ...
	// unit-tests
	{ // not starting at one
		File f { "out.txt" };
		auto it = f.begin();
		it = f.insert(it, { "line 1", "out.txt", 4 });
		it = f.insert(it, { "line 2", "out.txt", 5 });
		auto c { write_file_to_string(f) };
		assert(c == "#line 4\nline 1\nline 2\n");
	}
// ...
```

Oder wenn sich die Datei ändert:


```c++
// ...
	// unit-tests
	{ // different files
		File f { "out.txt" };
		auto it = f.begin();
		it = f.insert(it, { "line 1", "out.txt", 1 });
		it = f.insert(it, { "line 2", "other.txt", 2 });
		auto c { write_file_to_string(f) };
		assert(c == "line 1\n#line 2 \"other.txt\"\nline 2\n");
	}
// ...
```

Bei der eigentlichen Ausgabe wird statt dessen die `lazy-write` Bibliothek
benutzt.
Damit werden die Dateien nur dann neu geschrieben, wenn sie sich auch wirklich
verändern.

```c++
// ...
ST &write_file_to_stream(const File &f, ST &out) {
	// ...
}
#include "lazy-write.h"

inline void write_file(const File &f) {
	Lazy_Write out { f.name() };
	write_file_to_stream(f, out);
}
// ...
```

Diese Bibliothek ist auch der Grund dafür, dass Zahlen nicht direkt ausgegeben
werden können. `lazy-write` unterstützt nur die Ausgabe von Zeichen und
`string`s.

Nun kann die Ausgabe abgeschlossen werden:

```c++
// ...
	// write output
	for (const auto &f: pool) {
		write_file(f.second);
	}
// ...
```

## Eingabe lesen

Das Lesen der Eingabe übernimmt eine weitere Bibliothek: `line-reader`.
Die Klasse `Line_Reader_Pool` enthält eine ganze Liste von offenen Dateien,
die nach einander abgearbeitet werden.

```c++
// ...
#include <string>
#include <iostream>
#include "line-reader.h"

static std::string line;
static Line_Reader_Pool reader;

static bool next() {
	return reader.next(line);
}

std::ostream &err_pos() {
	return std::cerr << reader.pos().file_name() <<
		':' << reader.pos().line() << ' ';
}

// next defined
// ...
```

Der zusätzliche Kommentar hilft uns später Funktionen
zu definieren,
die `next` aufrufen.
Diese müssen nach der Funktion `next` definiert werden,
oder es gibt einen Fehler bei der Übersetzung.

Die Methode `next` gibt die nächste Zeile zurück und mit
`pos` kann der Dateiname und die Zeilennummer der nächsten Zeile ermittelt
werden.

```c++
// ...
	// unit-tests
	{ // reading lines
		Line_Reader_Pool pool;
		std::istringstream in { "abc\ndef\n" };
		pool.push_back("x", in);
		std::string line;
		assert(pool.next(line));
		assert(line == "abc");
		assert(pool.pos().file_name() == "x");
		assert(pool.pos().line() == 1);
		assert(pool.next(line));
		assert(line == "def");
		assert(pool.pos().file_name() == "x");
		assert(pool.pos().line() == 2);
		assert(! pool.next(line));
	}
// ...
```

Es können auch mehrere Dateien auf einmal gelesen werden:


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
		assert(pool.next(line));
		assert(line == "abc");
		assert(pool.pos().file_name() == "x");
		assert(pool.pos().line() == 1);
		assert(pool.next(line));
		assert(line == "def");
		assert(pool.pos().file_name() == "y");
		assert(pool.pos().line() == 1);
		assert(! pool.next(line));
	}
// ...
```

`#line` Makros in der Eingabe werden richtig verarbeitet:

```c++
// ...
	// unit-tests
	{ // reading lines with line macro
		Line_Reader_Pool pool;
		std::istringstream in { "abc\n#line 3 \"z\"\ndef" };
		pool.push_back("x", in);
		std::string line;
		assert(pool.next(line));
		assert(line == "abc");
		assert(pool.pos().file_name() == "x");
		assert(pool.pos().line() == 1);
		assert(pool.next(line));
		assert(line == "def");
		assert(pool.pos().file_name() == "z");
		assert(pool.pos().line() == 3);
		assert(! pool.next(line));
	}
// ...
```

Damit kann das Lesen in der `main` Funktion in
`md-patcher.cpp` beschrieben werden
(auch wenn es die aufgerufenen Funktionen noch nicht gibt):

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

Es werden so lange Zeilen gelesen, bis das Ende erreicht ist.
Code-Blöcke ohne Syntax-Angabe werden nicht geparst.

Die Funktion `starts_with` in `md-patcher.cpp` prüft einfach,
ob ein String mit einer bestimmten Sequenz beginnt:

```c++
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

Hier sieht man wieder, wie eine bestehende Zeile
herangezogen wird,
um einen geeigneten Einfügepunkt der Funktion zu finden.

Ebenfalls recht einfach kann ein neuer
Dateiname ermittelt werden.
Es wird in der aktuellen Zeile geprüft,
ob zwei Backticks vorkommen.
Der String zwischen den letzten zwei Backticks ist dann
der aktuelle Dateiname:

```c++
// ...
static std::string line;

static void change_cur_file_name(std::string &file) {
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
// ...
```

## Programmcode extrahieren

Ich hoffe,
Sie haben so wie ich Spaß an der schrittweisen Definition
von Programmen gefunden.

Ursprünglich hatte ich ein deutlich komplizierteres
Programm `hex` geschrieben.
Aber das eignet sich nicht so gut,
um Programme zu dokumentieren.
Es war mit seiner eigenen Syntax zu kompliziert.
Für eine andere Programmiersprache `xtx` habe ich nach
einer einfacheren Lösung gesucht.
Und `md-patcher` ist das Ergebnis.

Die Funktion `read_patch` in `md-patcher.cpp` liest das
Fragment Zeile für Zeile,
während es einen Iterator auf die bisherigen Zeilen der
Datei hält:

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

Beim Einfügen einer Zeile muss darauf geachtet werden,
dass danach der Iterator nicht mehr gültig sein muss.
Es muss also über den Index gegangen werden:

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

Bei der Wildcard-Erkennung wird ein `ident` ermittelt,
welcher der Funktion in `md-patcher.cpp` mitgegeben wird,
um ein vorzeitiges Ende des Kopierens zu erkennen:

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

Ob der Füll-Kommentar vorhanden ist, ermittelt die
folgende Funktion in `md-patcher.cpp`:

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

Damit kann zu guter Letzt die Füll-Funktion angegeben
werden:

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

Wenn Markdown-Dateien aus der aktuellen Datei heraus verlinkt werden,
dann sollen diese auch mit prozessiert werden.
Dadurch müssen nicht bei neuen Dateien ständig die Makefiles angepasst
werden.
Dafür soll es eine Hilfs-Funktion geben, die Links aus einer Zeile
extrahiert:


```c++
// ...
	// unit-tests
	{ // find file name in line
		std::string l { "a line with [bla](bla.md) a link" };
		std::string got { link_in_line(l) };
		assert(got == "bla.md");
	}
// ...
```

Aber diese Funktion gibt es noch nicht.
Hier ist die Implementierung:

```c++
// ...
#include <string>

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

Nun muss nur noch geprüft werden, ob ein passender Link existiert.
Falls ja, wird die Datei ebenfalls bearbeitet.

```c++
// ...
			change_cur_file_name(cur_file);
			auto sub { link_in_line(line) };
			if (sub.size() > 3 && sub.rfind(".md") == sub.size() - 3) {
				// normalize path
				reader.push_front(sub);
			}
// ...
```

Für die Normalisierung wird der Pfad erst einmal in seine Komponenten
zerlegt:

```c++
// ...
#include <sstream>

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
					if (!parts.empty()) { parts.pop_back(); }
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

Als weitere Optimierung sollen keine Teile ausgegeben werden, die mit
`#if 0` auskommentiert sind:

```c++
// ...
ST &write_file_to_stream(const File &f, ST &out) {
	bool skipping { false };
	std::string if_prefix { };
	// ...
	for (const auto &l : f) {
		if (skipping) {
			if (l.value() == if_prefix + "#endif") {
				skipping = false;
				continue;
			}
			continue;
		}
		auto idx { l.value().find("#if 0") };
		if (idx != std::string::npos) {
			skipping = true;
			if_prefix = l.value().substr(0, idx);
			for (char ch : if_prefix) {
				if (ch > ' ') { skipping = false; break; }
			}
			if (skipping) { continue; }
		}
		// ...
	}
	if (skipping) {
		std::cerr << "open #if 0\n";
	}
	// ...
}
// ...
```

Damit ist der gesamte Quellcode beschrieben.

Und aus dieser Markdown-Datei wurde mit `md-patcher`
das Programm selbst extrahiert.
