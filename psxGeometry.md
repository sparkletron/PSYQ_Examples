#PSX Geometry Notes

#### Library: libgte.h

#### Example: None

### Using Geometry

This library allows you perform Geometry operations on polygons.
* Two Primary Types of Calculation
  * Coordinate Calculation, takes 3D coordinates of polygon vertices and generates 2D coordinates for the screen.
  * Light Source Calculation, finds the light source of the polygon.
* Three Coordinate Systems
  * Local, of the object
  * World, of environment the object belongs
  * Screen, of the display

##### Types of Interest
* *VECTOR, various vectors for storing different types of infromation for transformation
* DIVPOLYGON(3 or 4), Polygon made for division.
* *MESH, A collection of vertices, edges and faces that defines the shape of a poly based object.
* POL(3 or 4), polygons, the basic building block of life!
* SPOL, vertex information, used for subdiving a polygon

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