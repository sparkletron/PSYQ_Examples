#PSX XML notes

#### Library: yxml.h (PSYQ_YXML_PORT), libgp (libgetprim)

#### Example: all examples use xml parsing to get object data

### Using yxml

YXML can be used to make custom routines to parse the xml data and return it.
* Does not use malloc
* Stack is the buffer space for file, only allocate as large as needed.

### Using libgp

libgp (libgetprim) is a xml parsing library that allows you to parse xml files for information.

This library depends on YXML.

This information is used to create primitives and has the following methods available:

#### Functions
* initGetPrimData(), setup libgetprim for the first time (only call once)
* resetGetPrimData(), reset yxml stack and data to start a new document (may or maynot be needed)
* freePrimData(), free data from the primitive struct returned by getPrimData();
* setXMLdata(char *), pass the xml data to be used for parsing (can be changed whenever needed).
* getPrimData(), using data set by setXMLdata(), this will parse the data, allocate a struct and return it to the caller.

### Examples

#### libgp (LIBGETPRIM) xml format
```xml
<ACTOR_PRIM type="TYPE_F4">
  <vertex0>
    <x>50</x>
    <y>50</y>
  </vertex0>
  <vertex1>
    <x>150</x>
    <y>50</y>
  </vertex1>
  <vertex2>
    <x>150</x>
    <y>75</y>
  </vertex2>
  <vertex3>
    <x>50</x>
    <y>75</y>
  </vertex3>
  <color0>
    <red>255</red>
    <green>0</green>
    <blue>255</blue>
  </color0>
  <color1>
    <red>255</red>
    <green>0</green>
    <blue>255</blue>
  </color1>
  <color2>
    <red>255</red>
    <green>0</green>
    <blue>255</blue>
  </color2>
  <color3>
    <red>255</red>
    <green>0</green>
    <blue>255</blue>
  </color3>
  <width>50</width>
  <height>50</height>
  <texture>
    <vertex0>
      <x>0</x>
      <y>0</y>
    <vertex0>
    <vramVertex>
      <x>320</x>
      <y>0</y>
    </vramVertex>
    <twidth>50</twidth>
    <theight>50</theight>
    <file>\\YAKKO.BMP;1</file>
  </texture>
</ACTOR_PRIM>
```

#### Main parse loop for YXML
```
do {
  returnValue = yxml_parse(&data, *p_buffer);
    
  switch(returnValue)
    {
      case YXML_ELEMSTART:
	strcpy(elem, data.elem);
	index = 0;
	break;
      case YXML_ATTRSTART:
	strcpy(attr, data.attr);
	break;
      case YXML_ATTRVAL:
	if(data.data[0] != '\n')
	{
	  attrValue[index] = data.data[0];
	  index++;
	  index = index % 256;
	}
	break;
      case YXML_CONTENT:
	if(data.data[0] != '\n')
	{
	  string[index] = data.data[0];
	  //printf("CONTENT: %c\n", data.data[0]);
	  index++;
	  index = index % 256;
	}
	break;
      case YXML_ATTREND:
	printf("ATTR: %s, %s\n", attr, attrValue);
	index = 0;
	memset(attr, 0, 256);
	memset(string, 0, 256);
	break;
      case YXML_ELEMEND:
	if(*(p_buffer + 1) != 0)
	{
	  printf("ELEM: %s, %s\n", elem, string);
	  index = 0;
	  memset(elem, 0, 256);
	  memset(string, 0, 256);
	}
      default:
	break;
    }
    
    p_buffer++;
  } while(*p_buffer && returnValue >= 0);
```
