# Projektbericht: Multiplikation dünnbesetzter Matrizen

## Ziel des Projekts
Es sollten Algorithmen zur Matrixmultiplikation dünnbesetzter Matrizen im CSC Format zu entwickelt und/oder optimiert
und verglichen werden. Dazu sollten der C Standard C17 und gcc als Compiler verwendet werden.

## Ausführen des entwickelten Programms
Im Ordner *Implementierung/* befindet sich ein *Makefile*, über das das Programm unter Linux erstellt werden kann. *gcc*
und *make* müssen dazu installiert sein.

```bash
cd ./Implementierung
make release
./main -a matrixA.txt -a matrixB.txt -o ergebnis.txt
```

## Arbeitsweise
Es wurde einmal wöchentlich ein Stand-up abgehalten, bei dem der Arbeitsfortschritt und aktuelle Probleme besprochen und
weitere Aufgaben verteilt wurden.

Im Zuge dieser Meetings wurde ein Codereview aller Codeänderungen seit dem letzten Termin von Allen durchgeführt.

Von git-Branches wurde abgesehen, um schnellere Zusammenarbeit zu ermöglichen. Um dennoch Merge-Konflikte zu vermeiden
wurde der Quellcode in viele separate Dateien aufgeteilt und Änderungen klar kommuniziert.

## Performanzanalyse
Es wurden parallel mit zwei verschiedenen Systemen und CPU-Architekturen die Performanz der Implementierungen gemessen.

Dazu wurde eine *Benchmarking-Funktionalität* in das Programm eingebaut: *-U <csv-Datei-Pfad>*. Es werden Matrizen
unterschiedlicher Größe und Dichte automatisch generiert und mehrfach multipliziert. Die Zeiten der einzelnen
Algorithmen werden in der angegebenen Datei dokumentiert und die Ergebnisse überprüft.

## Arbeitsaufteilung
Für verschiedene Algorithmen war *Piotr Siwy* verantwortlich, wobei die Ideen für die meisten Algorithmen
in den Meetings jeweils gesammelt und von allen Mitgliedern verbessert wurden. Darüber hinaus erstellte er verschiedene
Hilfsfunktionen zu Matrizen. Zudem optimierte der diese mittels u. A. Profiling. Der Quellcode dazu befindet sich
überwiegend in *algorithms/*.**TODO: weitere Gebiete?**

Für das Erstellen von Benchmarks und alles direkt damit Verbundene war *Dominik Pecher* verantwortlich. Zudem kümmerte
er sich um das Einlesen von Matrizen, Umsetzung von adaptiven dynamischen Arrays ohne Templates, weitere
utility-Funktionen und die Struktur des Projektberichts. Um das Beheben sinnvoller und Ignorieren fälschlicher Warnungen
sowie Debugging von Memory-Leaks kümmerte er sich ebenfalls. Letztere wurden von allen Anderen mit bearbeitet. Der
verbundene Quellcode befindet sich überwiegend in *util/* und *benchmarking/*. **TODO: weitere Gebiete?**

Für das Parsen von Argumenten, Speichern von Matrizen und Tests war *Lars Ellinger* zuständig. Es entwickelte auch ein
Programm zur Visualisierung der Benchmark-Ergebnisse und die Grafiken für die Präsentation. Außerdem kümmerte er sich um
einheitliche Code-Formatierung mittels Clang-Tidy und erstellte das *Makefile*. Der Quellcode ist in den entsprechenden
Ordnern zu finden.**TODO: weitere Gebiete?**

Alle nicht genannten Aufgabengebiete können nicht überwiegend einer bestimmten Person zugeordnet werden. Zudem hat jedes
Mitglied in anderen Teilbereichen kleine Änderungen vorgenommen und ausgeholfen.
