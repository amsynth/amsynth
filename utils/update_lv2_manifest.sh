#!/bin/bash

csplit data/amsynth.lv2/amsynth.ttl '/#presets/' && \
echo '#presets' >> xx00 && echo >> xx00 && \
python utils/lv2_presets.py >> xx00 && \
rm data/amsynth.lv2/amsynth.ttl && \
mv xx00 data/amsynth.lv2/amsynth.ttl && \
rm xx01
