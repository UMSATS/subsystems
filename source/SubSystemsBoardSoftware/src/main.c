// UMSATS - CDH Scheduler
//
// File Description:
// Created using a trimmed version of the FREERTOS_UC3_EXAMPLE1 from Atmel Studio.
//
// Added ASF CDC USB studio module.
//
// History
// 2018-06-12 by Tamkin Rahman
// - Created.


//----------------------------------------------------------------------------
//	INCLUDES
//----------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Environment header files. */
#include "power_clocks_lib.h"
#include "gpio.h"
#include "sysclk.h"
#include "stdio_usb.h"

/* Scheduler header files. */
#include "Generic_FreeRTOS.h"

#include "CANManager.h"
#include "CDHSchedulerMain.h"
#include "SerialPrint.h"

//----------------------------------------------------------------------------
//	DEFINES
//----------------------------------------------------------------------------
#define ERR_LED GPIO1

//----------------------------------------------------------------------------
//	FUNCTION PROTOTYPES
//----------------------------------------------------------------------------
// CDH Implementation of getchar due to a known bug in Atmel's USB CDC stack.
static char cdh_getchar(char default_result);

// CDH Implementation of printf due to a known bug in Atmel's USB CDC stack.
static void cdh_printf(const char *theString);

// CDH Implementation of a "print int" method to print integers.
static void cdh_printInt(int print_value);

// Blinking LED task for the dev board.
static void vBlinking( void *pvParameters );

int schedulerStarted = 0;

//----------------------------------------------------------------------------
//	FUNCTIONS
//----------------------------------------------------------------------------
int main( void )
{
	// Use the subsystems external oscillator. 
	// If the oscillator used is changed, make sure to update FreeRTOSConfig.h.
	pcl_switch_to_osc(PCL_OSC0, FOSC0, OSC0_STARTUP);

	// Call a local utility routine to initialize C-Library Standard I/O over
	// a USB CDC protocol. Tunable parameters in a conf_usb.h file must be
	// supplied to configure the USB device correctly.
	//
	// Interrupts are configured when the scheduler is started (i.e. don't need to
	// configure them here). STDIO (e.g. printf, scanf) cannot be used until the
	// scheduler has started.
	stdio_usb_init(); 

	// A blinking LED task so that we can plainly see the scheduler running.
	xTaskCreate(vBlinking, (const signed char *) "Blinking", 256, NULL, 2, NULL);
	
	// Initialize the gpio.
	gpio_local_init();
	gpio_enable_gpio_pin(DBG_LED_1);
	gpio_enable_gpio_pin(DBG_LED_2);
	gpio_enable_gpio_pin(DBG_LED_3);
	gpio_enable_gpio_pin(ERR_LED);
			
	gpio_set_pin_high(DBG_LED_1);
	gpio_set_pin_high(DBG_LED_2);
	gpio_set_pin_high(DBG_LED_3);
	gpio_set_pin_high(ERR_LED);
			
	gpio_configure_pin(DBG_LED_1, GPIO_DIR_OUTPUT);
	gpio_configure_pin(DBG_LED_2, GPIO_DIR_OUTPUT);
	gpio_configure_pin(DBG_LED_3, GPIO_DIR_OUTPUT);
	gpio_configure_pin(ERR_LED, GPIO_DIR_OUTPUT);
			
	// Initialise the scheduler.
	SchedulerSetup();
	schedulerStarted = 1;
	
	// Start the FreeRTOS scheduler.
	vTaskStartScheduler();
	
	// Will get here if there's insufficient RAM.
	printf("ERROR: Insufficient RAM.");
	while(1){}

	return 0;
}
/*-----------------------------------------------------------*/

int SerialRead(int default_value)
{
	int result = default_value;
	
	if (schedulerStarted)
	{
		WaitForSemaphore( scanLock );
		result = cdh_getchar(default_value);	
		xSemaphoreGive( scanLock );
	}
	
	return result;
}

// --------------------------------------------------------------------------------
void SerialPrint(const char * text)
{
	if (schedulerStarted)
	{
		WaitForSemaphore( printLock );
		cdh_printf(text);
		xSemaphoreGive( printLock );
	}
}

// --------------------------------------------------------------------------------
void SerialPrintInt(int text)
{
	if (schedulerStarted)
	{
		WaitForSemaphore( printLock );
		cdh_printInt(text);
		xSemaphoreGive( printLock );
	}
}

// --------------------------------------------------------------------------------
// Stub until we get CAN working.
void CANMonitor(void *pvParameters)
{
	UNUSED(pvParameters);
	
	while (1)
	{
		vTaskDelay(2000000);
	}
}

// --------------------------------------------------------------------------------
static void vBlinking( void *pvParameters )
{
		while(1)
		{
			gpio_set_pin_low(DBG_LED_1);
			gpio_set_pin_low(DBG_LED_2);
			gpio_set_pin_low(DBG_LED_3);
			gpio_set_pin_low(ERR_LED);
			vTaskDelay(pdMS_TO_TICKS(1000));
			
			gpio_set_pin_high(DBG_LED_2);
			gpio_set_pin_high(DBG_LED_3);
			gpio_set_pin_high(ERR_LED);
			vTaskDelay(pdMS_TO_TICKS(1000));
		}
}

// --------------------------------------------------------------------------------
static void cdh_printf(const char *theString)
{
	char buf[100];
	int s=0;
	
	strcpy(buf, theString);
	
	while (buf[s] != '\0')
	{
		if( udi_cdc_is_tx_ready() )	
		{
			stdio_usb_putchar(NULL, buf[s]);
		}
		s++;
	}
	//uart_usb_flush();
}

// --------------------------------------------------------------------------------
static void cdh_printInt(int print_value)
{
	char buf[15];
	sprintf(buf, "%d", print_value);
	
	cdh_printf(buf);
}

// --------------------------------------------------------------------------------
static char cdh_getchar(char default_result)
{
	char rc = default_result;
	
	if (udi_cdc_is_rx_ready()) 
	{
		stdio_usb_getchar(NULL, &rc);
	}
	
	return rc;
}
