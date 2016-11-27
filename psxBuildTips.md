#PSX Build Tips

#### Example: None

### Using Build

* Make files included can help in showing how to use old PsyQ tools
* CCPSX.EXE, Compiler, windows exe
* CPE2X.EXE, CPE to EXE converter, dos exe
* CPE2XWIN.EXE, CPE to EXE converter from psxdev.net forums, win exe
* PSYLIB.EXE, takes object files a creates static library, dos exe, or use the win exe version from psxdev.net forums (from a developer disc).
* All win exe can be run with wine no problem, any dos exe must be run with dosemu or dosbox
  * Some tips in the readme.md of this project
  * sample makefile line:
    * PSX_LIBRARY_CREATE = echo 'D:\r cd "$(CURDIR:/%=%)/"\r $(PSX_AR) /a $(LIBRARY) $(PSX_OBJECTS)\r exitemu\r' | dosemu -dumb