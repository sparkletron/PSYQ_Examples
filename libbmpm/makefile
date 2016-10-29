SOURCES = bmpmanip.c
LIBRARY = libbmpm.lib
PSX_CC = CCPSX.EXE
PSX_AR = PSYLIB.EXE
PSX_CFLAGS = -O3  -c
PSX_ARFLAGS = /u
PSX_OBJECTS = $(SOURCES:.c=.obj)


all: PSX_BUILD
	
PSX_BUILD: $(LIBRARY)
	
$(LIBRARY): $(PSX_OBJECTS)
	$(PSX_AR) $(PSX_ARFLAGS) $@ $<
	rm -f $<

%.obj: %.c
	$(PSX_CC) $< $(PSX_CFLAGS) -o $@

clean:
	rm -f $(PSX_OBJECTS) $(LIBRARY)