% AMSYNTH(1) amsynth VERSION | Commandes utilisateur
%
% Janvier 2017

NOM
===

amsynth - un synthétiseur logiciel soustractif à deux oscillateurs

SYNOPSIS
========

**amsynth** \[options\]

DESCRIPTION
===========

amsynth est un synthétiseur logiciel facile à utiliser avec une topologie de synthétiseur soustractif classique.

OPTIONS
=======

Les options suivantes écrase celle du fichier de configuration (\$HOME/.amSynthrc).

`-h`

:   affiche un message d'aide

`-v`

:   affiche les informations de version

`-x`

:   run in headless mode (without GUI)

`-b` \<nom\_de\_fichier\>

:   utilise \<nom\_de\_fichier\> en tant que banque pour sauvegarder les pré-réglages

`-t` \<nom\_de\_fichier\>

:   utilise \<nom\_de\_fichier\> en tant que fichier d'accordage

`-a` \<chaîne\_de\_caractères\>

:   paramètre le pilote de sortie son à utiliser \[alsa/oss/auto(défaut)\]

`-r` \<entier\>

:   paramètre le taux d'échantillonnage à utiliser

`-m` \<chaîne\_de\_caractères\>

:   paramètre le pilote MIDI à utiliser \[alsa/oss/auto(défaut)\]

`-c` \<entier\>

:   paramètre le canal MIDI auquel répondre (défaut=tous)

`-p` \<entier\>

:   paramètre la polyphonie (voix actives maximum)

`-n` \<nom\>

:   spécifie le nom de client JACK à utiliser

`--jack_autoconnect[=<true|false>]`

:   automatically connect jack audio ports to hardware I/O ports. (Default: true)

FICHIERS
========

`$HOME/.amSynthrc`

:   Configuration pour amsynth.

`$HOME/.amsynth/*`

:   Banques et autres.

`$HOME/.amSynth.presets`

:   Pré-réglages.

ENVIRONNEMENT
=============

`AMSYNTH_NO_GUI`

:   If AMSYNTH\_NO\_GUI is set, amsynth is started in headless mode (without GUI).

`AMSYNTH_SKIN`

:   Specifies the directory from which amsynth should load its skin.

BOGUES & DEMANDE DE FONCTIONNALITÉS
===================================

Si vous trouvez un bogue, ou si vous voulez proposer une nouvelle fonctionnalité, veuillez le reporter à https://github.com/amsynth/amsynth/issues .

AUTEURS
=======

Nick Dowell et contributeurs. Voir la liste complète dans le dialogue d'amsynth : "Aide" -> "À propos".

La version initiale de cette page de manuel a été écrite par Olivier Humbert <trebmuh@tuxfamily.org> le 06 septembre 2016 en tant que partie du projet LibraZiK (et peut être utilisé par d'autres).
