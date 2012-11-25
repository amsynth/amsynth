# presets2ttl.py
#
# Converts an amsynth preset bank file to an LV2 presets ttl manifest.
#

import re;
import sys;

pluginURI = 'http://code.google.com/p/amsynth/amsynth'

def main(argv=sys.argv):
	presets = []
	currentPreset = {}
	with open(argv[1], 'rb') as file:
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

	for preset in presets:
		name = 'factory_preset_%s' % re.sub(r'\s', '_', preset['name'].lower())
		filename = '%s.ttl' % name
		print '<%s#%s>' % ( pluginURI, name )
		print '    a pset:Preset ;'
		print '    lv2:appliesTo <%s> ;' % pluginURI
		print '    rdfs:seeAlso <%s> .' % filename
		print ''

		with open(filename, 'w') as file:
			print >> file, '@prefix atom: <http://lv2plug.in/ns/ext/atom#> .'
			print >> file, '@prefix lv2: <http://lv2plug.in/ns/lv2core#> .'
			print >> file, '@prefix pset: <http://lv2plug.in/ns/ext/presets#> .'
			print >> file, '@prefix rdf: <http://www.w3.org/1999/02/22-rdf-syntax-ns#> .'
			print >> file, '@prefix rdfs: <http://www.w3.org/2000/01/rdf-schema#> .'
			print >> file, '@prefix state: <http://lv2plug.in/ns/ext/state#> .'
			print >> file, '@prefix xsd: <http://www.w3.org/2001/XMLSchema#> .'
			print >> file, ''
			print >> file, '<%s#%s>' % ( pluginURI, name )
			print >> file, '    a pset:Preset ;'
			print >> file, '    lv2:appliesTo <%s> ;' % pluginURI
			print >> file, '    rdfs:label "%s" ;' % preset['name']
			print >> file, '    lv2:port ['
			for key, value in preset['parameters'].items():
				print >> file, '        lv2:symbol "%s" ;' % key
				print >> file, '        pset:value %f' % float(value)
				if key == preset['parameters'].keys()[-1]:
					print >> file, '    ] .'
				else:
					print >> file, '    ] , ['

if __name__ == '__main__':
	main()
