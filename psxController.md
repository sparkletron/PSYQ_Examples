#PSX Controller Notes

#### Library: libpad.h
#### Example: controller

### Data Structure
Below is the way data is stored for the controller using PadInitDirect().

```
struct s_gamePad
{
  uint8_t status:8;

  uint8_t recvSize:4;
  uint8_t type:4;
  
  uint8_t select:1;
  uint8_t na2:1;
  uint8_t na1:1;
  uint8_t start:1;
  uint8_t up:1;
  uint8_t right:1;
  uint8_t down:1;
  uint8_t left:1;

  uint8_t l2:1;
  uint8_t r2:1;
  uint8_t l1:1;
  uint8_t r1:1;
  uint8_t triangle:1;
  uint8_t circle:1;
  uint8_t ex:1;
  uint8_t square:1;
};
```

#### bits

* 1 is button is released, 0 is button is pressed.

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


