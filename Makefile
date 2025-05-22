.PHONY: all tests clean libs

CXXFLAGS += -O3 -Wall -Wextra -Wpedantic -Werror -std=c++20 \
	-I./solid-require/include -I./lazy-write/include \
	-I./line-reader/include

APP = mdp
SOURCES = md-patcher.cpp

LIBS = solid-require/librequire.a lazy-write/liblazywrite.a \
	line-reader/liblinereader.a

LIBS_HEADER = solid-require/include/solid/require.h \
	lazy-write/include/lazy-write/lazy-write.h \
	line-reader/include/line-reader/line-reader.h

MDP_RUN = mdp.run

all: $(MDP_RUN) libs
	@$(MAKE) --no-print-directory tests

libs:
	@echo building libraries
	@$(MAKE) --no-print-directory -C solid-require
	@$(MAKE) --no-print-directory -C lazy-write
	@$(MAKE) --no-print-directory -C line-reader

$(MDP_RUN): $(wildcard *.md)
	@echo extracting source code
	@[ -x "$$(command -v mdp)" ] || echo "mdp not installed" 1>&2
	@[ -x "$$(command -v mdp)" ] && mdp README.md
	@date >$@

%.o:%.cpp
	@echo "  buliding" $@
	@$(CXX) $(CXXFLAGS) -c $< -o $@

tests: $(APP)
	@echo running unit-tests
	@./$(APP) --run-only-tests

$(APP): $(SOURCES) $(LIBS) $(LIBS_HEADER)
	@echo building $@
	@$(CXX) $(CXXFLAGS) $(SOURCES) $(LIBS) -o $@

clean:
	@echo remove temporaries
	@rm -f $(APP) $(MDP_RUN)
	@$(MAKE) --no-print-directory -C solid-require clean
	@$(MAKE) --no-print-directory -C lazy-write clean
	@$(MAKE) --no-print-directory -C line-reader clean
	@[ -x "$$(command -v mdp)" ] && rm -f $(SOURCES)
