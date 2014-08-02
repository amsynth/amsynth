#!/bin/bash

csplit amsynth.lv2/amsynth.ttl '/#presets/' && \
echo '#presets' >> xx00 && echo >> xx00 && \
python utils/lv2_presets.py >> xx00 && \
rm amsynth.lv2/amsynth.ttl && \
mv xx00 amsynth.lv2/amsynth.ttl && \
rm xx01
