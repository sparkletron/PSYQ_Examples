/*
 * Started: 10/21/2016
 * By: John Convertino
 * electrobs@gmail.com
 * 
 * Library for getting primitive data in a cross platform manner. 
 * 
 * Depends on: YXML
 * 
 * Note: They only mallocs are for the s_primParam, and the texture struct inside, the data pointer inside
 * the texture struct is NOT allocated by this library.
 *
 * This library is written to parse over and over the xml, so elements do
 * not need to be in order, but missing elements can cause unforseen results.
 * Malformed files are not checked at this time (probably result in null output anyways).
 * 
 */

#ifndef GETPRIM_H
#define GETPRIM_H

#include <ENGTYP.h>

//call to initilize yxml and setup get prim data
void initGetPrimData();

//reset prim data for new prim data to be fed in (followed by setXMLdata)
int resetGetPrimData();

//free the primitive struct this library returns (will also free texture data if it is allocated).
void freePrimData(struct s_primParam **p_primParam);

//set xml data pointer to use for data extraction 
void setXMLdata(char const *p_xmlData);

//parse the data
struct s_primParam *getPrimData();


#endif // GETPRIM_H
