# Projekt GRA SoSe 2024 - Multiplikation dünnbesetzter Matrizen: CSC Format

### Autoren
- Piotr Siwy
- Dominik Pecher
- Lars Ellinger

## Ziel des Projekts
Ziel des Projekts ist es verschiedene Algorithmen zur Matrixmultiplikation dünnbesetzter Matrizen zu entwickeln,
optimieren und vergleichen. Dazu soll die Programmiersprache C mit Standard C17 und gcc als Compiler verwendet werden.

Manche Algorithmen sind selbst entwickelt, andere sind an wissenschaftlichen Veröffentlichungen angelehnt. Der Ursprung
ist jeweils im Quellcode mittels Kommentar beschreiben. ***TODO!!!***

## Ausführen des entwickelten Programms
Der Quellcode des Programms befindet sich im Ordner *Implementierung/*. Dort befindet sich ein *Makefile*, über das
das Programm unter Linux erstellt werden kann. *gcc* und *make* müssen dazu installiert sein.

```bash
cd ./Implementierung
make [release|debug|...] #genau eine der genannten Optionen
./main --help
./main -a matrixA.txt -a matrixB.txt -o ergebnis.txt #Beispielausführung
```

## Arbeitsweise
Es wurde mindestens einmal wöchentlich ein Stand-up abgehalten, bei dem der bisherige Arbeitsfortschritt, aktuelle
Probleme besprochen und die nächsten Aufgaben verteilt und besprochen wurden.

Im Zuge dieser Meetings wurde stets ein Codereview aller Codeänderungen seit dem letzten Termin von Allen durchgeführt,
um Bugs zu vermeiden.

Von git-branches wurde abgesehen, da sowohl Artemis diese nicht unterstützt, als auch, um schnellere Zusammenarbeit
zu ermöglichen. Um dennoch Merge-Konflikte zu vermeiden wurde der Quellcode in viele separate Dateien aufgeteilt und
Änderungen klar kommuniziert.

## Performanzanalyse
Gegen Ende des Projekts wurden parallel mit zwei verschiedenen Systemen und CPU-Architekturen die Performanz der
Implementierungen gemessen.

Dazu wurde eine *Benchmarking-Funktionalität* in das Programm eingebaut, welche über die Option *-U <csv-Datei-Pfad>*
verwendet werden kann. In diesem Modus werden Matrizen unterschiedlicher Größe und Dichte automatisch generiert und
mehrfach multipliziert. Die Zeiten der einzelnen Algorithmen werden in der angegebenen Datei dokumentiert und die
Ergebnisse überprüft.

Verwendete Systeme:

| System               | Prozessor                                    | Hauptspeicher    | Betriebssystem                        |
|----------------------|----------------------------------------------|------------------|---------------------------------------|
| Dell® Poweredge R730 | 14 x 3,3GHz Intel® Xeon® E5-2695 v3 (x86_64) | 128GB DDR4 (ECC) | Debian 12.6.0 - Kernel 6.1.0-22-amd64 |
| Raspberry Pi 4b      | 4 x 1,5GHz A72 (Arm)                         | 4GB LPDDR4       | Debian 12 - Kernel 6.6.31-rpt-rpi-v8  |

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
