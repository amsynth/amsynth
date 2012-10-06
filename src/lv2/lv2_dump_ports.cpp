
#include "../Preset.cc"
#include "../Parameter.cc"

#include <cstdio>

int main ()
{
	Preset preset;
	for (size_t i=0; i<preset.ParameterCount(); i++) {
		Parameter &p = preset.getParameter(i);
		printf("        a lv2:InputPort ;\n");
		printf("        a lv2:ControlPort ;\n");
		printf("        lv2:index %d ;\n", i + 3);
		printf("        lv2:symbol \"%s\" ;\n", p.getName().c_str());
		printf("        lv2:name \"%s\" ;\n", p.getName().c_str());
		printf("        lv2:portProperty epp:hasStrictBounds ;\n");
		printf("        lv2:default %f ;\n", p.getValue());
		printf("        lv2:minimum %f ;\n", p.getMin());
		printf("        lv2:maximum %f ;\n", p.getMax());
		printf("    ] , [\n");
	}
	return 0;
}
