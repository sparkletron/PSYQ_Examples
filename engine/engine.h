/*
 * Started: 10/01/2016
 * By: John Convertino
 * electrobs@gmail.com
 * 
 * Primitive engine, simple methods for getting primitives on screen, along with some other neat features.
 * 
 * Can Do:
 * 	-Memory Card Access
 * 	-Texture From CD
 * 	-Sprites, Tiles, and all polys.
 *	-Add textures to correct type 
 * 	-Game Pad
 * 
 * Can NOT Do:
 * 	-3D
 * 
 * Version: 1.0
 * 
*/

#ifndef ENGINE_H
#define ENGINE_H

#include "ENGTYP.h"

#define SCREEN_WIDTH  320 // screen width
#define	SCREEN_HEIGHT 240 // screen height

extern u_long __ramsize;  //  = 0x00200000;  force 2 megabytes of RAM
extern u_long __stacksize; // = 0x00004000; force 16 kilobytes of stack

//setup environment and set the number of primitives (sets otSize (ordering table size) to this).
void initEnv(struct s_environment *p_env, int numPrim);
//setup sound for cd
void setupSound(struct s_environment *p_env);
//play cd tracks (loops all tracks)
void playCDtracks(int *p_tracks, int trackNum);
//update display
void display(struct s_environment *p_env);
//populate textures
void populateTextures(struct s_environment *p_env);
//load a tim from CD, return address to load tim from in memory.
void *loadFileFromCD(char *p_path, uint32_t *op_len);
//get objects from xml files
struct s_primParam *getObjects(char *fileName);
//cleanup primitives
void freeObjects(struct s_primParam **p_primParam);
//call to populate the ordering table with primitives.
void populateOT(struct s_environment *p_env);
//call to update the position of primitives if it has been altered
void updatePrim(struct s_environment *p_env);
//translate current primitive
void transPrim(struct s_primParam *p_primParam, struct s_environment *p_env);
//simple move routine to keep primitives within the screen
void movPrim(struct s_environment *p_env);
//read from the memory card and return pointer to data
char *memoryCardRead(uint32_t len);
//write to the memory card using data passed to it.
void memoryCardWrite(char *p_phrase, uint32_t len);

#endif