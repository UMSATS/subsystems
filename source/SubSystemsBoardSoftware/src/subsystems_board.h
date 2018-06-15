#ifndef SUBSYSTEMS_BOARD_H
#define SUBSYSTEMS_BOARD_H
// UMSATS - CDH Scheduler
//
// File Description:
//   Common mappings from subsystems board to the chip.
//
//
// History
// 2018-06-14 by Tamkin Rahman
// - Created using the Subsystems Board schematic.


// -----------------------------------------------------------------------------------------------
// ----------------------- INCLUDES --------------------------------------------------------------
// -----------------------------------------------------------------------------------------------
#include "compiler.h"

// -----------------------------------------------------------------------------------------------
// ----------------------- DEFINES ---------------------------------------------------------------
// -----------------------------------------------------------------------------------------------
// OSCILLATOR RELATED DEFINITIONS
#ifdef AVR32_SCIF_101_H_INCLUDED
#define AVR32_SCIF_OSCCTRL0_STARTUP_2048_RCOSC  0x00000003
#define AVR32_SCIF_OSCCTRL0_STARTUP_16384_RCOSC 0x00000006
#define AVR32_SCIF_OSCCTRL32_STARTUP_8192_RCOSC 0x00000002
#endif

// This is mounted by default.
#define FOSC32          AVR32_SCIF_OSC32_FREQUENCY              //!< Osc32 frequency: Hz.
#define OSC32_STARTUP   AVR32_SCIF_OSCCTRL32_STARTUP_8192_RCOSC //!< Osc32 startup time: RCOsc periods.

// The main oscillator for the subsystems board.
#define FOSC0           16000000                                //!< Osc0 frequency: Hz.
#define OSC0_STARTUP    AVR32_SCIF_OSCCTRL0_STARTUP_2048_RCOSC  //!< Osc0 startup time: RCOsc periods.

#define BOARD_OSC0_HZ           16000000
#define BOARD_OSC0_STARTUP_US   2000
#define BOARD_OSC0_IS_XTAL      true
#define BOARD_OSC32_HZ          32768
#define BOARD_OSC32_STARTUP_US  71000
#define BOARD_OSC32_IS_XTAL     true

// ADC
#define ADC1 AVR32_PIN_PA04
#define ADC2 AVR32_PIN_PA05
#define ADC3 AVR32_PIN_PA06
#define ADC4 AVR32_PIN_PA07
#define ADC5 AVR32_PIN_PA08
#define ADC6 AVR32_PIN_PA09

// GPIO
#define GPIO1 AVR32_PIN_PA16
#define GPIO2 AVR32_PIN_PA19
#define GPIO3 AVR32_PIN_PA20
#define GPIO4 AVR32_PIN_PA21
#define GPIO5 AVR32_PIN_PA22
#define GPIO6 AVR32_PIN_PA23

// LEDS
#define DBG_LED_1 GPIO4
#define DBG_LED_2 GPIO5
#define DBG_LED_3 GPIO6

#endif /* SUBSYSTEMS_BOARD_H */
