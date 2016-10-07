# README

#### Toolkit: PSYQ

#### This setup assumes Linux using WINE, this will also work with Windows XP setup per [psxdev.net instructions](http://www.psxdev.net/help/psyq_install.html).

This directory contains example code for the PlayStation. Ordering tables are avoided in examples to keep them simple.

Toolkit is PSYQ, available at [psxdev.net](http://www.psxdev.net/downloads.html).

### Files

* psxCDcreate.md   = Create a ISO image
* psxController.md = Access Controller
* psxGraphics.md   = Graphics Info
* psxMemCard.md    = Memory Card Usage
* psxTextures.md   = Texture Loading

#### Requirements

PlayStation (WINDOWS XP SP3)

	Psy-Q SDK from psxdev.net
	MinGW
	
PlayStation (KUBUNTU 14.04)

	Psy-Q SDK from psxdev.net
	WINE
	dosemu
	BUILD_ESSENTIALS

### Library Install Directions

#### DOWNLOAD LINKS
   1. [PlayStation Dev Kit ... psxdev.net](http://www.psxdev.net/downloads.html)

   2. [Windows Reg File for PlayStation Dev Kit ... psxdev.net/forum](http://www.psxdev.net/forum/viewtopic.php?f=49&t=206)

Linux using WINE

        1. Install WINE
        2. Install dosemu 
        3. Initialize WINE (winecfg works well for this)
        4. Initialize dosemu (just run dosemu)
        5. Copy psyq directory to wine drive_c directory
        6. Use the command "wine regedit" to import reg file
        7. Edit dosemu's autoexec.bat and alter the file to mount your linux drive as root for drive D:.
           Then add psyq folder in wine as a path.
        8. Install pcsx
        9. Execute pcsx
        10. Add a bios file to pcsx
        11. Tell pcsx to use the file

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

#### DOSEMU Autoexec.bat
```
@echo off
rem autoexec.bat for DOSEMU + FreeDOS
path z:\bin;z:\gnu;z:\dosemu;D:\home\${YOUR_USERNAME}\.wine\drive_c\psyq\bin;D:\home\${YOUR_USERNAME}\.wine\drive_c\psyq\cdemu$
set HELPPATH=z:\help
set TEMP=c:\tmp
blaster
prompt $P$G
unix -s DOSDRIVE_D
if "%DOSDRIVE_D%" == "" goto nodrived
lredir del d: > nul
lredir d: linux\fs/
:nodrived
rem uncomment to load another bitmap font
rem loadhi display con=(vga,437,2)
rem mode con codepage prepare=((850) z:\cpi\ega.cpx)
rem mode con codepage select 850
rem chcp 850
lredir e: linux\fs/cdrom c
unix -s DOSEMU_VERSION
echo "Welcome to dosemu %DOSEMU_VERSION%!"
unix -e

```

### Notes
* Make sure to mark all PSYQ executables as executable (chmod +x *.EXE).