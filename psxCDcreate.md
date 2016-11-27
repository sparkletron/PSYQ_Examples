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

## NEW METHOD OF CREATING ISO (CDDA TRACKS WORK)

* mkpsxiso, uses an xml file to dictate the build
  * Adds licences as well (location must be specified).
  * Creates bin and cue.
* Simply download, compile, and run mkpsxiso your.xml and it will generate your CD image.
* mkpsxiso seems to use the exact path you use for placement as the path in the cue. This can cause issues
  and you may have to change by hand (till I fix this and submit an update).

[Forked Repo for Linux/Windows build with cmake](https://github.com/electroCupcake/mkpsxiso)

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

#### XML Example
```
<?xml version="1.0" encoding="UTF-8"?>

<!-- MKPSXISO example XML script -->

<!-- <iso_project>
		Starts an ISO image project to build. Multiple <iso_project> elements may be
		specified within the same xml script which useful for multi-disc projects.
	
		<iso_project> elements must contain at least one <track> element.
	
	Attributes:
		image_name	- File name of the ISO image file to generate.
		cue_sheet	- Optional, file name of the cue sheet for the image file
					  (required if more than one track is specified).
-->
<iso_project image_name="myimage.bin" cue_sheet="myimage.cue">

	<!-- <track>
			Specifies a track to the ISO project. This example element creates a data
			track for storing data files and CD-XA/STR streams.
		
			Only one data track is allowed and data tracks must only be specified as the
			first track in the ISO image and cannot	be specified after an audio track.
		
		Attributes:
			type		- Track type (either data or audio).
			source		- For audio tracks only, specifies the file name of a wav audio
						  file to use for the audio track.
			
	-->
	<track type="data">
	
		<!-- <identifiers>
				Optional, Specifies the identifier strings to use for the data track.
				
			Attributes:
				system			- Optional, specifies the system identifier (PLAYSTATION if unspecified).
				application		- Optional, specifies the application identifier (PLAYSTATION if unspecified).
				volume			- Optional, specifies the volume identifier.
				volume_set		- Optional, specifies the volume set identifier.
				publisher		- Optional, specifies the publisher identifier.
				data_preparer	- Optional, specifies the data preparer identifier. If unspecified, MKPSXISO
								  will fill it with lengthy text telling that the image file was generated
								  using MKPSXISO.
		-->
		<identifiers
			system			="PLAYSTATION"
			application		="PLAYSTATION"
			volume			="MYDISC"
			volume_set		="MYDISC"
			publisher		="MYPUBLISHER"
			data_preparer	="MKPSXISO"
		/>
		
		<!-- <license>
				Optional, specifies the license file to use, the format of the license file must be in
				raw 2336 byte sector format, like the ones included with the PsyQ SDK in psyq\cdgen\LCNSFILE.
				
				License data is not included within the MKPSXISO program to avoid possible legal problems
				in the open source environment... Better be safe than sorry.
				
			Attributes:
				file	- Specifies the license file to inject into the ISO image.
		-->
		<license file="licensea.dat"/>
		
		<!-- <directory_tree>
				Specifies and contains the directory structure for the data track.
			
			Attributes:
				None.
		-->
		<directory_tree>
		
			<!-- <file>
					Specifies a file in the directory tree.
					
				Attributes:
					name	- File name to use in the directory tree (can be used for renaming).
					type	- Optional, type of file (data for regular files and is the default, xa for
							  XA audio and str for MDEC video).
					source	- File name of the source file.
			-->
			<!-- Stores system.txt as system.cnf -->
			<file name="system.cnf"	type="data"	source="system.txt"/>
			<file name="myexec.exe"	type="data"	source="myexec.exe"/>
			
			<!-- <dir>
					Specifies a directory in the directory tree. <file> and <dir> elements inside the element
					will be inside the specified directory.
			-->
			<dir name="mydir">
			
				<!-- All <file> and <dir> elements here will be inside mydir -->
				
				<file name="myfile.dat" type="data" source="myfile.dat"/>
				
				<!-- Specifies a directory inside mydir -->
				<dir name="mystreams">
				
					<!-- Specifies a file as XA audio -->
					<file name="mymusic.xa" type="xa" source="mymusic.xa"/>
					<!-- Specifies a file as STR MDEC video -->
					<file name="mymovie.str" type="str" source="mymovie.str"/>

				</dir>
			
			</dir>
		
		</directory_tree>
		
	</track>

	<!-- Specifies CD-DA audio tracks
	
		WAV files must be of Microsoft format, uncompressed, 44100KHz 16-bit stereo.
	-->
	<track type="audio" source="mytrack2.wav"/>
	<track type="audio" source="mytrack3.wav"/>
	
</iso_project>
```

