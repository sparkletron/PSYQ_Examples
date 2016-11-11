#PSX Geometry Notes

#### Library: libgte.h

#### Example: controllerTest

### Using Geometry

This library allows you perform Geometry operations on polygons.
* Two Primary Types of Calculation
  * Coordinate Calculation, takes 3D coordinates of polygon vertices and generates 2D coordinates for the screen.
  * Light Source Calculation, finds the light source of the polygon.
* Three Coordinate Systems
  * Local, of the object
  * World, of environment the object belongs
  * Screen, of the display
* ordering table needs to coorispond to z value.
* OTZ is larger for distant objects, as you get closer, the value approaches 0.
  *ordering table is drawn in reverse, so last value is on top.

##### Functions
* InitGeom(), Must be called first before doing any geometery operations  
* RotMatrix... Many different types that creates a rotation matrix for the object based on its rotation angle (can be a vector)
* ScaleMatrix(), Take a rotation matrix and scale it by a vector, the vector must set 1.0 as 4096.
* TransMatrix(), Takes a matrix and a standard vector to set the amount of parallel transfer.
* SetRotMatrix(), Set the constant rotate matrix to be used for rotation (must be set, or else all will be 0 (4096 is ONE, homogeneous?))
* SetTransMatrix(), Set the const parallel transfer vector from a matrix.
* SetGeomScreen(), Sets the distance from the visual point (eye) to the screen. (distance h).
* RotTrans(), Many functions for using the constant matrix to do a rotate and translation applied to a vector or other object. (page 509 of Run-Time Reference).
  * some return otz, divided by 4, so you can use a otz with a 1/4 of the dynamic range.

##### Other relatable functions
* ClearOTagR(), use to reverse ordering table and base it on z values.
  
##### Types of Interest
* MATRIX, used for matrix transforms
* *VECTOR, various vectors for storing different types of infromation for transformation
* DIVPOLYGON(3 or 4), Polygon made for division.
* *MESH, A collection of vertices, edges and faces that defines the shape of a poly based object.
* POL(3 or 4), polygons, the basic building block of life!
* SPOL, vertex information, used for subdiving a polygon

##### Notes
* Setting SetGeomScreen to 1024, then setting all z vertexs (transform, and object vertexs) to half that results in the correct size.
  * this is due to the math involved, if you set the z vertex to 512, then you add a transform of 512, the result is 1024.
    This puts you at 0, as defined by the SetGeomScreen, so now any addition will bring you down, (z increases distance away from 1024).
* Adding to Z sends it farther back. 
* Camera on object also changes its coordinates at the moment, so an increase in z, decreases the area the object has to move (in relation to its origin).
* Interpolation coefficient p, is used for interpolating colors. Seems like are max Z value is 4096?
* According to page 142 of the Run-Time overview, "polygons deeper than P=4096 do no merge into the background, they are not drawn".
* Also on page 452 of the Run-Time Reference guide, the return value of otz2p, which gets the interpolation value from p, can only be 0 to 4096.
* otz value is always a quarted of the sz value (ordering table compression).

### Examples

##### From page 101 of PSYQ Doc, this does a perspective transformation
```
void rotTransPersAddPrim(SVECTOR *pos, SPRT *sp, unsigned long *ot;
{
  long otz, dmy, flg;
  
  otz = RotTransPers(&( pppos->x[0], (long*)sp ->x0, &dmy, &flg);
  
  if (otz > 0 && otz < ot_size)
  {
    AddPrim(ot+otz, sp);
  }
}
```