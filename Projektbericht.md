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
das Programm unter Linux erstellt werden kann - *siehe unten*. *gcc* und *make* müssen dazu installiert sein.

```bash
cd ./Implementierung
make [release|debug|...] #genau eine der genannten Optionen
./main --help
./main -a matrixA.txt -a matrixB.txt -o ergebnis.txt #Beispielausführung
```

## Arbeitsweise
Von den Projektmitgliedern wurde mindestens einmal wöchentlich ein Meeting einberufen, bei dem der bisherige
Arbeitsfortschritt, aktuelle Probleme besprochen und die nächsten Aufgaben verteilt und besprochen wurden.

Im Zuge dieser Meetings wurde auch stets ein Codereview aller Codeänderungen seit dem letzten Meeting von allen
Mitgliedern durchgeführt, um Bugs zu vermeiden.

Von git-branches wurde abgesehen, da sowohl Artemis diese nicht unterstützt, als auch, um schnellere Zusammenarbeit
zu ermöglichen. Um dennoch Merge-Konflikte zu vermeiden wurde der Quellcode in viele separate Dateien aufgeteilt.

**TODO: static/dynamic analysis**

## Performanzanalyse
Gegen Ende des Projekts wurden parallel mit zwei verschiedenen Systemen und CPU-Architekturen die Performanz der
verschiedenen Algorithmen gemessen.

Dazu wurde eine *Benchmarking-Funktionalität* verwendet, die in das Programm eingebaut ist und über die Option
*-U <csv-Datei-Pfad>* verwendet werden kann. In diesem Modus werden pseudo-zufällige Matrizen unterschiedlicher Größe
und Dichte automatisch generiert und mehrfach multipliziert. Die Zeit der einzelnen Algorithmen wird gemessen und die
Ergebnis-Matrizen verglichen. Die Ergebnisse werden in der angegebenen Datei dokumentiert.

Es wurden folgende Systeme verwendet:

**TODO: ggf. als Tabelle formatieren**
- Dell Poweredge R730:
  - Prozessor:
    - Architektur: x86_64
    - Kerne: 14 x 3,3GHz Intel® Xeon® E5-2695 v3 (ein Socket besetzt)
    - Cache: 35MB
  - Hauptspeicher: 128GB DDR4 (ECC)
  - Betriebssystem: baremetal minimal Debian 12.6.0 - Kernel 6.1.0-22-amd64
- Raspberry Pi 4b
  - Prozessor:
    - Architektur: Arm
    - Kerne: 4 x 1,5GHz A72
    - 1Mib Cache
  - Hauptspeicher: 4GB LPDDR4 (non-ECC)
  - Betriebssystem: baremetal Debian 12 - Kernel 6.6.31-rpt-rpi-v8

## Arbeitsaufteilung