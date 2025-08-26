PREFIX=.

lua_srcdir=lua-5.4.8/src
lua_lib=$(lua_srcdir)/one/liblua.o

override CFLAGS:=\
	$(shell pkg-config --cflags jack sndfile samplerate) \
	-isystem $(lua_srcdir) -MMD -O3 \
	$(CFLAGS)

override LDFLAGS:=\
	$(shell pkg-config --libs jack sndfile samplerate) \
	-lm \
	$(LDFLAGS)

sources=$(wildcard src/*.c) $(wildcard src/lua/*.c)
objects=$(sources:%.c=%.o)
depends=$(sources:%.c=%.d)

tools_sources=$(wildcard tools/*.c)
tools=$(tools_sources:%.c=%)

all: grainman $(tools)

grainman: $(objects) $(lua_lib)
	@printf "  CCLD\t%s\n" $(@)
	@$(CC) $(LDFLAGS) -o $(@) $(^)

$(lua_lib):
	@cd $(lua_srcdir)/one && make

-include $(depends)

.c.o:
	@printf "  CC  \t%s\n" $(@)
	@$(CC) $(CFLAGS) -o $(@) -c $(<)

tools/%: tools/%.c
	@printf "  CCLD\t%s\n" $(@)
	@$(CC) $(CFLAGS) $(LDFLAGS) -o $(@) $(<)

.PHONY: clean
clean:
	rm -rf $(objects) $(depends) *.d grainman $(tools)
