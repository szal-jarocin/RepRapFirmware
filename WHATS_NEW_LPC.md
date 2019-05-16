Summary of LPC specific changes
===============================================

Version 3.0 beta2
=================

- New board.txt extry to define board: lpc.board. Currently supports smoothieboard, rearm TODO::: i.e. lpc.board = smoothieboard;

- Following configuration options previously located in Board.txt are now removed (they are now configurable by GCode (See new M950) and needs to be set in config.g): Fan Pins, Heater Pins, Tacho pins and Endstops
    - SpecialPins are no longer required as you now use M950 to create a "gpio port number", and that num is used in M42 and servo control
    - names are the same as written on the board silk screen of the manufacturer pinout diagram. TODO::::::::: also support 2.01 port/pin format?????????

- As part of the above changes, defaults for stepper pins are included in the firmware and wil be used if there is no entry defined in board.txt (for en/step/dir/current contro/timer pwm pinsl) then those defaults will be used. If settings present in board.txt, they will override the defaults.


- changed NO_PANELDUE from a define to a configurable option in board.txt. This new config option sets "Panel Due mode" on the AUX Serial Port (default is false). lpc.uartPanelDueMode = true; 

- Removed lpc.externalInterruptPins config. It is now done automatially when attempting to attach an interrupt to a pin on port 0 or port 2 (i.e. for Fan Tacho pin or configuring a filament detector)


-ReArm,ReArm pins are also labbeled according to the RAMPS pinout (which i figure most people would refer to) which uses ardunio naming, i.e. D8, E9, etc

