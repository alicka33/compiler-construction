VSLC := ../build/vslc

PS2_EXAMPLES := $(patsubst %.vsl, %.ast, $(wildcard ps2-parser/*.vsl))
PS2_GRAPHVIZ := $(patsubst %.vsl, %.svg, $(wildcard ps2-parser/*.vsl))
PS3_EXAMPLES := $(patsubst %.vsl, %.ast, $(wildcard ps3-simplify/*.vsl))
PS3_GRAPHVIZ := $(patsubst %.vsl, %.svg, $(wildcard ps3-simplify/*.vsl))

.PHONY: all ps2 ps2-graphviz ps3 ps3-graphviz clean ps2-check ps3-check

all: ps2 ps3

ps2: $(PS2_EXAMPLES)
ps2-graphviz: $(PS2_GRAPHVIZ)

ps3: $(PS3_EXAMPLES)
ps3-graphviz: $(PS3_GRAPHVIZ)

ps2-parser/%.ast: ps2-parser/%.vsl $(VSLC)
	$(VSLC) -t < $< > $@

ps2-parser/%.svg: ps2-parser/%.vsl $(VSLC)
	GRAPHVIZ_OUTPUT=1 $(VSLC) -t < $< | dot -Tsvg -o $@

ps3-simplify/%.ast: ps3-simplify/%.vsl $(VSLC)
	$(VSLC) -T < $< > $@

ps3-simplify/%.svg: ps3-simplify/%.vsl $(VSLC)
	GRAPHVIZ_OUTPUT=1 $(VSLC) -T < $< | dot -Tsvg -o $@

clean:
	-rm -rf */*.ast */*.svg */*.symbols */*.S */*.out

ps2-check: ps2
	cd ps2-parser; \
	find * -wholename "suggested/*.ast" | awk -F/ '{print $$0 " " $$2}' | xargs -L 1 diff -s --unified=0
	@echo "No differences found in PS2!"

ps3-check: ps3
	cd ps3-simplify; \
	find * -wholename "suggested/*.ast" | awk -F/ '{print $$0 " " $$2}' | xargs -L 1 diff -s --unified=0
	@echo "No differences found in PS3!"
