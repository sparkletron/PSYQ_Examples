#PSX Controller Notes

#### Library: libpad.h
#### Example: controller

### Data Structure
Below is the way data is stored for the controller using PadInitDirect().

```
struct s_gamePad
{
  union
  {
    struct
    {
      u_char one:1;
      u_char two:1;
      u_char three:1;
      u_char four:1;
      u_char five:1;
      u_char six:1;
      u_char seven:1;
      u_char eight:1;
    } bit;
    struct
    {
      u_char status;
    }byte;
 
  } first;
  
  union
  {
    struct
    {
      u_char recvSize:4;
      u_char type:4;
    } nibble;
    u_char byte;
    
  } second;
  
  union
  {
    struct
    {
      u_char select:1;
      u_char NA2:1;
      u_char NA1:1;
      u_char start:1;
      u_char up:1;
      u_char right:1;
      u_char down:1;
      u_char left:1;
    } bit;
    u_char byte;
	  
  } third;
  
  union
  {
    struct
    {
      u_char L2:1;
      u_char R2:1;
      u_char L1:1;
      u_char R1:1;
      u_char triangle:1;
      u_char circle:1;
      u_char ex:1;
      u_char square:1;
    } bit;
    u_char byte;
	  
  } fourth;
};
```

#### To use PadInitDirect() you must for receving:

* PadInitDirect(), and pass parameters that store pad info.
  * Only port 1, port 2. must be case to a u_char pointer if using the struct above.
* OR you can use PadInitMtap() for multitap compatiblity.
* PadStartCom() to start communication.

#### Callback

* Can setup a callback to deal with pad buffer, but there is no controller callback.
  * Attach to vsync, draw or some othe callback, if multiple are attached, use callback list.

#### Transmitting requires:

* PadSetAct(), registers a transmit data buffer.

#### Interesting Functions:

* PadCheckVsync(), Checks for communication with controller.
* CheckCallback(), determines if program is currently executing in normal or callback context.

### Example Code

#### Generic Buffer
```
static u_char g_pad[2][34];
PadInitDirect(g_pad[0], g_pad[1]);
PadStartCom();
```
#### Struct Buffer
```
struct s_gamePad gamePad[2];
PadInitDirect((u_char *)&gamePad[0], (u_char *)gamePad[1]);
PadStartCom();
```


