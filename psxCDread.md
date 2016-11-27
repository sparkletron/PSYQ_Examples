#PSX CD Notes

#### Library: libcd.h libds.h

#### Example: textureTestCD

#### Functions

* DsInit() = Setup CDROM for operations
* DsSearchFile() = Find a file and return info to a struct of file info, this as useful things such as location and size.
* DsRead() = Read a file at a certain position, and a certain amount of sectors. Always round bytes to the next sector. Also requires a buffer to load data to, and a mode settings (speed and such).
* DsReadSync() = Check if all sectors have been read, when all have been read, this will return 0. Till then, each call tells you how many are left.
* DsClose() = Done with CDROM.
* DsPlay() = Can be used to play a certian audio track listing from any track in that list, can be set to repeat if wanted.
* DsCommand() = Issue CDROM command (such as play, pause, skip, etc).

#### Notes
* File names have to have a ";1" after them in the quotes, this is the version of the file, it is always 1.

### Example

##### Loading TIM from CD
```
DslFILE fileInfo;
int sizeSectors = 0;

u_long *image = NULL;

//CD init 
DsInit();

if(DsSearchFile(&fileInfo, "\\TIM\\YAKKO.TIM;1") <= 0)
{
  printf("\nFILE SEARCH FAILED\n");
}
else
{
  printf("\nFILE SEARCH SUCCESS\n");
}

sizeSectors = (fileInfo.size + 2047) / 2048;


printf("\nSECTOR SIZE: %d %d", sizeSectors, fileInfo.size);

image = malloc(sizeSectors * 2048);

if(image == NULL)
{
  printf("\nALLOCATION FAILED\n");
}
else
{
  printf("\nMEMORY ALLOCATED\n");
}

DsRead(&fileInfo.pos, sizeSectors, image, DslModeSpeed);

do
{
  numRemain = DsReadSync(&result);
  
  if(numRemain != prevNumRemain)
  {
    printf("\nNUM REMAIN: %d\n", numRemain);
    prevNumRemain = numRemain;
  }
}
while(numRemain);


printf("\nREAD COMPLETE\n");

DsClose();
```