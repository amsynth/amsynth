#!/bin/bash

set -o errexit
set -o xtrace

cd /tmp

if [ ! -f lv2lint-0.6.0/x86_64-linux-gnu-stretch/bin/lv2lint ]; then
  wget --quiet https://dl.open-music-kontrollers.ch/lv2lint/stable/lv2lint-0.6.0.zip
  unzip -qq lv2lint-0.6.0.zip lv2lint-0.6.0/x86_64-linux-gnu-stretch/bin/lv2lint
  rm lv2lint-0.6.0.zip
fi

./lv2lint-0.6.0/x86_64-linux-gnu-stretch/bin/lv2lint -d -M pack http://code.google.com/p/amsynth/amsynth
