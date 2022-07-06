# README


#### Toolkit: PSYQ

#### This setup assumes Linux using WINE, this will also work with Windows XP setup per [psxdev.net instructions](http://www.psxdev.net/help/psyq_install.html).

This directory contains example code for the PlayStation. Ordering tables are avoided in examples to keep them simple.

Toolkit is PSYQ, available at [psxdev.net](http://www.psxdev.net/downloads.html).

### Files

* psxCDaudio.md    = Play Audio Track (CDDA)
* psxCDcreate.md   = Create a ISO image
* psxCDread.md     = Read Data from a cd
* psxController.md = Access Controller
* psxGeometry.md   = Depth sorting, Polygons
* psxGraphics.md   = Graphics Info
* psxMemCard.md    = Memory Card Usage
* psxTextures.md   = Texture Loading
* psxBuildTips.md  = Build Tips
* psxXML.md	   = YXML/LIBGETPRIM for the playstation
* psxBitmapManip   = convert bitmaps to raw data and convert it from ARGB to ABGR or back again.
* psxEvent.md	   = usage of event handler in psyq

#### Requirements

PlayStation (WINDOWS XP SP3)

	Psy-Q SDK from psxdev.net
	Windows Reg File
	PSYLIB win version
	CPE2X win version
	mkpsxiso
	STDINT.h
	MinGW
	
PlayStation (KUBUNTU ~~14.04~~ 22.04)

	Psy-Q SDK from psxdev.net
	Windows Reg File
	PSYLIB win version
	CPE2X win version
	mkpsxiso
	STDINT.h
	WINE
	WINE-BINFMT
	BUILD_ESSENTIALS

### Library Install Directions

#### DOWNLOAD LINKS
   1. [PlayStation Dev Kit ... psxdev.net](http://www.psxdev.net/downloads.html)

   2. [Windows Reg File for PlayStation Dev Kit ... psxdev.net/forum](http://www.psxdev.net/forum/viewtopic.php?f=49&t=206)
   
   3. [PSYLIB.EXE windows version ... psxdev.net/forum](http://www.psxdev.net/forum/viewtopic.php?f=69&t=1137)
   
   4. [CPE2X.EXE windows version ... psxdev.net/forum](http://www.psxdev.net/forum/viewtopic.php?f=53&t=225)
   
   5. [mkpsxiso by Lameguy64 ... github.com](https://github.com/Lameguy64/mkpsxiso)
   
   6. [STDINT.h by John Convertino ... github.com](https://github.com/electroCupcake/PSYQ_Examples/tree/master/PSYQ_MODS)

Linux using WINE

	1. Install WINE
	2. Install wine-binfmt
	3. Initialize WINE (winecfg works well for this)
	4. Copy psyq directory to wine drive_c directory
	5. Use the command "wine regedit" to import reg file
	6. Copy psylib into psyq/bin (overwrite the old one or rename the old one)
	7. Copy CPE2X.EXE into pysq/bin, rename it to CPE2XWIN.EXE
	8. Build mkpsxiso using cmake/make (follow the readme in the repo)
	9. Copy mkpsxiso into psyq/bin
	10. From the PSY_MODS folder in this repository, copy STDINT.h to psyq/include
	11. Add psyq paths to your .bashrc, paths are below.
	12. make will now build the executables.
	13. mkpsxiso will build the ISO image
	    * You will have to change the license path, since it is included in pysq under your user name.
	    * Edit with any normal text editior

### Basic Tool Setup

#### .bashrc
```
## Paths
#psy-q psx compile tools
export PATH=/home/$USER/.wine/drive_c/psyq/bin:$PATH
export PATH=/home/$USER/.wine/drive_c/psyq/cdemu/BIN/:$PATH
export PATH=/home/$USER/.wine/drive_c/psyq/bin/TIMTOOL:$PATH
```

### Notes
* Make sure to mark all PSYQ executables as executable (chmod +x *.EXE).
