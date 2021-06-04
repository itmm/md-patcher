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
also zum Beispiel mit `main.cpp` auf einen sinnvolleren
Namen gesetzt werden.

Die Datei muss nicht auf einen Schlag angegeben werden.
Es können Fragmente angegeben werden,
die im Laufe des Dokumentes erweitert werden können.
Ein Anfang von `main.cpp` könnte zum Beispiel so aussehen:

```c++
int main() {
	// parse input
	// write output
	return 0;
}
```

Zugegeben, das Programm macht noch nicht sehr viel.
Aber wir können es jetzt Stück für Stück erweitern.
Um zum Beispiel eine Struktur zu haben,
in der die Dateien zwischengespeichert werden,
können wir `main.cpp` wie folgt erweitern:

```c++
#include <map>
#include <vector>
#include <string>

using Lines = std::vector<std::string>;
using Files = std::map<std::string, Lines>;
static Files pool;
// ...
```

Wichtig ist dabei die Kommentarzeile `// ...`.
Genau nach dieser Zeile wird gesucht und das bisher
bestehende Programm eingefügt.
Der resultierende Code (dessen Ausgabe jetzt nach `/dev/null` geschrieben wird), ist also:

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

Nutzen wir diese Elemente,
um das Programm `main.cpp` vollständig zu beschreiben.

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
}
```

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
Zusätzlich gibt es in `main.cpp` eine Funktion,
um die nächste Zeile zu lesen:

```c++
// ...
#include <string>
#include <iostream>

static std::string line;
static int line_nr { 0 };

bool next() {
	return std::getline(std::cin, line) && ++line_nr;
}

// next defined
// ...
```

Damit kann das Lesen in der `main` Funktion in `main.cpp`
beschrieben werden
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

Die Funktion `starts_with` in `main.cpp` prüft einfach,
ob ein String mit einer bestimmten Sequenz beginnt:

```c++
// ...
static std::string line;

bool starts_with(
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

Die Funktion `read_patch` in `main.cpp` liest das
Fragment Zeile für Zeile,
während es einen Iterator auf die bisherigen Zeilen der
Datei hält:

```c++
// ...
static Files pool;

bool read_patch(Lines &lines) {
	if (!next()) { return false; }
	Lines::iterator cur { lines.begin() };
	std::string indent;
	while (line != "```") {
		// handle code
		if (! next()) {
			std::cerr << "end of file in code block\n";
			return false;
		}
	}
	return next();
}
// ...
```

Es gibt folgende Fälle beim Code-Parsen zu unterscheiden:

```c++
// ...
bool read_patch(Lines &lines) {
	// ...
	while (line != "```") {
		// handle code
		if (line_is_wildcard(indent)) {
			// do wildcard
			continue;
		} else if (cur != lines.end() && line == *cur) {
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
bool read_patch(Lines &lines) {
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
welcher der Funktion in `main.cpp` mitgegeben wird,
um ein vorzeitiges Ende des Kopierens zu erkennen:

```c++
// ...
bool read_patch(Lines &lines) {
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
folgende Funktion in `main.cpp`:

```c++
// ...
static std::string line;

bool line_is_wildcard(std::string &indent) {
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
// ...
```

Damit ist der gesamte Quellcode beschrieben.

Und aus dieser Markdown-Datei wurde mit `md-patcher`
das Programm selbst extrahiert.