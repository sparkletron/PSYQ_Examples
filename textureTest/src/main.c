/*
*/

#include <stdlib.h>
#include <libgte.h>
#include <libgpu.h>
#include <libgs.h>
#include <libpad.h>
#include <libetc.h>
#include "image.h"

#define SCREEN_WIDTH  320 // screen width
#define	SCREEN_HEIGHT 240 // screen height

u_long __ramsize   = 0x00200000; // force 2 megabytes of RAM
u_long __stacksize = 0x00004000; // force 16 kilobytes of stack

void graphics();
void display(POLY_FT4 *f4);

static u_char g_pad[2][34];

extern unsigned char image[];
GsIMAGE g_timData;

struct
{
	u_char status;
	u_char type:4;
	u_char recvSize:4;
	
	union
	{
		struct
		{
			u_char left:1;
			u_char down:1;
			u_char right:1;
			u_char up:1;
			u_char start:1;
			u_char NA1:1;
			u_char NA2:1;
			u_char select:1;
		} bit;
		u_char byte;
		
	}first;
	
	union
	{
		struct
		{
			u_char square:1;
			u_char ex:1;
			u_char circle:1;
			u_char triangle:1;
			u_char R1:1;
			u_char L1:1;
			u_char R2:1;
			u_char L2:1;
		} bit;
		u_char byte;
		
	}second;
} g_pad0;

int main() 
{
	POLY_FT4 f4;
	int prevTime = 0;
	
	graphics(); // setup the graphics (seen below)
	
	//controller
	PadInitDirect(g_pad[0], g_pad[1]);
	PadStartCom();
	
	//setup textured primitive
	SetPolyFT4(&f4);
	setRGB0(&f4, 128, 128, 128);
	
	//add image to primitive
	GsGetTimInfo((u_long *)(image+4), &g_timData);
	
	//debug info
	printf("PMODE: %d\nPX: %d\nPY: %d\nPW: %d\nPH: %d\nPIXEL: %p %d\nCX: %d\nCY: %d\nCW: %d\nCH: %d\nCLUT: %p %d\n", g_timData.pmode, g_timData.px, g_timData.py,
		g_timData.pw, g_timData.ph, g_timData.pixel, *(g_timData.pixel), g_timData.cx, g_timData.cy, g_timData.cw, g_timData.ch, g_timData.clut, *(g_timData.clut));
	
	//load texture page, and return its id to the textured primitve
	f4.tpage = LoadTPage(g_timData.pixel, g_timData.pmode, 0, g_timData.px, g_timData.py, g_timData.pw, g_timData.ph);
	
	//load clut, and return its id to the texture primitive
	f4.clut = LoadClut(g_timData.clut, g_timData.cx, g_timData.cy);
	
	//set primitive coordinates, and set its texture coordinates
	setXY4(&f4, 0, 0, 50, 0, 0, 50, 50, 50);
	setUV4(&f4, 0, 0, 50, 0, 0, 50, 50, 50);
	/*execute primitive*/
	printf("\nTEXTURE SET\n");
		
	while (1) // draw and display forever
	{
		switch((g_pad[0][2] << 8) | g_pad[0][3])
		{
			case 0xFFBF:
				if(prevTime == 0 || ((VSync(-1) - prevTime) > 60))
				{
					f4.r0 = rand() % 256;
					f4.g0 = rand() % 256;
					f4.b0 = rand() % 256;
					prevTime = VSync(-1);
				}
				break;
			case 0xEFFF:
				f4.y0 -= 1;
				f4.y1 -= 1;
				f4.y2 -= 1;
				f4.y3 -= 1;
				if(f4.y0 < 0)
				{
					f4.y0 = SCREEN_HEIGHT - 50;
					f4.y1 = SCREEN_HEIGHT - 50;
					f4.y2 = SCREEN_HEIGHT;
					f4.y3 = SCREEN_HEIGHT;
				}
				break;
			case 0xDFFF:
				f4.x0 += 1;
				f4.x1 += 1;
				f4.x2 += 1;
				f4.x3 += 1;
				if(f4.x1 > SCREEN_WIDTH)
				{
					f4.x0 = 0;
					f4.x1 = 50;
					f4.x2 = 0;
					f4.x3 = 50;
				}
				break;
			case 0xBFFF:
				f4.y0 += 1;
				f4.y1 += 1;
				f4.y2 += 1;
				f4.y3 += 1;
				if(f4.y2 > SCREEN_HEIGHT)
				{
					f4.y0 = 0;
					f4.y1 = 0;
					f4.y2 = 50;
					f4.y3 = 50;
				}
				break;
			case 0x7FFF:
				f4.x0 -= 1;
				f4.x1 -= 1;
				f4.x2 -= 1;
				f4.x3 -= 1;
				if(f4.x0 < 0)
				{
					f4.x0 = SCREEN_WIDTH - 50;
					f4.x1 = SCREEN_WIDTH;
					f4.x2 = SCREEN_WIDTH - 50;
					f4.x3 = SCREEN_WIDTH;
				}
				break;
			default:
				break;
		}
		
		display(&f4);
	}

	return 0;
}

void graphics()
{
	// within the BIOS, if the address 0xBFC7FF52 equals 'E', set it as PAL (1). Otherwise, set it as NTSC (0)
	switch(*(char *)0xbfc7ff52=='E')
	{
		case 'E':
			SetVideoMode(1); 
			break;
		default:
			SetVideoMode(0); 
			break;	
	}
	
	GsInitGraph(SCREEN_WIDTH, SCREEN_HEIGHT, GsINTER|GsOFSGPU, 1, 0); // set the graphics mode resolutions. You may also try using 'GsNONINTER' (read LIBOVR46.PDF in PSYQ/DOCS for detailed information)
	GsDefDispBuff(0, 0, 0, SCREEN_HEIGHT); // set the top left coordinates of the two buffers in video memory
}


void display(POLY_FT4 *f4)
{
	GsClearDispArea(0,0,0);
	DrawPrim(f4);
	DrawSync(0); // wait for all drawing to finish
	VSync(0); // wait for v_blank interrupt
	GsSwapDispBuff(); // flip the double buffers
}