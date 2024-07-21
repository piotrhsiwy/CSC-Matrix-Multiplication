# Projektbericht: Multiplikation dünnbesetzter Matrizen

### Autoren
- Piotr Siwy
- Dominik Pecher
- Lars Ellinger

## Ziel des Projekts
Es sollten verschiedene Algorithmen zur Matrixmultiplikation dünnbesetzter Matrizen im CSC Format zu entwickelt und/oder
optimiert und verglichen werden. Dazu sollten der mit C Standard C17 und gcc als Compiler verwendet werden.

## Ausführen des entwickelten Programms
Im Ordner *Implementierung/* befindet sich ein *Makefile*, über das das Programm unter Linux erstellt werden kann. *gcc*
und *make* müssen dazu installiert sein.

```bash
cd ./Implementierung
make release
./main --help
./main -a matrixA.txt -a matrixB.txt -o ergebnis.txt #Beispielausführung
```

## Arbeitsweise
Es wurde einmal wöchentlich ein Stand-up abgehalten, bei dem der Arbeitsfortschritt und aktuelle Probleme besprochen und
die nächsten Aufgaben verteilt wurden.

Im Zuge dieser Meetings wurde stets ein Codereview aller Codeänderungen seit dem letzten Termin von Allen durchgeführt,
um Bugs zu vermeiden.

Von git-branches wurde abgesehen, um schnellere Zusammenarbeit zu ermöglichen. Um dennoch Merge-Konflikte zu vermeiden
wurde der Quellcode in viele separate Dateien aufgeteilt und Änderungen klar kommuniziert.

## Performanzanalyse
Es wurden parallel mit zwei verschiedenen Systemen und CPU-Architekturen die Performanz der Implementierungen gemessen.

Dazu wurde eine *Benchmarking-Funktionalität* in das Programm eingebaut, welche über die Option *-U <csv-Datei-Pfad>*
verwendet werden kann. Es werden Matrizen unterschiedlicher Größe und Dichte automatisch generiert und mehrfach
multipliziert. Die Zeiten der einzelnen Algorithmen werden in der angegebenen Datei dokumentiert und die Ergebnisse
überprüft.

## Arbeitsaufteilung
Das Projekt wurde in drei große Bereiche unterteilt, wovon jeder einen übernommen hat. Jedoch hat jedes Mitglied in
jedem Teilbereich kleine Änderungen übernommen und/oder ausgeholfen.

Für verschiedene Implementierung Algorithmen war *Piotr Siwy* verantwortlich, wobei die Ideen für die meisten Algorithmen
in den Meetings jeweils gesammelt und von allen Mitgliedern verbessert wurden. Darüber hinaus war er für verschiedene
Hilfsfunktionen zu Matrizen verantwortlich, welche nicht explizit genannt werden. **TODO: weitere Gebiete?**

Für das Erstellen von Benchmarks (sowohl -V als auch -U) und alles direkt damit Verbundene war *Dominik Pecher*
verantwortlich. Zudem kümmerte er sich um das Einlesen von Matrizen, Umsetzung von adaptiven dynamischen Arrays in C
ohne Templates und die Struktur des Projektberichts. Um das Beheben sinnvoller und Ignorieren fälschlicher Warnungen
sowie Debugging von Memory-Leaks kümmerte er sich ebenfalls. Die letzten beiden Punkte wurden jedoch von allen Anderen
mit bearbeitet. **TODO: weitere Gebiete?**

Für das Parsen von Argumenten, Speichern von Matrizen und Benchmarks war *Lars Ellinger* zuständig. Des Weiteren war es
seine Aufgabe ein Programm zur Visualisierung der Benchmark-Ergebnisse und damit die Grafiken für die Präsentation
zu erstellen. Außerdem kümmerte er sich um einheitliche Code-Formatierung mittels Clang-Tidy und erstellte das
*Makefile*. **TODO: weitere Gebiete?**

Alle nicht genannten Aufgabengebiete können nicht überwiegend einer bestimmten Person zugeordnet werden.
