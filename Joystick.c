/*
Nintendo Switch Fightstick - Proof-of-Concept

Based on the LUFA library's Low-Level Joystick Demo
	(C) Dean Camera
Based on the HORI's Pokken Tournament Pro Pad design
	(C) HORI

This project implements a modified version of HORI's Pokken Tournament Pro Pad
USB descriptors to allow for the creation of custom controllers for the
Nintendo Switch. This also works to a limited degree on the PS3.

Since System Update v3.0.0, the Nintendo Switch recognizes the Pokken
Tournament Pro Pad as a Pro Controller. Physical design limitations prevent
the Pokken Controller from functioning at the same level as the Pro
Controller. However, by default most of the descriptors are there, with the
exception of Home and Capture. Descriptor modification allows us to unlock
these buttons for our use.
*/

#include "Joystick.h"

static const uint8_t UP = 0;
static const uint8_t DOWN = 1;
static const uint8_t LEFT = 2;
static const uint8_t RIGHT = 3;
static const uint8_t HOME = 4;
static const uint8_t A = 5;
static const uint8_t B = 6;
static const uint8_t NOTHING = 7;

typedef struct {
	uint8_t button;
	uint8_t duration;
} command; 

// Number of cycles to hold no inputs after button press
static const uint16_t NORMAL_TAP = 20;
static const uint8_t DAYS_IN_MONTH = 3;

// Number of cycles until the macro resets the date counter
// back to 1 and skips an energy collection
// WARNING: THIS MUST MATCH REPEAT_REDUCE_CYCLES IN MACRO BELOW

static const command step[] = {
	{ NOTHING, 256 },

	{ B, 10 }, // FAKE TAP TO WAKE CTRL UP
	{ NOTHING, NORMAL_TAP },
	{ A, 10 }, // Tap well
	{ NOTHING, NORMAL_TAP },

	{ NOTHING, 40 },

	{ A, 10 }, // Scroll text
	{ NOTHING, NORMAL_TAP },

	{ NOTHING, 40 },

	{ A, 10 }, // Scroll text
	{ NOTHING, NORMAL_TAP },

	{ NOTHING, 40 },

	{ A, 10 }, // Invite
	{ NOTHING, NORMAL_TAP },

	{ NOTHING, 130 },

	{ HOME, 5 },
	{ NOTHING, NORMAL_TAP },

	{ NOTHING, 80 },

  // ADVANCE DATE - CURSOR SHOULD BE ON POKEMON GAME TO START
	{ DOWN, 5 },
	{ NOTHING, NORMAL_TAP/2 },
	{ DOWN, 5 },
	{ NOTHING, NORMAL_TAP/2 },
	{ RIGHT, 5 },
	{ NOTHING, NORMAL_TAP/2 },
	{ RIGHT, 5 },
	{ NOTHING, NORMAL_TAP/2 },
	{ RIGHT, 5 },
	{ NOTHING, NORMAL_TAP/2 },
	{ RIGHT, 5 },
	{ NOTHING, NORMAL_TAP/2 },

	{ A, 5 },
	{ NOTHING, NORMAL_TAP },

	{ NOTHING, 20 },

	{ DOWN, 5 },
	{ NOTHING, NORMAL_TAP/2 },
	{ DOWN, 5 },
	{ NOTHING, NORMAL_TAP/2 },
	{ DOWN, 5 },
	{ NOTHING, NORMAL_TAP/2 },
	{ DOWN, 5 },
	{ NOTHING, NORMAL_TAP/2 },
	{ DOWN, 5 },
	{ NOTHING, NORMAL_TAP/2 },
	{ DOWN, 5 },
	{ NOTHING, NORMAL_TAP/2 },
	{ DOWN, 5 },
	{ NOTHING, NORMAL_TAP/2 },
	{ DOWN, 5 },
	{ NOTHING, NORMAL_TAP/2 },
	{ DOWN, 5 },
	{ NOTHING, NORMAL_TAP/2 },
	{ DOWN, 5 },
	{ NOTHING, NORMAL_TAP/2 },
	{ DOWN, 5 },
	{ NOTHING, NORMAL_TAP/2 },
	{ DOWN, 5 },
	{ NOTHING, NORMAL_TAP/2 },
	{ DOWN, 5 },
	{ NOTHING, NORMAL_TAP/2 },
	{ DOWN, 5 },
	{ NOTHING, NORMAL_TAP/2 },

	{ A, 5 },
	{ NOTHING, NORMAL_TAP },

	{ NOTHING, 20 },

	{ DOWN, 5 },
	{ NOTHING, NORMAL_TAP/2 },
	{ DOWN, 5 },
	{ NOTHING, NORMAL_TAP/2 },
	{ DOWN, 5 },
	{ NOTHING, NORMAL_TAP/2 },
	{ DOWN, 5 },
	{ NOTHING, NORMAL_TAP/2 },


	{ A, 5 },
	{ NOTHING, NORMAL_TAP },

	{ NOTHING, 20 },

	{ DOWN, 5 },
	{ NOTHING, NORMAL_TAP },
	{ DOWN, 5 },
	{ NOTHING, NORMAL_TAP },

	{ A, 5 },
	{ NOTHING, NORMAL_TAP },

	{ NOTHING, 20 },

	{ RIGHT, 5 },
	{ NOTHING, NORMAL_TAP/2 },
	{ UP, 5 },
	{ NOTHING, NORMAL_TAP/2 },

	{ RIGHT, 5 },
	{ NOTHING, NORMAL_TAP/2 },
	{ RIGHT, 5 },
	{ NOTHING, NORMAL_TAP/2 },
	{ RIGHT, 5 },
	{ NOTHING, NORMAL_TAP/2 },
	{ RIGHT, 5 },
	{ NOTHING, NORMAL_TAP/2 },
	{ RIGHT, 5 },
	{ NOTHING, NORMAL_TAP/2 },


	{ A, 5 },
	{ NOTHING, 2*NORMAL_TAP },

	{ B, 5 },
	{ NOTHING, NORMAL_TAP },
	{ B, 5 },
	{ NOTHING, NORMAL_TAP },
	{ B, 5 },
	{ NOTHING, 2*NORMAL_TAP },

	{ UP, 5 },
	{ NOTHING, NORMAL_TAP/2 },
	{ LEFT, 5 },
	{ NOTHING, NORMAL_TAP/2 },
	{ LEFT, 5 },
	{ NOTHING, NORMAL_TAP/2 },

	{ NOTHING, 50 },
	// END RESET DATE -- CURSOR IS ON POKEMON GAME

	// GAME SECTION
	{ A, 10 }, // Enter game
	{ NOTHING, NORMAL_TAP },
	{ NOTHING, 50 },

	{ B, 10 }, // Quit
	{ NOTHING, 100 },
	{ A, 10 }, // Confirm Quit
	{ NOTHING, NORMAL_TAP },
};

static const command skip_step[] = {
	{ NOTHING, 256 },

	// { B, 10 }, // FAKE TAP TO WAKE CTRL UP
	// { NOTHING, NORMAL_TAP },

	// { A, 10 }, // Tap well
	// { NOTHING, NORMAL_TAP },

	// { NOTHING, 40 },

	// { A, 10 }, // Invite
	// { NOTHING, NORMAL_TAP },

	// { NOTHING, 130 },

	// { HOME, 5 },
	// { NOTHING, NORMAL_TAP },

	// { NOTHING, 80 },

  // // ADVANCE DATE - CURSOR SHOULD BE ON POKEMON GAME TO START
	// { DOWN, 5 },
	// { NOTHING, NORMAL_TAP/2 },
	// { DOWN, 5 },
	// { NOTHING, NORMAL_TAP/2 },
	// { RIGHT, 5 },
	// { NOTHING, NORMAL_TAP/2 },
	// { RIGHT, 5 },
	// { NOTHING, NORMAL_TAP/2 },
	// { RIGHT, 5 },
	// { NOTHING, NORMAL_TAP/2 },
	// { RIGHT, 5 },
	// { NOTHING, NORMAL_TAP/2 },

	// { A, 5 },
	// { NOTHING, NORMAL_TAP },

	// { NOTHING, 20 },

	// { DOWN, 5 },
	// { NOTHING, NORMAL_TAP/2 },
	// { DOWN, 5 },
	// { NOTHING, NORMAL_TAP/2 },
	// { DOWN, 5 },
	// { NOTHING, NORMAL_TAP/2 },
	// { DOWN, 5 },
	// { NOTHING, NORMAL_TAP/2 },
	// { DOWN, 5 },
	// { NOTHING, NORMAL_TAP/2 },
	// { DOWN, 5 },
	// { NOTHING, NORMAL_TAP/2 },
	// { DOWN, 5 },
	// { NOTHING, NORMAL_TAP/2 },
	// { DOWN, 5 },
	// { NOTHING, NORMAL_TAP/2 },
	// { DOWN, 5 },
	// { NOTHING, NORMAL_TAP/2 },
	// { DOWN, 5 },
	// { NOTHING, NORMAL_TAP/2 },
	// { DOWN, 5 },
	// { NOTHING, NORMAL_TAP/2 },
	// { DOWN, 5 },
	// { NOTHING, NORMAL_TAP/2 },
	// { DOWN, 5 },
	// { NOTHING, NORMAL_TAP/2 },
	// { DOWN, 5 },
	// { NOTHING, NORMAL_TAP/2 },

	// { A, 5 },
	// { NOTHING, NORMAL_TAP },

	// { NOTHING, 20 },

	// { DOWN, 5 },
	// { NOTHING, NORMAL_TAP/2 },
	// { DOWN, 5 },
	// { NOTHING, NORMAL_TAP/2 },
	// { DOWN, 5 },
	// { NOTHING, NORMAL_TAP/2 },
	// { DOWN, 5 },
	// { NOTHING, NORMAL_TAP/2 },


	// { A, 5 },
	// { NOTHING, NORMAL_TAP },

	// { NOTHING, 20 },

	// { DOWN, 5 },
	// { NOTHING, NORMAL_TAP },
	// { DOWN, 5 },
	// { NOTHING, NORMAL_TAP },

	// { A, 5 },
	// { NOTHING, NORMAL_TAP },

	// { NOTHING, 20 },

	// { RIGHT, 5 },
	// { NOTHING, NORMAL_TAP/2 },

	// // START REPEAT_REDUCE_CYCLES
	// { DOWN, 5 },
	// { NOTHING, NORMAL_TAP/2 },
	// { DOWN, 5 },
	// { NOTHING, NORMAL_TAP/2 },
	// { DOWN, 5 },
	// { NOTHING, NORMAL_TAP/2 },
	// // END REPEAT_REDUCE_CYCLES
	// { RIGHT, 5 },
	// { NOTHING, NORMAL_TAP/2 },
	// { RIGHT, 5 },
	// { NOTHING, NORMAL_TAP/2 },
	// { RIGHT, 5 },
	// { NOTHING, NORMAL_TAP/2 },
	// { RIGHT, 5 },
	// { NOTHING, NORMAL_TAP/2 },
	// { RIGHT, 5 },
	// { NOTHING, NORMAL_TAP/2 },


	// { A, 5 },
	// { NOTHING, 2*NORMAL_TAP },

	// { B, 5 },
	// { NOTHING, NORMAL_TAP },
	// { B, 5 },
	// { NOTHING, NORMAL_TAP },
	// { B, 5 },
	// { NOTHING, 2*NORMAL_TAP },

	// { UP, 5 },
	// { NOTHING, NORMAL_TAP/2 },
	// { LEFT, 5 },
	// { NOTHING, NORMAL_TAP/2 },
	// { LEFT, 5 },
	// { NOTHING, NORMAL_TAP/2 },

	// { NOTHING, 50 },
	// // END RESET DATE -- CURSOR IS ON POKEMON GAME


	// // GAME SECTION
	// { A, 10 }, // Enter game
	// { NOTHING, NORMAL_TAP },
	// { NOTHING, 50 },

	// { B, 10 }, // Quit
	// { NOTHING, 100 },
	// { A, 10 }, // Confirm Quit
	// { NOTHING, NORMAL_TAP },
};

// Main entry point.
int main(void) {
	// We'll start by performing hardware and peripheral setup.
	SetupHardware();
	// We'll then enable global interrupts for our use.
	GlobalInterruptEnable();
	// Once that's done, we'll enter an infinite loop.
	for (;;)
	{
		// We need to run our task to process and deliver data for our IN and OUT endpoints.
		HID_Task();
		// We also need to run the main USB management task.
		USB_USBTask();
	}
}

// Configures hardware and peripherals, such as the USB peripherals.
void SetupHardware(void) {
	// We need to disable watchdog if enabled by bootloader/fuses.
	MCUSR &= ~(1 << WDRF);
	wdt_disable();

	// We need to disable clock division before initializing the USB hardware.
	clock_prescale_set(clock_div_1);
	// We can then initialize our hardware and peripherals, including the USB stack.

	#ifdef ALERT_WHEN_DONE
	// Both PORTD and PORTB will be used for the optional LED flashing and buzzer.
	#warning LED and Buzzer functionality enabled. All pins on both PORTB and \
PORTD will toggle when printing is done.
	DDRD  = 0xFF; //Teensy uses PORTD
	PORTD =  0x0;
                  //We'll just flash all pins on both ports since the UNO R3
	DDRB  = 0xFF; //uses PORTB. Micro can use either or, but both give us 2 LEDs
	PORTB =  0x0; //The ATmega328P on the UNO will be resetting, so unplug it?
	#endif
	// The USB stack should be initialized last.
	USB_Init();
}

// Fired to indicate that the device is enumerating.
void EVENT_USB_Device_Connect(void) {
	// We can indicate that we're enumerating here (via status LEDs, sound, etc.).
}

// Fired to indicate that the device is no longer connected to a host.
void EVENT_USB_Device_Disconnect(void) {
	// We can indicate that our device is not ready (via status LEDs, sound, etc.).
}

// Fired when the host set the current configuration of the USB device after enumeration.
void EVENT_USB_Device_ConfigurationChanged(void) {
	bool ConfigSuccess = true;

	// We setup the HID report endpoints.
	ConfigSuccess &= Endpoint_ConfigureEndpoint(JOYSTICK_OUT_EPADDR, EP_TYPE_INTERRUPT, JOYSTICK_EPSIZE, 1);
	ConfigSuccess &= Endpoint_ConfigureEndpoint(JOYSTICK_IN_EPADDR, EP_TYPE_INTERRUPT, JOYSTICK_EPSIZE, 1);

	// We can read ConfigSuccess to indicate a success or failure at this point.
}

// Process control requests sent to the device from the USB host.
void EVENT_USB_Device_ControlRequest(void) {
	// We can handle two control requests: a GetReport and a SetReport.

	// Not used here, it looks like we don't receive control request from the Switch.
}

// Process and deliver data from IN and OUT endpoints.
void HID_Task(void) {
	// If the device isn't connected and properly configured, we can't do anything here.
	if (USB_DeviceState != DEVICE_STATE_Configured)
		return;

	// We'll start with the OUT endpoint.
	Endpoint_SelectEndpoint(JOYSTICK_OUT_EPADDR);
	// We'll check to see if we received something on the OUT endpoint.
	if (Endpoint_IsOUTReceived())
	{
		// If we did, and the packet has data, we'll react to it.
		if (Endpoint_IsReadWriteAllowed())
		{
			// We'll create a place to store our data received from the host.
			USB_JoystickReport_Output_t JoystickOutputData;
			// We'll then take in that data, setting it up in our storage.
			while(Endpoint_Read_Stream_LE(&JoystickOutputData, sizeof(JoystickOutputData), NULL) != ENDPOINT_RWSTREAM_NoError);
			// At this point, we can react to this data.

			// However, since we're not doing anything with this data, we abandon it.
		}
		// Regardless of whether we reacted to the data, we acknowledge an OUT packet on this endpoint.
		Endpoint_ClearOUT();
	}

	// We'll then move on to the IN endpoint.
	Endpoint_SelectEndpoint(JOYSTICK_IN_EPADDR);
	// We first check to see if the host is ready to accept data.
	if (Endpoint_IsINReady())
	{
		// We'll create an empty report.
		USB_JoystickReport_Input_t JoystickInputData;
		// We'll then populate this report with what we want to send to the host.
		GetNextReport(&JoystickInputData);
		// Once populated, we can output this data to the host. We do this by first writing the data to the control stream.
		while(Endpoint_Write_Stream_LE(&JoystickInputData, sizeof(JoystickInputData), NULL) != ENDPOINT_RWSTREAM_NoError);
		// We then send an IN packet on this endpoint.
		Endpoint_ClearIN();
	}
}

typedef enum {
	SYNC_CONTROLLER,
	SYNC_POSITION,
	BREATHE,
	PROCESS,
	CLEANUP,
	DONE
} State_t;
State_t state = SYNC_CONTROLLER;

#define ECHOES 2
int echoes = 0;
USB_JoystickReport_Input_t last_report;

int report_count = 0;
int xpos = 0;
int ypos = 0;
int bufindex = 0;
int duration_count = 0;
int portsval = 0;
int cycle_repeat_counter = 0;
int skipsize = 0;

// Prepare the next report for the host.
void GetNextReport(USB_JoystickReport_Input_t* const ReportData) {

	// Prepare an empty report
	memset(ReportData, 0, sizeof(USB_JoystickReport_Input_t));
	ReportData->LX = STICK_CENTER;
	ReportData->LY = STICK_CENTER;
	ReportData->RX = STICK_CENTER;
	ReportData->RY = STICK_CENTER;
	ReportData->HAT = HAT_CENTER;

	// Repeat ECHOES times the last report
	if (echoes > 0)
	{
		memcpy(ReportData, &last_report, sizeof(USB_JoystickReport_Input_t));
		echoes--;
		return;
	}

	uint16_t buttonDuration = step[bufindex].duration;
	uint16_t buttonToHit = step[bufindex].button;
	if (buttonToHit > 6) {
		buttonDuration = buttonToHit;
		buttonToHit = NOTHING;
	} else {
		buttonDuration = NORMAL_TAP;
	}

	// if (cycle_repeat_counter == DAYS_IN_MONTH) {
	// 	buttonDuration = skip_step[bufindex].duration;
	// 	buttonToHit = skip_step[bufindex].button;
	// }

	// States and moves management
	switch (state)
	{

		case SYNC_CONTROLLER:
			state = BREATHE;
			break;

		// case SYNC_CONTROLLER:
		// 	if (report_count > 550)
		// 	{
		// 		report_count = 0;
		// 		state = SYNC_POSITION;
		// 	}
		// 	else if (report_count == 250 || report_count == 300 || report_count == 325)
		// 	{
		// 		ReportData->Button |= SWITCH_L | SWITCH_R;
		// 	}
		// 	else if (report_count == 350 || report_count == 375 || report_count == 400)
		// 	{
		// 		ReportData->Button |= SWITCH_A;
		// 	}
		// 	else
		// 	{
		// 		ReportData->Button = 0;
		// 		ReportData->LX = STICK_CENTER;
		// 		ReportData->LY = STICK_CENTER;
		// 		ReportData->RX = STICK_CENTER;
		// 		ReportData->RY = STICK_CENTER;
		// 		ReportData->HAT = HAT_CENTER;
		// 	}
		// 	report_count++;
		// 	break;

		case SYNC_POSITION:
			bufindex = 0;


			ReportData->Button = 0;
			ReportData->LX = STICK_CENTER;
			ReportData->LY = STICK_CENTER;
			ReportData->RX = STICK_CENTER;
			ReportData->RY = STICK_CENTER;
			ReportData->HAT = HAT_CENTER;


			state = BREATHE;
			break;

		case BREATHE:
			state = PROCESS;
			break;

		case PROCESS:

			switch (buttonToHit)
			{

				case UP:
					ReportData->LY = STICK_MIN;				
					break;

				case LEFT:
					ReportData->LX = STICK_MIN;				
					break;

				case DOWN:
					ReportData->LY = STICK_MAX;				
					break;

				case RIGHT:
					ReportData->LX = STICK_MAX;				
					break;

				case HOME:
					ReportData->Button |= SWITCH_HOME;
					break;

				case A:
					ReportData->Button |= SWITCH_A;
					break;

				case B:
					ReportData->Button |= SWITCH_B;
					break;

				// case R:
				// 	ReportData->Button |= SWITCH_R;
				// 	break;

				// case THROW:
				// 	ReportData->LY = STICK_MIN;				
				// 	ReportData->Button |= SWITCH_R;
				// 	break;

				// case TRIGGERS:
				// 	ReportData->Button |= SWITCH_L | SWITCH_R;
				// 	break;

				default:
					ReportData->LX = STICK_CENTER;
					ReportData->LY = STICK_CENTER;
					ReportData->RX = STICK_CENTER;
					ReportData->RY = STICK_CENTER;
					ReportData->HAT = HAT_CENTER;
					break;
			}

			duration_count++;

			if (duration_count > buttonDuration)
			{
				bufindex++;
				duration_count = 0;				
			}

			if (cycle_repeat_counter == DAYS_IN_MONTH) {
				skipsize = (int)( sizeof(skip_step) / sizeof(skip_step[0]));
			} else {
				skipsize = (int)( sizeof(step) / sizeof(step[0]));
			}
			if (bufindex > skipsize - 1)
			{

				bufindex = 0;
				duration_count = 0;
				if (cycle_repeat_counter == DAYS_IN_MONTH) {
					cycle_repeat_counter = 0;
				} else {
					cycle_repeat_counter++;
				}

				state = BREATHE;

				ReportData->LX = STICK_CENTER;
				ReportData->LY = STICK_CENTER;
				ReportData->RX = STICK_CENTER;
				ReportData->RY = STICK_CENTER;
				ReportData->HAT = HAT_CENTER;
			}

			break;

		case CLEANUP:
			state = DONE;
			break;

		case DONE:
			#ifdef ALERT_WHEN_DONE
			portsval = ~portsval;
			PORTD = portsval; //flash LED(s) and sound buzzer if attached
			PORTB = portsval;
			_delay_ms(250);
			#endif
			return;
	}

	// // Inking
	// if (state != SYNC_CONTROLLER && state != SYNC_POSITION)
	// 	if (pgm_read_byte(&(image_data[(xpos / 8) + (ypos * 40)])) & 1 << (xpos % 8))
	// 		ReportData->Button |= SWITCH_A;

	// Prepare to echo this report
	memcpy(&last_report, ReportData, sizeof(USB_JoystickReport_Input_t));
	echoes = ECHOES;

}
