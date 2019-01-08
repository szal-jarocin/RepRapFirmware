#include "Pins_LPC.h"


//variables needed for core
//this makes the core and rrf less decoupled, but does help to save memory (and time searching arrays withing interrupts)

extern const Pin Timer1PWMPins[3] = Timer1_PWMPins;
extern const Pin Timer2PWMPins[3] = Timer2_PWMPins;
extern const Pin Timer3PWMPins[3] = Timer3_PWMPins;

extern const uint8_t TimerPWMPinsArray[MaxPinNumber] = { REP(MaxPinNumber) };

extern const uint16_t Timer1PWMFrequency = Timer1_PWM_Frequency;
extern const uint16_t Timer2PWMFrequency = Timer2_PWM_Frequency;
extern const uint16_t Timer3PWMFrequency = Timer3_PWM_Frequency;


//External Interrupt Pins
extern const Pin ExternalInterruptPins[MaxExtInterrupts] = EXTERNAL_INTERRUPT_PINS;

extern const uint8_t ExternalInterruptPinsPort0Array[32] = { EXTINTREP(32, 0) };
extern const uint8_t ExternalInterruptPinsPort2Array[32] = { EXTINTREP(32, 2) };
