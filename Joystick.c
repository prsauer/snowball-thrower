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
static const uint8_t X = 7;
static const uint8_t NOTHING = 8;
static const uint8_t LONG_LEFT = 9;
static const uint8_t LONG_UP = 10;
static const uint8_t LONG_DOWN = 11;
static const uint8_t LONG_RIGHT = 12;
static const uint8_t SHORT_LEFT = 13;
static const uint8_t SHORT_UP = 14;
static const uint8_t SHORT_DOWN = 15;
static const uint8_t SHORT_RIGHT = 16;
static const uint8_t STICK_CIRCLE = 17;
static const uint8_t STICK_CIRCLE_ADOWN = 18;
static const uint8_t R = 19;

static const uint8_t MAX_JOY_ENUM = 20;

typedef struct {
	uint8_t button;
} command;

// Number of cycles to hold no inputs after button press
static const uint16_t NORMAL_TAP = 20;
static const uint16_t FAST_TAP = 244;

// Length of press constants
static const uint8_t LONGNESS_FACTOR = 5;
static const uint16_t NORMAL_DOWN = 8;
static const uint16_t FAST_DOWN = 2;

// Column size limits for pokemon release sequence
static const uint8_t NUM_ROWS = 5;
static const uint8_t NUM_COLUMNS = 5;

static const uint8_t WAIT_FOR_RELEASE = 64;
static const uint8_t NORMAL_WAIT = 32;
static const uint8_t WAIT_BETWEEN = 64;

static const command step[] = {
	{ 128 },
	{ X }, // wake up -- ACTUALLY PLUS!
	{ NORMAL_WAIT },
	{ R },
	{ WAIT_BETWEEN },
	{ A },
	{ NORMAL_WAIT },
	{ UP },
	{ NORMAL_WAIT },
	{ UP },
	{ NORMAL_WAIT },
	{ A },
	{ NORMAL_WAIT },
	{ UP },
	{ NORMAL_WAIT },
	{ A },
	{ WAIT_FOR_RELEASE },
	{ A },
	{ WAIT_BETWEEN },
	{ DOWN },
	{ NORMAL_WAIT },
	{ A },
	{ NORMAL_WAIT },
	{ UP },
	{ NORMAL_WAIT },
	{ UP },
	{ NORMAL_WAIT },
	{ A },
	{ NORMAL_WAIT },
	{ UP },
	{ NORMAL_WAIT },
	{ A },
	{ WAIT_FOR_RELEASE },
	{ A },
	{ WAIT_BETWEEN },
	{ DOWN },
	{ NORMAL_WAIT },
	{ A },
	{ NORMAL_WAIT },
	{ UP },
	{ NORMAL_WAIT },
	{ UP },
	{ NORMAL_WAIT },
	{ A },
	{ NORMAL_WAIT },
	{ UP },
	{ NORMAL_WAIT },
	{ A },
	{ WAIT_FOR_RELEASE },
	{ A },
	{ WAIT_BETWEEN },
	{ DOWN },
	{ NORMAL_WAIT },
	{ A },
	{ NORMAL_WAIT },
	{ UP },
	{ NORMAL_WAIT },
	{ UP },
	{ NORMAL_WAIT },
	{ A },
	{ NORMAL_WAIT },
	{ UP },
	{ NORMAL_WAIT },
	{ A },
	{ WAIT_FOR_RELEASE },
	{ A },
	{ WAIT_BETWEEN },
	{ DOWN },
	{ NORMAL_WAIT },
	{ A },
	{ NORMAL_WAIT },
	{ UP },
	{ NORMAL_WAIT },
	{ UP },
	{ NORMAL_WAIT },
	{ A },
	{ NORMAL_WAIT },
	{ UP },
	{ NORMAL_WAIT },
	{ A },
	{ WAIT_FOR_RELEASE },
	{ A },
	{ WAIT_BETWEEN },
};

static const command column_swap[] = {
	{ 32 },
	{ RIGHT },
	{ 32 },
	{ UP },
	{ 32 },
	{ UP },
	{ 32 },
	{ UP },
	{ 32 },
	{ UP },
	{ 32 },
};

static const command end_swap[] = {
	{ 64 },
	{ UP },
	{ 32 },
	{ UP },
	{ 32 },
	{ UP },
	{ 32 },
	{ UP },
	{ 32 },
	{ LEFT },
	{ 32 },
	{ LEFT },
	{ 32 },
	{ LEFT },
	{ 32 },
	{ LEFT },
	{ 32 },
	{ LEFT },
	{ 32 },
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
int skipsize = 0;
int rows_done = 0;
int cur_col = 0;

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

	uint16_t buttonDuration = NORMAL_DOWN;
	uint16_t buttonToHit = step[bufindex].button;

	if (rows_done && cur_col == NUM_COLUMNS) {
		buttonToHit = end_swap[bufindex].button;
	}
	else if (rows_done) {
		buttonToHit = column_swap[bufindex].button;
	}

	if ( buttonToHit == STICK_CIRCLE || buttonToHit == STICK_CIRCLE_ADOWN ) {
		buttonDuration = NORMAL_DOWN * 20;
	}

	if (
		buttonToHit == SHORT_LEFT ||
		buttonToHit == SHORT_UP ||
		buttonToHit == SHORT_DOWN
		|| buttonToHit == SHORT_RIGHT
	) {
		buttonDuration = FAST_DOWN;	
	}
	if (
		buttonToHit == LONG_LEFT || 
		buttonToHit == LONG_UP || 
		buttonToHit == LONG_DOWN
		|| buttonToHit == LONG_RIGHT
	) {
		buttonDuration = LONGNESS_FACTOR*NORMAL_DOWN;
	}

	if (buttonToHit == FAST_TAP) {
		buttonDuration = 7;
		buttonToHit = NOTHING;
	}

	if (buttonToHit > MAX_JOY_ENUM) {
		buttonDuration = buttonToHit;
		buttonToHit = NOTHING;
	}

	// States and moves management
	switch (state)
	{

		case SYNC_CONTROLLER:
			state = BREATHE;
			break;

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

				case STICK_CIRCLE:
					ReportData->RX = STICK_MIN;
					ReportData->LX = STICK_MAX;
					break;

				case STICK_CIRCLE_ADOWN:
					ReportData->RX = STICK_MIN;
					ReportData->LX = STICK_MAX;
					ReportData->Button |= SWITCH_A;
					break;

				case SHORT_UP:
				case LONG_UP:
				case UP:
					ReportData->LY = STICK_MIN;				
					break;

				case SHORT_LEFT:
				case LONG_LEFT:
				case LEFT:
					ReportData->LX = STICK_MIN;				
					break;

				case LONG_DOWN:
				case SHORT_DOWN:
				case DOWN:
					ReportData->LY = STICK_MAX;				
					break;

				case LONG_RIGHT:
				case SHORT_RIGHT:
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

				case X:
					ReportData->Button |= SWITCH_PLUS;
					break;

				case R:
					ReportData->Button |= SWITCH_R;
					break;

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

			if (rows_done && cur_col == NUM_COLUMNS) {
				skipsize = (int)( sizeof(end_swap) / sizeof(end_swap[0]));
			} else if (rows_done) {
				skipsize = (int)( sizeof(column_swap) / sizeof(column_swap[0]));
			} else {
				skipsize = (int)( sizeof(step) / sizeof(step[0]));
			}

			if (bufindex > skipsize - 1)
			{
				state = BREATHE;
				bufindex = 0;
				duration_count = 0;

				if (!rows_done) {
					rows_done = 1;
				} else if (rows_done) {
					bufindex = 4; // start on step 4 to skip warm up
					rows_done = 0;
					cur_col++;
				}
				if (cur_col > NUM_COLUMNS) {
					state = DONE;
				}

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

	// Prepare to echo this report
	memcpy(&last_report, ReportData, sizeof(USB_JoystickReport_Input_t));
	echoes = ECHOES;

}
