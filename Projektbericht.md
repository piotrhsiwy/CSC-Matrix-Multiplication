# Projekt GRA SoSe 2024 - Multiplikation dünnbesetzter Matrizen: CSC Format

### Autoren
- Piotr Siwy
- Dominik Pecher
- Lars Ellinger

## Ziel des Projekts
Ziel des Projekts ist es verschiedene Algorithmen zur Matrixmultiplikation dünnbesetzter Matrizen zu entwickeln,
optimieren und vergleichen. Dazu soll die Programmiersprache C mit Standard C17 und gcc als Compiler verwendet werden.

Manche Algorithmen sind selbst entwickelt, während andere an wissenschaftlichen Veröffentlichungen angelehnt sind. Die
Algorithmen sind dazu jeweils im Quellcode mittels Kommentar beschreiben. ***TODO!!!***

## Ausführen des entwickelten Programms
Der Quellcode des Programms befindet sich im Ordner *Implementierung/*. Dort befindet sich auch ein *Makefile*, über das
das Programm erstellt werden kann - *siehe unten*.

```bash
cd ./Implementierung
make [release|debug|...] #eine der genannten Optionen
./main [optionen (bspw. --help oder -a matrixA.txt -a matrixB.txt -o ergebnis.txt)]
```

## Arbeitsweise
Von den Projektmitgliedern wurde mindestens einmal wöchentlich ein Meeting einberufen, bei dem der bisherige
Arbeitsfortschritt, aktuelle Probleme besprochen und die nächsten Aufgaben verteilt und besprochen wurden.

Im Zuge dieser Meetings wurde auch stets ein Codereview aller Codeänderungen seit dem letzten Meeting von allen
Mitgliedern durchgeführt, um Bugs zu vermeiden.

Von git-branches wurde abgesehen, da sowohl Artemis diese nicht unterstützt, als auch, um schnellere Zusammenarbeit
zu ermöglichen. Um dennoch Merge-Konflikte zu vermeiden wurde der Quellcode in viele separate Dateien aufgeteilt.

## Arbeitsaufteilung