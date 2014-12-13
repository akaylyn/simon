Simon v2 Overview

The general relationship between the four micro-controllers is:

Light <- Console <-> Tower, Music

That is, Console has a unidirectional relationship with Light (I/O pins), and bidirectional 
relationships with Tower (radio) and Music (serial). 

The four micro-controllers are summarized:

Light
     Arduino Pro Mini controlling WS2812 RGB pixel strips to light rim and buttons on the console. 
     Has 4x GPIO pin connection via Console(Light).

Tower
     JeeNode controlling tower lights and fire. 
     Has RFB12b connection for two-way communication via Console(Tower).

Music
     Arduino Pro Mini controlling VS1053 music breakout. 
     Has Serial connection for two-way communication via Console(Sound).

Console
     Arduino Mega controlling the Simon implementation. 
     It is arranged around submodules, mirroring the outboard micro-controllers and internal functions:

     Button: Responsible for hard buttons on a PCB. Used for debugging purposes.
     Extern: Responsible for interfacing with other projects via RFM12b (currently: Clouds, Giles).
     Gameplay: Responsible for Simon implementation as a Finite State Machine.
     Touch: Responsible for UX input via MPR121 capsense breakout.
     Light: Responsible for UX (light) output local to the console. Coordinates outboard Light.
     Tower: Responsible for UX (light/fire) output to Towers. Coordinates outboard Tower.
     Music: Responsible for UX (sound) output. Coordinates outboard Music.


