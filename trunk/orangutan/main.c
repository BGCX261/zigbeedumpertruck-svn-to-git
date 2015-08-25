#include <pololu/orangutan.h>
#include "control.h"
#include <avr/interrupt.h>

extern char receive_buffer[];
extern unsigned char receive_buffer_position;
extern char send_buffer[];

// Counter for calculating one second for the speed measurement
uint8_t second_counter = 0;
// Number of pulses received from speed sensor within one second
uint8_t speed_pulses = 0;
extern volatile uint8_t current_speed;

/**
 * Interrupt handler for timer 0 overflow. Timer 0 is used for
 * saving the speed of the car once every second.
 */
SIGNAL(TIMER0_OVF_vect)
{
	// If one second has passed: 76*256*1024 =~ 20 000 000
	if(second_counter >= 76) {
		current_speed = speed_pulses;
		second_counter = 0;
		speed_pulses = 0;
	}
	else
		++second_counter;
}


/**
 * Interrupt handler for pin change interrupts of theft alarm
 * and speed sensor pins. The theft alarm is connected to PC1
 * and the speed sensor to PC5.
 */
SIGNAL(PCINT1_vect)
{
	// Theft alarm is active if the pin is low
/*	if(!(PINC & _BV(PIN1))) {
		clear();
		print("halytys");
		play_note('c', 16, 15);
	}*/
	// Speedometer pulse received
//	else {
		++speed_pulses;
//	}
}


/**
 * Watchdog overflow interrupt. Stop the car.
 */
SIGNAL(WDT_vect)
{
	set_speed(0);
	set_steering(0);
	clear();
	print("WDT");
	lcd_goto_xy(0, 1);
	print("reset");
}



int main()
{
	// Turn off the watchdog timer so that it doesn't reset the
	// microcontroller all the time
	wdt_off();

	init_serial();

	print("Starting");
	delay(1000);
//	play("!T240 L8 a gafaeada c+adaeafa");
	play("!L16 V8 cdefgab>cbagfedc");
	delay(1000);
	// Steering servo connected to PC2 and motor controller
	// servo to PC3
	init_servos();
//	const unsigned char servoPinsB[] = {IO_C2, IO_C3};
//	servos_start(servoPinsB, sizeof(servoPinsB));

	set_speed(0);
	set_steering(0);
	
	init_speedometer();

	while(1)
	{
		// Deal with any new bytes received.
		check_for_new_bytes_received();

		if(button_is_pressed(TOP_BUTTON)) {
			set_speed(0);
			set_steering(0);
			wait_for_button_release(TOP_BUTTON);
		}
		if (button_is_pressed(MIDDLE_BUTTON)) {
			wait_for_sending_to_finish();
			memcpy_P(send_buffer, PSTR("Hi there! How are you feeling today? This must work now\r\n"), 59);
			serial_send(send_buffer, 59);

			wait_for_button_release(MIDDLE_BUTTON);
		}
		if(button_is_pressed(BOTTOM_BUTTON)) {
			clear();
			print("WDT ON");
			wdt_on();
			wait_for_button_release(BOTTOM_BUTTON);
		}
	}
}

