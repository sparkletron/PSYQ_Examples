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
	MinGW
	
PlayStation (KUBUNTU 14.04)

	Psy-Q SDK from psxdev.net
	WINE
	BUILD_ESSENTIALS

### Library Install Directions

#### DOWNLOAD LINKS
   1. [PlayStation Dev Kit ... psxdev.net](http://www.psxdev.net/downloads.html)

   2. [Windows Reg File for PlayStation Dev Kit ... psxdev.net/forum](http://www.psxdev.net/forum/viewtopic.php?f=49&t=206)
   
   3. [PSYLIB.EXE windows version ... psxdev.net/forum](http://www.psxdev.net/forum/viewtopic.php?f=69&t=1137)
   
   4. [CPE2X.EXE windows version ... psxdev.net/forum](http://www.psxdev.net/forum/viewtopic.php?f=53&t=225)
   
   5. [mkpsxiso by Lameguy64 ... github.com](https://github.com/Lameguy64/mkpsxiso)

Linux using WINE

        1. Install WINE
        2. Initialize WINE (winecfg works well for this)
        3. Copy psyq directory to wine drive_c directory
        4. Use the command "wine regedit" to import reg file
	5. Copy psylib into psyq/bin (overwrite the old one or rename the old one)
	6. Copy CPE2X.EXE into pysq/bin, rename it to CPE2XWIN.EXE
	7. Build mkpsxiso using cmake/make (follow the readme in the repo)
	9. Copy mkpsxiso into psyq/bin
        10. Install pcsx
	11. Execute pcsx
        12. Add a bios file to pcsx
        13. Tell pcsx to use the file

### Basic Tool Setup

#### ${YOUR_USERNAME} should be replace with your user name.

#### .bashrc
```
## Paths
#psy-q psx compile tools
export PATH=/home/${YOUR_USERNAME}/.wine/drive_c/psyq/bin:$PATH
export PATH=/home/${YOUR_USERNAME}/.wine/drive_c/psyq/cdemu/BIN/:$PATH
export PATH=/home/${YOUR_USERNAME}/.wine/drive_c/psyq/bin/TIMTOOL:$PATH
```

### Notes
* Make sure to mark all PSYQ executables as executable (chmod +x *.EXE).