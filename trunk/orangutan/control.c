#include <pololu/orangutan.h>
#include "ds1820.h"
#include "control.h"
#include <avr/wdt.h>
#include <avr/io.h>

// Command being currently processed and for which
// we are waiting more bytes to be received
uint8_t current_cmd = NO_CMD;

/*// Current speed obtained with speed sensor
// Number of sensor pulses per second
volatile uint8_t speed_pulses = 0;
volatile uint8_t second_counter = 0;
*/
char receive_buffer[32];
unsigned char receive_buffer_position = 0;
char send_buffer[32];

volatile uint8_t current_speed = 100;

void init_serial()
{
	serial_set_baud_rate(9600);
        serial_receive_ring(receive_buffer, sizeof(receive_buffer));
}


void init_servos()
{
	const unsigned char servoPinsB[] = {IO_C2, IO_C3};
	servos_start(servoPinsB, sizeof(servoPinsB));
	set_servo_target(0, STEERING_CENTER);
	set_servo_target(1, MOTOR_CENTER);
}


/**
 * Wait until the serial send buffer is empty so that new
 * bytes can be safely written to the buffer for sending
 */
void wait_for_sending_to_finish()
{
	while(!serial_send_buffer_empty());
}


/**
 * Set speed. -10 is full speed backwards, 0 stops,
 * 10 full speed forward.
 */
void set_speed(int8_t speed)
{
	if(speed >= -10 && speed <= 10)
		set_servo_target(1, MOTOR_CENTER + speed * MOTOR_STEP);
	else {
		clear();
		print("SPD ERR");
		lcd_goto_xy(0, 1);
		print_long(speed);
	}
}


/**
 * Set steering:
 * -10 = full left
 * 0   = straight
 * 10  = full right
 */
void set_steering(int8_t steering)
{
	if(steering >= -10 && steering <= 10)
		set_servo_target(0, STEERING_CENTER + steering * STEERING_STEP);
	else {
		clear();
		print("STEER ER");
		lcd_goto_xy(0, 1);
		print_long(steering);
	}
}


/**
 * Send the voltage to UART (XBee module).
 * Using this function might not be very wise because
 * the orangutan seems to lock up during the battery
 * voltage reading.
 */
void send_voltage()
{
	uint8_t i;
	uint16_t voltage = read_battery_millivolts_sv();

	// vonvert int to string
	for(i = 3; i >= 0; --i) {
		send_buffer[i] = voltage % 10 + '0';
		voltage = voltage / 10;
	}
	clear();
	print_long(voltage);
	wait_for_sending_to_finish();
	serial_send(send_buffer, 4);
}


/**
 * Sends temperature encoded as two characters to the computer.
 * The first byte is the msb and the second byte the lsb of a
 * 16 bit sign extended integer. Four lowest order bytes of the
 * integer form the decimal part.
 */
void send_temp()
{
	int16_t temperature = GetTemp();
	wait_for_sending_to_finish();
	send_buffer[0] = temperature >> 8;
	send_buffer[1] = temperature & 0xFF;
	send_buffer[2] = '\r';
	serial_send(send_buffer, 3);

	// Print the temperature to lcd
	clear();
	print("Temp");
	lcd_goto_xy(0,1);
	if(temperature < 0) {
		print("-");
		temperature = -temperature;
	}
	print_long(temperature >> 4);
	print_character('.');
	if(temperature & 0x08)
		print_character('5');
	else
		print_character('0');
	print(" C");
}


/**
 * Init the speedometer (connected to PC5).
 * The speed is obtained by calculating the number of pulses
 * the speed sensor sends within one second. Timer 0 is used
 * in the calculation of the 1 second timing.
 */
void init_speedometer()
{
	// Enable pin change interrupt of speed sensor
	PCICR |= _BV(PCIE1);
	PCMSK1 |= _BV(PCINT13);
	// Clock select: clk_io / 1024
	TCCR0B |= _BV(CS02);
	TCCR0B |= _BV(CS00);
	// Enable TCNT0 overflow interrupt
	TIMSK0 |= _BV(TOIE0);
	sei();
}


/**
 * Sends the speed of the truck to the computer.
 * The speed is typically between 0 and 14.
 */
void send_speed()
{
	send_buffer[0] = current_speed;
	wait_for_sending_to_finish();
	serial_send(send_buffer, 1);
	clear();
	print("Speed");
	lcd_goto_xy(0, 1);
	print_long(current_speed);
}


/**
 * Turn on the antitheft alarm system
 * (connected to PC1 (PCINT9))
 */
void alarm_on()
{
	PCICR |= _BV(PCIE1);
	PCMSK1 |= _BV(PCINT9);
}


/**
 * Turn off the antitheft alarm system
 */
void alarm_off()
{
	PCMSK1 &= ~_BV(PCINT9);
}


/**
 * Turn on the watchdog timer. The watchdog resets the
 * microcontroller (and thus stops the truck) if it is
 * not reset within 500 ms from the last reset.
 * The resetting of the watchdog is performed when a
 * heartbeat signal is received from the controlling
 * computer.
 */
void wdt_on()
{
/*	cli();
	wdt_reset();
	// Change prescaler mode
	WDTCSR |= _BV(WDCE) | _BV(WDE);
	// Set time-out to 0.25 s
	WDTCSR |= _BV(WDE) | _BV(WDP3);
	// Set interrupt mode
	WDTCSR &= ~_BV(WDE);
	WDTCSR |= _BV(WDIE);
	sei();
*/
	clear();
	print("WDT ON");
	play("!T240 L8 a gafaeada c+adaeafa");
	init_servos();
	wdt_enable(WDTO_500MS);
}


/**
 * Turn off the watchdog timer.
 */
void wdt_off()
{
	cli();
	wdt_reset();
	// Reset "Watchdog System Reset Flag"
	MCUSR &= ~_BV(WDRF);

	WDTCSR |= _BV(WDCE) | _BV(WDE);
	// Turn off WDT
	WDTCSR = 0x00;
	sei();
}


/**
 * Processes an argument data byte of received message.
 */
void process_data(char byte)
{
	switch(current_cmd) {
		case SPEED_COMMAND:
			set_speed((int8_t)byte);
			current_cmd = NO_CMD;
			break;
		case STEERING_COMMAND:
			set_steering((int8_t)byte);
			current_cmd = NO_CMD;
			break;
		case PRINT_MSG:
			// '\n' begins a new line
			if(byte == '\n')
				lcd_goto_xy(0, 1);
			// '\0' ends the message
			else if(byte == '\0')
				current_cmd = NO_CMD;
			else
				print_character(byte);
			break;
	}
}


/**
 * Processes a byte received from the computer.
 * The messages received from computer are of the following format:
 * MSG_TYPE [ARG_BYTES]
 * Each message begins with a message type and after that follows
 * optional arguments. Commands other than PRINT_MSG, SPEED_COMMAND,
 * and STEERING_COMMAND have a message length of one byte.
 * SPEED_COMMAND and STEERING_COMMAND have one argument byte.
 * The PRINT_MSG command message can be of any length and it 
 * must end with a byte '\0'.
 */
void process_received_byte(char byte)
{
	if(current_cmd != NO_CMD)
		process_data(byte);
	else {
		switch(byte) {
			case HEART_BEAT:
				wdt_reset();
				break;	
			case PRINT_MSG:
				clear();
			case SPEED_COMMAND:
			case STEERING_COMMAND:
				current_cmd = byte;
				break;
			case READ_VOLTAGE:
				send_voltage();
				break;
			case READ_TEMP:
				send_temp();
				break;
			case READ_SPEED:
				send_speed();
				break;
			case ALARM_ON:
				alarm_on();
				break;
			case ALARM_OFF:
				alarm_off();
				break;
			case WDT_ON:
				wdt_on();
				wdt_reset();
				break;
			default:
				clear();
				print("UNKNOWN");
				lcd_goto_xy(0, 1);
				print("CMD ");
				print_long(byte);
				break;
		}
	}
}


/**
 * Checks if there are new bytes received from the UART
 * in the receive buffer. If new bytes have been received,
 * they are processed with the function
 * process_received_byte.
 */
void check_for_new_bytes_received()
{
	while(serial_get_received_bytes() != receive_buffer_position)
	{
		// Process the new byte that has just been received.
		process_received_byte(receive_buffer[receive_buffer_position]);

		// Increment receive_buffer_position, but wrap around when it gets to
		// the end of the buffer. 
		if (receive_buffer_position == sizeof(receive_buffer)-1)
		{
			receive_buffer_position = 0;
		}
		else
		{
			receive_buffer_position++;
		}
	}
}

