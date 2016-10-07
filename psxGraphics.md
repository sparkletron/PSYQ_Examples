#PSX Graphics Notes

#### Library: libgs.h, libgpu.h, libgte.h

#### Example: controller, textureTest, textureTestCD

### Using Extended Graphics Libarary

#### To setup graphics:

* Check if memory address is equal to character E, if its E, set video mode to PAL, otherwise, NTSC.
  * SetVideoMode() is the function is use to set the above.
* GsInitGraph() set the graphics resolution, and various modes.
* GsDefDispBuff() set top left coordinates. Sets both buffers, display, and draw.

#### To update the display:

* GsClearDispArea(), display area is cleared using IO.
  * GetDrawArea(), GetDrawEnv()... etc looks like there are more complicated ways to draw... dispArea seems to be the wrong choice.
* Draw******() are functions that draw a primitive into the draw buffer.
* DrawSync() waits for all drawing to finish.
* VSynch() wait for v_blank interrupt
* GsSwapDispBuff() flips the double buffers.

### Using Basic Graphics Library

#### Display environment:

* SetDefDispEnv() to set fields of DISPENV struct.
* PutDispEnv() to make a display environment current.
* GetDispEnv() gets a pointer to the current display environment.

#### Draw environment:

* SetDefDrawEnv() to set fields in DRAWENV struct.
* PutDrawEnv() to make a draw environment current.
* GetDrawEnv() gets a pointer to the current drawing environment.

#### Sync and Reset:

* ResetGraph() resets the graphics system.
	* Has 3 levels:
		* Level 0: Complete reset of graphics system.
		* Level 1: Cancels current commands and clears queue.
		* Level 3: Same as 0, but the draw and display environments are preserved.
* Sync has two methods:
	* Polling:
		* DrawSync() can block till drawing queue done, or return number of positions in queue.
		* VSync() deteck the next vertical blank period.
	* Callbacks:
		* VSyncCallback() set a callback function to call at beginning of vertical blank period.
		* DrawSyncCallback() set a callback function that is run when a non-blocking drawing operation completes

#### Packet Double Buffer:

* Ordering table (OT) used as packet buffer. 
* CPU registers OT, GPU draws.
*  One OT for registering, the other for drawing, double buffer setup.
	* Example in run-time library over view makes little sense, since in loops over the register, then draws. Blocking vs non-blocking?

#### Primitive Rendering Speed

* Check for bottleneck at CPU or GPU
* DrawSync(), immediate return, bottleneck at CPU, else at GPU (measure latency of GPU).

### Examples
##### Basic code stubs to deal with graphics
```
switch(*(char *)0xbfc7ff52=='E')
{
  case 'E':
    SetVideoMode(1); 
    break;
  default:
    SetVideoMode(0); 
    break;	
}

GsInitGraph(SCREEN_WIDTH, SCREEN_HEIGHT, GsINTER|GsOFSGPU, 1, 0);
GsDefDispBuff(0, 0, 0, SCREEN_HEIGHT);

//functions to call to update display
GsClearDispArea(0,0,0);
DrawPrim(f4);
DrawSync(0); // wait for all drawing to finish
VSync(0); // wait for v_blank interrupt
GsSwapDispBuff(); // flip the double buffers
```
##### Code for switching between double buffers.
```
DRAWENV drawenv[2]; 	 	/*drawing environments*/
DISPENV dispenv[2];			/*display environments*/
int dispid = 0;    	 		/*display buffer ID*/
while (1) {
  VSync(0);			        /*wait for vertical blank*/
  dispid = (dispid + 1) %2;	/*toggle buffer ID between 0 and 1*/
  PutDrawEnv(&drawenv[dispid]);	/*switch drawing environment*/
  PutDispEnv(&dispenv[dispid]);	/*switch display environment*/
}
```
 
