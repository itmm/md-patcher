# md-patcher

Ein Programm, um aus Code-Fragmenten einer Markdown-Datei
kompilierbare Programme zu extrahieren.

## Genereller Aufbau

`md-patcher` ist sehr einfach gehalten.
Es liest eine Markdown-Datei über die Standard-Eingabe
und probiert darin enthaltene Code-Fragmente zu Dateien
zusammenzusetzen.
Diese werden am Ende dann herausgeschrieben.
Den Datei-Namen nimmt `md-patcher` aus den normalen
Text-Blöcken.
Jedes Inline-Codefragment ändert den Namen der aktuellen
Datei.
Im Moment lautet die Datei als `md-patcher`.
Bevor ein relevantes Code-Fragment kommt, sollte die Datei
also zum Beispiel mit `md-patcher.cpp` auf einen sinnvolleren
Namen gesetzt werden.
Das Programm prüft nicht, ob der Name sinnvoll ist.
Zur Sicherheit sollte immer direkt vor einem Code-Block
der Name der Datei angegeben werden.
Dies erleichtert zusätzlich dem Leser,
den Zusammenhang zu begreifen.

Die Datei muss nicht auf einen Schlag angegeben werden.
Es können Fragmente angegeben werden,
die im Laufe des Dokumentes erweitert werden können.
Ein Anfang von `md-patcher.cpp` könnte zum Beispiel so
aussehen:

```c++
int main() {
	// parse input
	// write output
	return 0;
}
```

Das Programm ist in C++ geschrieben.
Genauer: dem C++17 Standard.
Ich denke,
diese Sprache erlaubt eine vernünftige Balance
zwischen Kompaktheit und Ausführungsgeschwindigkeit.
Im Gegensatz zu C gibt es schon mächtige Werkzeuge zum
Umgang mit Containern.
Und trotzdem ist das erzeugte Programm schnell.

Das gerade angegebene Fragment definiert nur eine
Funktion `main`.
Dies ist die zentrale Funktion,
die aufgerufen wird,
wenn wir das Programm starten.
Die Zeichen `//` leiten einen Kommentar ein.
Alles was hinter den beiden Zeichen steht,
wird vom Compiler ignoriert.

Ein ausführbares Programm kann zum Beispiel unter Linux
im Terminal mit

```
$ c++ md-patcher.cpp -o md-patcher
```

erzeugt werden werden.
Wenn Ihr C++ Compiler anders heißt,
muss der Aufruf entsprechend angepasst werden.
Optimierungsoptionen sind in der Regel nicht notwendig.
Auch das Programm ist sehr einfach und sollte schnell laufen.

Zugegeben, das Programm macht noch nicht sehr viel.
Aber wir können es jetzt Stück für Stück erweitern.
Um zum Beispiel eine Struktur zu haben,
in der die Dateien zwischengespeichert werden,
können wir `md-patcher.cpp` wie folgt erweitern:

```c++
#include <map>
#include <vector>
#include <string>

using Lines = std::vector<std::string>;
using Files = std::map<std::string, Lines>;
static Files pool;
// ...
```

Das Programm definiert,
dass `Lines` eine Liste von Zeichenketten (Strings) ist
und `Files` ein Verzeichnis von `Lines`,
die über ihren Namen identifiziert werden.
Mit den `#include`-Zeilen werden die Definitionen
der verwendeten Klassen eingebunden.
Es handelt sich dabei um Standard-Klassen,
die mit dem C++-Compiler mitgeliefert werden.
Das Schlüsselwort `static` bewirkt,
dass die Variable nur in dieser Datei verwendet werden
darf.
Es hilft,
den Namensraum sauber zu halten.

Wichtig ist die Zeile mit dem Füll-Kommentar `// ...`.
Genau diese Zeilen erkennt `md-patcher` und fügt das bisher
bestehende Programm ein.
`md-patcher` probiert,
das bestehende Program mit dem Fragment
zusammenzuführen.
Dabei nutzt es zum einen identische Zeilen als
Verankerung der beiden Teile miteinander.
Zum anderen wird mit den Füll-Kommentaren angezeigt,
an welcher Stelle im Fragment der bestehende Code
eingesetzt werden kann.

Der resultierende Code
(dessen Ausgabe jetzt nach `/dev/null` geschrieben wird),
ist also:

```c++
#include <map>
#include <vector>
#include <string>

using Lines = std::vector<std::string>;
using Files = std::map<std::string, Lines>;
static Files pool;
int main() {
	// parse input
	// write output
	return 0;
}
```

Gleich werden wir sehen,
dass auch mehrere Füll-Kommentare in einem Fragment
verwendet werden können.
Nutzen wir Fragmente,
um das Programm `md-patcher.cpp` vollständig zu beschreiben.

## Ausgabe

Zäumen wir das Pferd von hinten auf und betrachten zuerst
die Ausgabe.
Es werden einfach alle Zeilen in die entsprechenden
Dateien geschrieben:

```c++
#include <fstream>
// ...
int main() {
	// ...
	// write output
	for (const auto &f: pool) {
		std::ofstream out(f.first.c_str());
		for (const auto &l: f.second) {
			out << l << '\n';
		}
	}
	// ...
}
```

Die äußere `for`-Schleife iteriert über alle Dateien,
die in der Eingabe gefunden wurden.
Für jede Datei wird ein Ausgabe-Strom geöffnet.
In diesen werden alle Zeilen der Datei hineingeschrieben.
Die Einträge der `pool`-Variablen sind Paare:
das erste Teil-Element `first` enthält den Namen der Datei.
Das zweite Teil-Element `second` die Liste der Zeilen.
Mit dem `<<` Operator können Elemente in Ausgabe-Ströme
geschrieben werden.
Die Zeilen werden mit einem einfachen Linefeed abgeschlossen.

Wenn die Füll-Kommentare mitten im Code stehen,
werden so lange bestehende Zeichen verwendet,
bis die nächste Zeile identisch ist,
oder eine Zeile mit geringerer Einrückung kommt.
In diesem Fall beendet einmal die Definition von
`main` und einmal der Kommentar `// write output` das
Kopieren.
Die gemeinsamen Zeilen werden übernommen und nicht
erneut eingefügt.

Wichtig ist hierbei zu beachten,
dass `md-patcher` keine eingefügten Zeilen erkennen kann,
die nach einem Füll-Komentar vor einer gemeinsamen
Zeile kommen.
Da keine bestehende Zeile mit der neuen Zeile
übereinstimmt,
wird das ganze bestehende Programm an dieser Stelle
eingesetzt.
Danach wird der gesamte Rest des Fragments am Ende
angehängt.
Inklusive der gemeinsamen Zeile
(die damit doppelt vorkommt).

## Eingabe lesen

Um das Programm einfach zu halten,
wird die aktuell gelesene Zeile und ihre Zeilennummer
in globalen Variablen abgelegt.
Zusätzlich gibt es in `md-patcher.cpp` eine Funktion,
um die nächste Zeile zu lesen:

```c++
// ...
#include <string>
#include <iostream>
#include "line-reader.h"

static std::string line;
static File_Position line_pos { "", 0 };
static Line_Reader reader { "", std::cin };

static bool next() {
	for (;;) {
		if (! reader.next(line)) {
			return false;
		}
		File_Position pos {
			line_pos.parse_line_macro(line)
		};
		if (pos) {
			line_pos = pos;
			continue;
		}
		++line_pos;
		return true;
	}
}

std::ostream &err_pos() {
	return std::cerr << reader.pos().file_name() <<
		':' << reader.pos().line() << ' ';
}

// next defined
// ...
```

Auch Funktionen können als `static` markiert werden.
Sie können dann auch nur innerhalb der aktuellen
Übersetzungseinheit verwendet werden.

Der zusätzliche Kommentar hilft uns später Funktionen
zu definieren,
die `next` aufrufen.
Diese müssen nach der Funktion `next` definiert werden,
oder es gibt einen Fehler bei der Übersetzung.

Damit kann das Lesen in der `main` Funktion in
`md-patcher.cpp` beschrieben werden
(auch wenn es die aufgerufenen Funktionen noch nicht gibt):

```c++
// ...
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
	// ...
}
```

Es werden so lange Zeilen gelesen,
bis das Ende erreicht ist.
Mehrere Markdown-Dateien
(z.B. mehrere Kapitel eines Buches)
können gemeinsam verarbeitet werden,
wenn sie vorher mit `cat` zusammengefasst werden.

Die Funktion `starts_with` in `md-patcher.cpp` prüft einfach,
ob ein String mit einer bestimmten Sequenz beginnt:

```c++
// ...
static std::string line;

static bool starts_with(
	const std::string &base,
	const std::string &prefix
) {
	return base.size() >= prefix.size() &&
		base.substr(0, prefix.size()) == prefix;
}
// ...
```

Hier sieht man wieder, wie eine bestehende Zeile
herangezogen wird,
um einen geeigneten EinfÜgepunkt der Funktion zu finden.

Ebenfalls recht einfach kann ein neuer
Dateiname ermittelt werden.
Es wird in der aktuellen Zeile geprüft,
ob zwei Backticks vorkommen.
Der String zwischen den letzten zwei Backticks ist dann
der aktuelle Dateiname:

```c++
// ...
static std::string line;

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
static Files pool;

static inline bool read_patch(Lines &lines) {
	if (!next()) { return false; }
	Lines::iterator cur { lines.begin() };
	File_Position pos { "", 0 };
	while (cur != lines.end()) {
		File_Position p { pos.parse_line_macro(*cur) };
		if (! p) { break; }
		pos = p;
		++cur;
	}
	std::string indent;
	while (line != "```") {
		// handle code
		if (! next()) {
			err_pos() << "end of file in code block\n";
			return false;
		}
	}
	if (cur != lines.end()) {
		err_pos() << "incomplete patch\n";
	}
	return next();
}
// ...
```

Es gibt folgende Fälle beim Code-Parsen zu unterscheiden:

```c++
// ...
static inline bool read_patch(Lines &lines) {
	// ...
	while (line != "```") {
		// handle code
		if (line_is_wildcard(indent)) {
			// do wildcard
			continue;
		} else if (cur != lines.end() && line == *cur) {
			++cur;
			++pos;
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
static inline bool read_patch(Lines &lines) {
	// ...
	while (line != "```") {
		// ...
			// insert line
			auto pos = cur - lines.begin();
			lines.insert(cur, line);
			cur = lines.begin() + (pos + 1);
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
static inline bool read_patch(Lines &lines) {
	// ...
	while (line != "```") {
		// ...
			// do wildcard
			if (! do_wildcard(
				indent, cur, lines.end()
			)) { return false; }
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
static Files pool;

static inline bool do_wildcard(
	const std::string &indent,
	Lines::iterator &cur,
	const Lines::const_iterator &end
) {
	if (! next()) {
		err_pos() << "end of file after wildcard\n";
		return false;
	}
	while (cur != end &&
		(line == "```" || *cur != line) &&
		starts_with(*cur, indent)
	) { ++cur; }
	return true;
}
// ...
```

Damit ist der gesamte Quellcode beschrieben.

Und aus dieser Markdown-Datei wurde mit `md-patcher`
das Programm selbst extrahiert.