VSLC := ../build/vslc

PS2_EXAMPLES := $(patsubst %.vsl, %.ast, $(wildcard ps2-parser/*.vsl))
PS2_GRAPHVIZ := $(patsubst %.vsl, %.svg, $(wildcard ps2-parser/*.vsl))

.PHONY: all ps2 ps2-graphviz clean ps2-check

all: ps2

ps2: $(PS2_EXAMPLES)
ps2-graphviz: $(PS2_GRAPHVIZ)

ps2-parser/%.ast: ps2-parser/%.vsl $(VSLC)
	$(VSLC) -t < $< > $@

%.svg: %.vsl $(VSLC)
	GRAPHVIZ_OUTPUT=1 $(VSLC) -t < $< | dot -Tsvg -o $@

clean:
	-rm -rf */*.ast */*.svg */*.symbols */*.S */*.out

ps2-check: ps2
	cd ps2-parser; \
	find * -wholename "suggested/*.ast" | awk -F/ '{print $$0 " " $$2}' | xargs -L 1 diff -s --unified=0
	@echo "No differences found in PS2!"
