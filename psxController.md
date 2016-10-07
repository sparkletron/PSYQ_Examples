#PSX Controller Notes

#### Library: libpad.h
#### Example: controller

### Data Structure
Below is the way data is stored for the controller using PadInitDirect().

```css
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
```

#### To use PadInitDirect() you must for receving:

* PadInitDirect(), and pass parameters that store pad info.
  * Only port 1, port 2.
* OR you can use PadInitMtap() for multitap compatiblity.
* PadStartCom() to start communication.

#### Transmitting requires:

* PadSetAct(), registers a transmit data buffer.

#### Interesting Functions:

* PadCheckVsync(), Checks for communication with controller.
* CheckCallback(), determines if program is currently executing in normal or callback context.

### Example Code
```
static u_char g_pad[2][34];
PadInitDirect(g_pad[0], g_pad[1]);
//PadInitDirect(gpad[0], gpad[1]);
PadStartCom();
```

