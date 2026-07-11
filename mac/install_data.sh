#!/bin/bash

set -euxo pipefail

mkdir -p "/Library/Audio/Presets/Nick Dowell/amsynth"
cp -c ../data/banks/*.bank "/Library/Audio/Presets/Nick Dowell/amsynth/"

mkdir -p "/Library/Application Support/amsynth/skins/default"
cp -c ../data/skins/default/layout.ini ../data/skins/default/*.png "/Library/Application Support/amsynth/skins/default/"
