# PSX CD Creation

### Example: textureTestCD

### Sources: [stripiso.exe](http://www.psxdev.net/forum/viewtopic.php?f=60&t=997), [PSXLICENSE.exe](http://www.psxdev.net/forum/viewtopic.php?f=69&t=704)

## Tools
* BUILDCD.EXE = Builds a ISO image based on the CTI file, must be short paths due to it being a DOS program.
* stripiso.exe = You can use the stripiso.exe included with PSYQ or the one downloaded in the sources, I rename the download "stripisowin.exe" to avoid confusion with the DOS version.
* PSXLICENSE.EXE = Licenses a ISO so it can boot with an emulator (will not boot on a regular PSX without a modchip).

## Files
* *.CTI = Listing of files and how to layout the disc.
* *.CNF = Basic config info for the disc.

## Example Files
#### System CNF
```
BOOT=cdrom:\MAIN.EXE;1
TCB=4
EVENT=10
STACK=801FFFF0
```

#### CD CTI
```
Define LicensePath d:\tmp\LCNSFILE\
Define LicenseFile licensee.dat

Disc CDROMXA_PSX ;the disk format

	CatalogNumber 0000000000000

	LeadIn XA ;lead in track (track 0)
		Empty 300 ;defines the lead in size (min 150)
		PostGap 150 ;required gap at end of the lead in
	EndTrack ;end of the lead in track

	Track XA ;start of the XA (data) track

		Pause 150 ;required pause in first track after the lead in

		Volume ISO9660 ;define ISO 9660 volume
			
			SystemArea [LicensePath][LicenseFile]
			
			PrimaryVolume ;start point of primary volume
				SystemIdentifier "PLAYSTATION" ;required indetifier (do not change)
				VolumeIdentifier "TEXTURE" ;app specific identifiers (changeable)
				VolumeSetIdentifier "TEXTURE"
				PublisherIdentifier "SCEA"
				DataPreparerIdentifier "SONY"
				ApplicationIdentifier "PLAYSTATION"
				LPath ;path tables as specified for PlayStation
				OptionalLpath
				MPath
				OptionalMpath

				Hierarchy ;start point of root directory definition

					XAFileAttributes  Form1 Audio
					XAVideoAttributes ApplicationSpecific
					XAAudioAttributes ADPCM_C Stereo ;you can also add 'Emphasis_On' before Stereo
					
					Directory TIM
					  File YAKKO.TIM
					    XAFileAttributes Form1 Data
					    Source d:\tmp\data\TIM\YAKKO.TIM
					  EndFile
					EndDirectory

					File SYSTEM.CNF
						XAFileAttributes Form1 Data ;set the attribute to form 1
						Source d:\tmp\data\cdrom\SYSTEM.CNF ;where the file above can be found
					EndFile

					File MAIN.exe
						XAFileAttributes Form1 Data
						Source d:\tmp\data\MAIN.exe
					EndFile

				EndHierarchy ;ends the root directory definition

			EndPrimaryVolume ;ends the primary volume definition 

		EndVolume ;ends the ISO 9660 definition

		Empty 300
		PostGap 150 ;required to change the track type
	
	EndTrack ;ends the track definition (end of the XA track)

	LeadOut XA ;note that the leadout track must be the same data type as the last track (IE: AUDIO, XA or MODE1)
		Empty 150
	EndTrack

EndDisc
```
#### Bash Script
```
#!/usr/bin/env bash

echo "Creating ISO image for PlayStation Emulator"

echo "Copying files to tmp directory"

mkdir /tmp/data

cp -r ../MAIN.exe /tmp/data
cp -r ../TIM	  /tmp/data
cp -r ../cdrom    /tmp/data
cp -r ~/.wine/drive_c/psyq/cdgen/LCNSFILE /tmp

echo "Generating unlicensed image file"

echo -e 'D:\r cd "tmp/data/cdrom/"\r BUILDCD.EXE -l -iTEXTURE.IMG TEXTURE.CTI\r exitemu\r' | dosemu -dumb

echo "Generating licensed ISO file"

stripisowin.exe s 2352 /tmp/data/cdrom/TEXTURE.IMG /tmp/data/cdrom/TEXTURE.ISO

PSXLICENSE.exe /eu /i /tmp/data/cdrom/TEXTURE.ISO

echo "Moving ISO to bin_psx directory"

cp /tmp/data/cdrom/TEXTURE.ISO ../

echo "Cleaning Up files"

rm -rf /tmp/data
rm -rf /tmp/LCNSFILE
```

