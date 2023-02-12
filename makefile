FLEX = flex
BISON = bison
CXX = g++
CXXFLAGS = -Iinc

all: emulator linker asembler

emulator: src/emulator.cpp inc/emulator.hpp | linker
	$(CXX) -o $@ $^

linker: src/linker.cpp inc/linker.hpp | asembler
	$(CXX) -o $@ $^

asembler: src/main.cpp src/assembler.cpp inc/assembler.hpp inc/structs.hpp lex.yy.cpp parser.tab.cpp parser.tab.hpp
	$(CXX) $(CXXFLAGS) -o $@ $^
	@$(MAKE) clean_assembly

parser.tab.cpp parser.tab.hpp: misc/parser.ypp
	$(BISON) -d $<

lex.yy.cpp: misc/lexer.lpp
	$(FLEX) -o $@ $<


clean_assembly:
	rm -f lex.yy.cpp parser.tab.cpp parser.tab.hpp

.PHONY: clean_assembly