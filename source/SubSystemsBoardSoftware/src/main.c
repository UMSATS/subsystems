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
// 2018-06-16 by Tamkin Rahman
// - Added (rudimentary) CAN message receiver and transmitter.


//----------------------------------------------------------------------------
//	INCLUDES
//----------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Environment header files. */
#include "can.h"
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

// CAN initialization routine. Should be called after the oscillator is set up.
static char cdh_can_init();

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

	cdh_can_init();

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
		char buf[100];
		int s=0;
			
		strcpy(buf, text);
			
		while (buf[s] != '\0')
		{
			if( udi_cdc_is_tx_ready() )
			{
				stdio_usb_putchar(NULL, buf[s]);
			}
			s++;
		}
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

// --------------------------------------------------------------------------------
// Rx can message.
can_msg_t rx_msg =
{
	0,                // Identifier
	0,                // Mask
	0x0LL,            // Data
};

// Rx Message object.
can_mob_t rx_mob =
{
	CAN_MOB_NOT_ALLOCATED, 			// Handle: by default CAN_MOB_NOT_ALLOCATED
	&rx_msg,	   					// Pointer on CAN Message
	8,		                		// Data length DLC
	CAN_DATA_FRAME,        	        // Request type : CAN_DATA_FRAME or CAN_REMOTE_FRAME
	CAN_STATUS_NOT_COMPLETED	    // Status: by default CAN_STATUS_NOT_COMPLETED
};
can_msg_t tx_msg =
{
	0,                // Identifier
	0,                // Mask
	0x0LL,            // Data
};

// Tx Message object.
can_mob_t tx_mob = 
{
	CAN_MOB_NOT_ALLOCATED, 			// Handle: by default CAN_MOB_NOT_ALLOCATED
	&tx_msg,     					// Pointer on CAN Message
	8,		                		// Data length DLC
	CAN_DATA_FRAME,        	        // Request type : CAN_DATA_FRAME or CAN_REMOTE_FRAME
	CAN_STATUS_NOT_COMPLETED	    // Status: by default CAN_STATUS_NOT_COMPLETED
};

volatile can_msg_t mob_ram_ch0[NB_MOB_CHANNEL];

static char cdh_can_init()
{
	// Generic clock must be set up in order to use CAN.
	/* Setup the generic clock for CAN */
	scif_gc_setup(AVR32_SCIF_GCLK_CANIF, SCIF_GCCTRL_OSC0, AVR32_SCIF_GC_NO_DIV_CLOCK, 0);
			
	/* Now enable the generic clock */
	scif_gc_enable(AVR32_SCIF_GCLK_CANIF);
	
	static const gpio_map_t CAN_GPIO_MAP = {
			{AVR32_CANIF_RXLINE_0_1_PIN, AVR32_CANIF_RXLINE_0_1_FUNCTION},
			{AVR32_CANIF_TXLINE_0_1_PIN, AVR32_CANIF_TXLINE_0_1_FUNCTION}
		};
		
	/* Assign GPIO to CAN. */
	gpio_enable_module(CAN_GPIO_MAP, sizeof(CAN_GPIO_MAP) / sizeof(CAN_GPIO_MAP[0]));
	
	can_init(CAN_CHANNEL_0, ((uint32_t)&mob_ram_ch0[0]), CANIF_CHANNEL_MODE_NORMAL, NULL);
	
	rx_mob.handle = can_mob_alloc(CAN_CHANNEL_0);
	tx_mob.handle = can_mob_alloc(CAN_CHANNEL_0);
	
	can_rx(CAN_CHANNEL_0, rx_mob.handle, rx_mob.req_type, rx_mob.can_msg);
	#if 0
	int ix = 0;
	for (;;) {
		/* Do nothing; interrupts handle the DAC conversions */
		if (CANIF_channel_receive_status(0))
		{
			ix ++;
			while (CANIF_channel_receive_status(0)) {}
					
			rx_mob.can_msg->data.u64 =
			can_get_mob_data(CAN_CHANNEL_0, 0).u64;
			rx_mob.can_msg->id = can_get_mob_id(CAN_CHANNEL_0, 0);
			rx_mob.dlc = can_get_mob_dlc(CAN_CHANNEL_0, 0);
			rx_mob.status = CAN_STATUS_COMPLETED;
			can_mob_free(CAN_CHANNEL_0, 0);
		}
		else
		{
			ix--;
		}
	}
	#endif
}

// --------------------------------------------------------------------------------
void CANMonitor(void *pvParameters)
{
	UNUSED(pvParameters);
	
	int ix;
	TickType_t lastWakeTime;

	CAN_Message currentTxMessage;
	CAN_Message currentRxMessage;
	    
	const TickType_t frequency = pdMS_TO_TICKS( 1 ); // Run every 1 ms.

	lastWakeTime = xTaskGetTickCount();
	while (1)
	{
		while (0 != GetNextCANTXMessage(&currentTxMessage))
		{
			tx_mob.can_msg->id = currentTxMessage.id;
			tx_mob.dlc = currentTxMessage.length;
			tx_mob.can_msg->ide_bit = currentTxMessage.extended;

			for (ix = 0; ix < tx_mob.dlc; ix++)
			{
				tx_mob.can_msg->data.u8[ix] = currentTxMessage.data.bytes[ix];
			}
			
			can_tx(CAN_CHANNEL_0, tx_mob.handle, tx_mob.dlc, tx_mob.req_type, tx_mob.can_msg);
			
			while (CANIF_channel_transmit_status(CAN_CHANNEL_0)){} // Block until finished transmitting.
				
			// Transmit the message.
			SerialPrint("Transmitted message with\r\n    ID: ");
			SerialPrintInt(currentTxMessage.id);
			SerialPrint("\r\n    Length: ");
			SerialPrintInt(currentTxMessage.length);
			SerialPrint("\r\n    Bytes : ");
			for (ix = 0; ix < currentTxMessage.length; ix++)
			{
				SerialPrintInt(currentTxMessage.data.bytes[ix]);
				SerialPrint(" ");
			}
			SerialPrint("\r\n");
		}
		/* Initialize RX message */
		if (CANIF_channel_receive_status(0))
		{
			while (CANIF_channel_receive_status(0)) {}
			
			if (rx_mob.req_type == CAN_DATA_FRAME)
			{
				rx_mob.can_msg->data.u64 =
				can_get_mob_data(CAN_CHANNEL_0, 0).u64;
				rx_mob.can_msg->id = can_get_mob_id(CAN_CHANNEL_0, 0);
				rx_mob.dlc = can_get_mob_dlc(CAN_CHANNEL_0, 0);
				rx_mob.status = CAN_STATUS_COMPLETED;
				can_mob_free(CAN_CHANNEL_0, 0);
						
				currentRxMessage.id = rx_mob.can_msg->id;
				currentRxMessage.extended = rx_mob.can_msg->ide_bit;
				currentRxMessage.length = rx_mob.dlc;
						
				for (ix = 0; ix< currentRxMessage.length; ix++)
				{
					currentRxMessage.data.bytes[ix] = (char)rx_mob.can_msg->data.u8[ix];
				}
						
				while (AddToRXQueue(&currentRxMessage) == 0)
				{
					// Prefer this to yielding.
					vTaskDelay(0);
				}	
			}
			can_rx(CAN_CHANNEL_0, rx_mob.handle, rx_mob.req_type, rx_mob.can_msg);
		}
		taskYIELD();
		//vTaskDelayUntil(&lastWakeTime, frequency);
	}
}
