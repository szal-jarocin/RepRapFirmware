Summary of LPC specific changes
===============================================

Version 3.0 beta3
=================


### Board Config (board.txt)

* Removed Fan Pins, Heater Pins, Tacho pins and Endstops configuration options as they are now configurable by GCode (See GCode M950) and needs to be set in config.g (See https://duet3d.dozuki.com/Wiki/RepRapFirmware_3_overview).
* Removed SpecialPins as it's no longer required in RRF3. M950 is now used to create a "gpio port number", and that gpio port number is used in M42 and M280.
* Removed lpc.externalInterruptPins config. It is now done automatially when attempting to attach an interrupt to a pin on port 0 or port 2 (i.e. for Fan Tacho pin or configuring a filament detector).
* Removed lpc.slowPWM.pins, lpc.fastPWM.pins and lpc.servoPins. They are now assigned automatically. When using M950 ensure the Frequency matches either the HWPWM frequency or the Timer Frequencies
* Added new entry: lpc.HWPWM.frequencyHz. To allow setting HW PWM to a different frequency. Default is 250.
* Added new entry: lpc.board. Currently supported values are: smoothieboard, rearm,  mkssbase, azsmzmini, biquskr_1.1, biquskr_1.3 and generic.  For example: lpc.board = smoothieboard;
    * Currently does not support control of drivers via UART/SPI on Biqu SKR 1.3. 
    * When defining a board, the names on the silk screen (or official pinout if there is no names on the silk) can be used in M950. LPC style of port.pin (i.e. 1.23) can also be used if desired, however.
    * For ReArm, the silk on the RAMPs shield is used and additionally support from the official pinout that uses arduino mega naming can be used, i.e. D8, etc.
    * For the above boards (except generic), some defaults are included in the firmware for stepper pins (en/step/dir), current control and will be used as the default. They can be overriden by defining those variables in board.txt.
    * The generic board has no defaults. 
* Added new option do enable/disable RRF Panel Due mode for the AUX serial port (default is false). For example: lpc.uartPanelDueMode = true; 
* Added new atxPowerPinInverted entry which is set to true or false.  Set to true if the PSON needs to be inverted. Default is False.

### Networking

* Fixed a bug causing a buffer to not be freed after a read socket error.
* Added mutexes to the callbacks which are called from a different task (IPTask).
* Upgraded FreeRTOS+TCP to V2.0.11
* Added extra FreeRTOS priorty and changed priories so the EMac task has a higher priority than IPTask.   
* Removed the f_expand to preallocate space as it takes longer than the default network time out when uploading large files.

### SDCard

* Updated functions for SDCard and SharedSPI to improve performance now using ChaN 2018 routines. Includes 16bit block transfers and takes advantage of the LPC SSP 8 frame FIFOs. 
* Added detection for SDCards that support HighSpeed mode which enables the card interface SPI speeds up to 50MHz.
* Added check to ensure that if user enters a higher SPI frequency than the card can support to only use the max card speed.
* If users select a slower SPI speed than the card can handle, that speed will instead be used (i.e. to have slower SPI speeds over long cables for external SDCard slot if desired).
* Implemented SDCard read/write retries as RRF does.
