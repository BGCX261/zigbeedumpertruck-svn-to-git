#ifndef CONTROL_H
#define CONTROL_H

// Servo limits
#define STEERING_LEFT 1000
#define STEERING_CENTER 1350
#define STEERING_RIGHT 1640
#define STEERING_STEP 32

#define MOTOR_MIN 1900 // forward
#define MOTOR_CENTER 1400
#define MOTOR_MAX 850 // backward
#define MOTOR_STEP 50

#define NO_CMD 0

// Command byte definitions
#define HEART_BEAT 0
#define SPEED_COMMAND 1
#define STEERING_COMMAND 2
#define READ_VOLTAGE 3
#define READ_TEMP 4
#define PRINT_MSG 5
#define ALARM_ON 6
#define ALARM_OFF 7
#define WDT_ON 8
#define READ_SPEED 9


/**
 * Initializes the UART used for serial communication
 * with the XBee module.
 */
void init_serial();

/**
 * Initialize the steering and motor controller servos.
 * Steering servo connected to PC2 and motor controller
 * servo to PC3.
 */
void init_servos();


/**
 * Wait until the serial send buffer is empty so that new
 * bytes can be safely written to the buffer for sending
 */
void wait_for_sending_to_finish();


/**
 * Set speed. -10 is full speed backwards, 0 stops,
 * 10 full speed forward.
 */
void set_speed(int8_t speed);


/**
 * Set steering:
 * -10 = full left
 * 0   = straight
 * 10  = full right
 */
void set_steering(int8_t steering);


/**
 * Send the voltage to UART (XBee module).
 * Using this function might not be very wise because
 * the orangutan seems to lock up during the battery
 * voltage reading.
 */
void send_voltage();


/**
 * Sends temperature encoded as two characters to the computer.
 * The first byte is the msb and the second byte the lsb of a
 * 16 bit sign extended integer. Four lowest order bytes of the
 * integer form the decimal part.
 */
void send_temp();


/**
 * Init the speedometer (connected to PC5).
 * The speed is obtained by calculating the number of pulses
 * the speed sensor sends within one second. Timer 0 is used
 * in the calculation of the 1 second timing.
 */
void init_speedometer();


/**
 * Sends the speed of the truck to the computer.
 * The speed is typically between 0 and 14.
 */
void send_speed();


/**
 * Turn on the antitheft alarm system
 * (connected to PC1 (PCINT9))
 */
void alarm_on();


/**
 * Turn off the antitheft alarm system
 */
void alarm_off();


/**
 * Turn on the watchdog timer. The watchdog resets the
 * microcontroller (and thus stops the truck) if it is
 * not reset within 500 ms from the last reset.
 * The resetting of the watchdog is performed when a
 * heartbeat signal is received from the controlling
 * computer.
 */
void wdt_on();


/**
 * Turn off the watchdog timer.
 */
void wdt_off();


/**
 * Processes an argument data byte of received message.
 */
void process_data(char byte);


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
void process_received_byte(char byte);


/**
 * Checks if there are new bytes received from the UART
 * in the receive buffer. If new bytes have been received,
 * they are processed with the function
 * process_received_byte.
 */
void check_for_new_bytes_received();

#endif // CONTROL_H
