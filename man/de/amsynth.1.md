% AMSYNTH(1) amsynth VERSION | Dienstprogramme für Benutzer
%
% Oktober 2020

NAME
====

amsynth - ein subtraktiver Softwaresynthesizer mit zwei Oszillatoren

ÜBERSICHT
=========

**amsynth** \[Optionen\]

BESCHREIBUNG
============

amsynth ist ein einfach zu verwendender Softwaresynthesizer mit einer klassischen subtraktiven Synthesizer-Topologie.

OPTIONEN
========

Die folgenden Optionen setzen diejenigen aus der Konfigurationsdatei (\$HOME/.amSynthrc) außer Kraft.

`-h`

:   zeigt eine Hilfe an

`-v`

:   zeigt Versionsinformationen

`-x`

:   führt amsynth ohne grafische Oberfläche aus

`-b` \<Datei\>

:   nutzt \<Datei\> als Bank zum Speichern von Presets

`-t` \<Datei\>

:   nutzt \<Datei\> als Stimmungsdatei

`-a` \<string\>

:   setzt den Audio-Ausgabe-Treiber als \[alsa/oss/auto(Standard)\]

`-r` \<int\>

:   setzt die zu verwendende Samplerate

`-m` \<string\>

:   setzt den zu verwendenden MIDI-Treiber \[alsa/oss/auto(Standard)\]

`-c` \<int\>

:   setzt den MIDI-Kanal, auf den reagiert werden soll (Standard=alle)

`-p` \<int\>

:   setzt die Polyphonie (maximale Anzahl der aktiven Stimmen)

`-n` \<Name\>

:   legt den JACK-Client-Namen fest

`--jack_autoconnect[=<true|false>]`

:   legt die automatische Verbindung von JACK-Audio-Ports mit Hardware-I/O-Ports fest. (Standard: true)

`--force-device-scale-factor` \<scale\>

DATEIEN
=======

`$XDG_CONFIG_HOME/amsynth/config`

:   amsynth-Konfiguration.

`$XDG_DATA_HOME/amsynth/banks/*`

:   Banks und Presets.

UMGEBUNG
========

`AMSYNTH_NO_GUI`

:   Wenn AMSYNTH\_NO\_GUI gesetzt ist, wird amsynth ohne grafische Oberfläche gestartet.

`AMSYNTH_SKIN`

:   Gibt das Verzeichnis an, aus dem amsynth seinen Skin laden soll.

`GDK_SCALE`

BUGS & FEATURE REQUESTS
=======================

Wenn Sie einen Bug gefunden haben oder ein neues Feature vorschlagen möchten, melden Sie dies bitte auf https://github.com/amsynth/amsynth/issues .

AUTOREN
=======

Nick Dowell und Mitwirkende. Eine vollständige Liste finden Sie im amsynth-Dialog unter "Hilfe" -> "Über".

Die ursprüngliche Version dieser Handbuchseite wurde am 6. September 2016 von Olivier Humbert <trebmuh@tuxfamily.org> als Teil des LibraZiK-Projekts geschrieben (und kann von anderen verwendet werden).
