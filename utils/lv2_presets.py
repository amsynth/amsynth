# presets2ttl.py
#
# Converts an amsynth preset bank file to an LV2 presets ttl manifest.
#

import os, re, sys

pluginURI = 'http://code.google.com/p/amsynth/amsynth'

lv2_file_header = '''\
@prefix atom: <http://lv2plug.in/ns/ext/atom#> .
@prefix lv2: <http://lv2plug.in/ns/lv2core#> .
@prefix pset: <http://lv2plug.in/ns/ext/presets#> .
@prefix rdf: <http://www.w3.org/1999/02/22-rdf-syntax-ns#> .
@prefix rdfs: <http://www.w3.org/2000/01/rdf-schema#> .
@prefix state: <http://lv2plug.in/ns/ext/state#> .
@prefix xsd: <http://www.w3.org/2001/XMLSchema#> .
'''

lv2_preset_header = '''\
<http://code.google.com/p/amsynth/amsynth#{preset_uri}>
    a pset:Preset ;
    lv2:appliesTo <http://code.google.com/p/amsynth/amsynth> ;
    rdfs:label "{label}" ;'''

def amsynth_bank_read(filepath):
	presets = []
	currentPreset = {}
	with open(filepath, 'rb') as file:
		for line in file:
			line = line.strip()
			if line.startswith('<preset> <name> '):
				if len(currentPreset):
					presets.append(currentPreset)
				currentPreset = { 'name': line[16:], 'parameters':{} }
			if line.startswith('<parameter>'):
				tokens = re.split(r'\s', line.strip())
				if len(tokens) == 3 and tokens[0] == '<parameter>':
					currentPreset['parameters'][tokens[1]] = tokens[2]
	presets.append(currentPreset)
	currentPreset = {}
	return presets

def lv2_bank_write(filepath, bank_name, presets):
	with open(filepath, 'w') as file:
		print >> file, lv2_file_header

		bank_uri = 'http://code.google.com/p/amsynth/amsynth#' + bank_name
		print '''<%s>
    a pset:Bank ;
    rdfs:label "%s" .
''' % (bank_uri, bank_name)

		for i, preset in enumerate(presets):
			preset_uri = bank_name + '_%03d_' % i + re.sub(r'\s', '_', preset['name'])
			label = '%s: %03d: %s' % (bank_name, i, preset['name'])
			# When banks are more widely supported, we should switch to this:
			#label = '%03d: %s' % (i, preset['name'])
			print lv2_preset_header.format(preset_uri=preset_uri, label=label)
			print '    rdfs:seeAlso <{}> .'.format(os.path.basename(filepath))
			print ''
			print >> file, lv2_preset_header.format(preset_uri=preset_uri, label=label)
			print >> file, '    pset:bank <%s> ;' % bank_uri
			print >> file, '    lv2:port ['
			for key, value in preset['parameters'].items():
				print >> file, '        lv2:symbol "%s" ;' % key
				print >> file, '        pset:value %f' % float(value)
				if key == preset['parameters'].keys()[-1]:
					print >> file, '    ] .'
				else:
					print >> file, '    ] , ['
			print >> file, ''

def main(argv=sys.argv):
	for filename in sorted(os.listdir('data/banks')):
		if filename.endswith('.bank'):
			presets = amsynth_bank_read(os.path.join('data/banks', filename))
			bank_name = filename.replace('.bank', '').replace('.amSynth', '')
			ttlfilename = filename + '.ttl'
			lv2_bank_write(os.path.join('data/amsynth.lv2', ttlfilename), bank_name, presets)

if __name__ == '__main__':
	main()
