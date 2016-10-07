#PSX Memory Card Notes

#### Example: memcardReader, memcardWrite

### Specifications

* Capacity, 120 Kbytes in 128-byte sectors
* Synchronous serial communication
* No access for 20 ms after reading 1 sector
* Max continuous reading speed is 10 Kbyte/sec
* basic library, low level library.

### Base Memory Card Library

#### Library: libcard.h

#### Functions

* InitCARD(), Initializes the memory card bios and enters idle state. Can set if shared with controller or not.
* StartCARD(), Changes memory Card BIOS to run state (initCARD must be called first!).
* StopCARD(), Sets memory card BIOS to idle state. Stops game pad as well.
* _bu_init(), Initialize filesystem, as this does not happen automatically (unformatted?, initCARD must be called first).
* _card_format(), format memory card at specified port.
* _card_info(), get card info at specified port.
* _card_load(), Reads file management info, at a specified port, must be called once before using open().
* _card_read(), read one block from the memory card (128 bytes, one sector), at a specified port.
* _card_write(), same as above, except it writes a block.

### EXTENDED Memory Card Library

#### Library: libmcrd.h

* High-Level library for the memory card
	* Can check for the memory card.
	* Red, write, delete and create files.
	* Directory Info
	* Formatting.
	* Check state.

##### Functions

* MemCardAccept(), Test connection, if connected gets more info. Must be called first, and only again if a card is swapped
* MemCardClose(), Closes file that was opened by MemCardOpen.
* MemCardCreateFile(), Creates a file on the memory card, with a specified name.
* MemCardDeleteFile(), Deletes file of a specific name.
* MemCardEnd(), Terminates system, MemCardStop must be called first is MemCardStart has been called.
* MemCardExist(), Checks for the presence of a card.
* MemCardFormat(), Format a memory card (full format).
* MemCardGetDirentry(), Finds files in a specified directory
* MemCardInit(), Initializes memory card system, must be called first after InitPAD,GUN,TAP and not called twice.
* MemCardOpen(), Opens a file from a specified memory card.
* MemCardReadData(),  Reads data from file setup by open, from the memory card in 128 byte chunks (bytes must be a multiple of 128).
* MemCardReadFile(), Reads data from a specified memory card file. Same byte requirement as above.
* MemCardStart(), places memory card system in active state after MemoryCardInit has been called.
* MemCardStop(), stops memory card system started my MemCardStart().
* MemCardSync(), Sync memory card access by waiting for termination of asynchronous functions calls.
* MemCardWriteData(), Writes data to a file setup by open, to the memory card in 128 byte chunks (bytes must be a multiple of 128).
* MemCardWriteFile(), Write data to a specified memory card file. Same byte requirement as above.

### Example

#### Writing to a Memory Card

```
MemCardInit(1);
	
MemCardStart();

if(MemCardSync(0, &cmds, &result) <= 0)
{
  FntPrint("START WTF\n\n");
}

MemCardAccept(0);

if(MemCardSync(0, &cmds, &result) <= 0)
{
  FntPrint("ACCEPT WTF\n\n");
}

if(MemCardCreateFile(0, "test", 1) != 0)
{
  FntPrint("FILE CREATE WTF\n\n");
}

if(MemCardOpen(0, "test", O_WRONLY) != 0)
{
  FntPrint("Open WTF\n\n");
}

if(MemCardWriteData((unsigned long *)&(*phrase), 0, 128)  != 0)
{
  FntPrint("Write WTF\n\n");
}

if(MemCardSync(0, &cmds, &result) <= 0)
{
  FntPrint("WRITE WTF\n\n");
}

MemCardClose();

MemCardStop();
```

#### Reading from a Memory Card
```
MemCardInit(1);

MemCardStart();

if(MemCardSync(0, &cmds, &result) <= 0)
{
  FntPrint("START WTF\n\n");
}

MemCardAccept(0);

if(MemCardSync(0, &cmds, &result) <= 0)
{
  FntPrint("ACCEPT WTF\n\n");
}

if(MemCardCreateFile(0, "test", 1) != 0)
{
  FntPrint("FILE CREATE WTF\n\n");
}

if(MemCardOpen(0, "test", O_WRONLY) != 0)
{
  FntPrint("Open WTF\n\n");
}

if(MemCardWriteData((unsigned long *)&(*phrase), 0, 128)  != 0)
{
  FntPrint("Write WTF\n\n");
}

if(MemCardSync(0, &cmds, &result) <= 0)
{
  FntPrint("WRITE WTF\n\n");
}

MemCardClose();

MemCardStop();
```