SOURCES = main.c
HEADERS = ../engine
PSX_EXEC = audioTest.exe
PSX_CC = CCPSX.EXE
PSX_CPE2X = CPE2XWIN.EXE
PSX_CFLAGS = -O3 -Dpsx -c
PSX_ADDRESS = 0x80010000
PSX_LDFLAGS =  -l libpad -l libmcrd -l libsio -l libds -l libeng -l libspu -l libyxml -l libgp -l libbmpm -L ../libbmpm -L ../libgetprim -L ../YXML_PSYQ_PORT -L ../engine -Xo$(PSX_ADDRESS)
PSX_OBJECTS = $(SOURCES:.c=.obj)
CPE = $(PSX_EXEC:.exe=.cpe)
SYM = $(PSX_EXEC:.exe=.sym)
MAP = $(PSX_EXEC:.exe=.map)


all: PSX_BUILD
	
PSX_BUILD: $(SOURCES) $(PSX_EXEC)

$(PSX_EXEC): $(CPE)
	$(PSX_CPE2X) $(CPE)
	rm -rf $(PSX_OBJECTS) $(CPE) $(SYM) $(MAP)

$(CPE): $(PSX_OBJECTS)
	$(PSX_CC) $(PSX_OBJECTS) $(PSX_LDFLAGS) -o$(CPE),$(SYM),$(MAP)
	
%.obj: %.c
	$(PSX_CC) -I $(HEADERS) $< $(PSX_CFLAGS) -o $@

clean:
	rm -f $(EXEC) $(PSX_EXEC) $(CPE) $(SYM) $(MAP) $(OBJECTS) $(PSX_OBJECTS) $(SYM) $(MAP) $(OBJECTS) $(PSX_OBJECTS)