SOURCES = main.c
PSX_EXEC = textureTest.exe
PSX_CC = CCPSX.EXE
PSX_CPE2X = CPE2XWIN.EXE
PSX_CFLAGS = -O3 -Dpsx -c
PSX_ADDRESS = 0x80010000
PSX_LDFLAGS =  -l libpad -l libmcrd -Xo$(PSX_ADDRESS)
PSX_OBJECTS = $(SOURCES:.c=.obj)
CPE = $(PSX_EXEC:.exe=.cpe)
SYM = $(PSX_EXEC:.exe=.sym)
MAP = $(PSX_EXEC:.exe=.map)
DOSEMU = echo 'D:\r cd "$(CURDIR:/%=%)/"\r $(PSX_CPE2X) $(CPE)\r exitemu\r' | dosemu -dumb

all: PSX_BUILD
	
PSX_BUILD: $(SOURCES) $(PSX_EXEC)

$(PSX_EXEC): $(CPE)
	$(PSX_CPE2X) $(CPE)
	rm -rf $(PSX_OBJECTS) $(CPE) $(SYM) $(MAP)

$(CPE): $(PSX_OBJECTS)
	$(PSX_CC) $(PSX_OBJECTS) $(PSX_LDFLAGS) -o$(CPE),$(SYM),$(MAP)
	
%.obj: %.c
	$(PSX_CC) $< $(PSX_CFLAGS) -o $@

clean:
	rm -f $(EXEC) $(PSX_EXEC) $(CPE) $(SYM) $(MAP) $(OBJECTS) $(PSX_OBJECTS) $(SYM) $(MAP) $(OBJECTS) $(PSX_OBJECTS)