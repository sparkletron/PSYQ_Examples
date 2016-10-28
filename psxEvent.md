#PSYQ Event Notes

#### Library: libapi.lib (.h)

Events allow a programmer to create software interrupts in their code.

#### Usage

* Events can be generated with different cause descriptors
  * See page 32 of Run-Time Library Overview 4.6
  * DescUEV | m , can be used to create user defined discriptors
* System keeps track of the event in a event control block structure
* Events have status, which needs to be correct to execute.

#### Functions

* OpenEvent() , opens an event, can have a callback function or not, status is EvStWAIT
* EnableEvent(), enables an event, status becomes EvStACTIVE
* DeliverEvent(), takes descriptor and activates associated event, status becomes EvStALREADY
* UnDeliverEvent(), takes descriptor and sets state from EvStALREADY to EvStACTIVE, this must be called before reissue
* TestEvent(), same as above, but takes event class, also only does so if mode is EvMdNOINTR
* Defines are on page 32 and 33 of Run-Time Library Overview 4.6
* EnterCriticalSection(), disable interrupts 
* ExitCriticalSection(), enable interrupts
* SwEnterCriticalSection(), suppresses interrupts
* SwExitCriticalSection(), unsuppresses interrupts

#### Notes


### Example

##### Coming soon
```
//soon
```
