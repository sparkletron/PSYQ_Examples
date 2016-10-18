#PSX CD Notes

#### Library: libcd.h libds.h libspu.h

#### Example: textureTestCDaudio

#### Functions

* DsInit() = Setup CDROM for operations
* DsClose() = Done with CDROM.
* DsPlay() = Can be used to play a certian audio track listing from any track in that list, can be set to repeat if wanted.
  * Has 4 modes:
    * 0 = Stop Playing
    * 1 = Play through tracks once
    * 2 = Play tracks and repeat over and over
    * 3 = Return number of current track playing
* DsCommand() = Issue CDROM command (such as play, pause, skip, etc).

#### Notes
* Track listing must be a array of track numbers that terminates in 1.
  * Aka int inArray[] = {1, 2, 3, 4, 5, 0};

### Example

##### Set Up Sound for CD Audio
```
//type for audio, in this example its located in struct p_env.
SpuCommonAttr soundAttr;

void setupSound(struct s_environment *p_env)
{ 
  //setup volume and cd into mix
  p_env->soundAttr.mask = (SPU_COMMON_MVOLL | SPU_COMMON_MVOLR | SPU_COMMON_CDVOLL | SPU_COMMON_CDVOLR | SPU_COMMON_CDMIX);
  
  p_env->soundAttr.mvol.left = 0x1FFF;
  p_env->soundAttr.mvol.right = 0x1FFF;
  
  p_env->soundAttr.cd.volume.left = 0x1FFF;
  p_env->soundAttr.cd.volume.right = 0x1FFF;
  
  p_env->soundAttr.cd.mix = SPU_ON;
  
  //set the spu attributes
  SpuSetCommonAttr(&p_env->soundAttr);
  
  SpuSetTransferMode(SPU_TRANSFER_BY_DMA);
}
```

##### Play CD Tracks
```
void playCDtracks(int *p_tracks)
{
  if(DsPlay(2, p_tracks, 0) < 0)
  {
    printf("\nNo CD Track Playing\n");
  }
  
  printf("\nCurrent Track: %d\n", DsPlay(3, p_tracks, 1));
}
```
