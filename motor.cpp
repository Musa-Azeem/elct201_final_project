/**
 * ELCT201 Final Project
 * 
 * Driver Code for KL25Z microcontroller 
 * 
 * Pins:
 *      PTD1:   Torque Sensor Input
 *      PTA1:   Pressure Pad Button 0 Interrupt
 *      
 *      PTB0:   Motor Up Output
 *      PTB1:   Motor Down Output
 * 
 * Circuit Assumptions:
 *      Motor Current Measuring Resistor is 10 Ohm
 */

#include "mbed.h"
#include <iostream>

// ASSIGN PINS
// Sensor input
AnalogIn TorqueSensor(PTD1);

// Pressure Pad Buttons
InterruptIn PressureButton0(PTA1);

// Output Signals
DigitalOut OutputMotorUp(PTB0);
DigitalOut OutputMotorDown(PTB1);

// on-board LEDs
DigitalOut RED_LED(LED1);
DigitalOut GREEN_LED(LED2);
DigitalOut BLUE_LED(LED3);

// GLOBAL CONSTANTS
#define V_SUPPLY 3.3f                       // Microcontroller voltage supply 3.3 V
#define MOTOR_SERIES_RESISTANCE 10.0f       // Resistance of torque (current) sensing resistor in series with the Motor in Ohms
#define MOTOR_CURRENT_LIMIT 0.1f            // Threshold current in amps for motor to shut off
#define CYCLE_TIME 0.5f                     // Time in seconds for microcontroller to loop
#define DOOR_FALL_TIME 10.0f                // Time in seconds that it takes house door to close 
#define DOOR_RISE_TIME 10.0f                // Time in seconds that it takes house door to close 

// For LED function
#define RED 0
#define GREEN 1
#define BLUE 2

// GLOBAL VARIABLES
bool timer_up_en = false;                   // initialize with timer disabled
bool timer_down_en = false;                 // initialize with timer disabled
float timer_down_value = DOOR_FALL_TIME;    // initialize timer as time to close door
float timer_up_value = DOOR_RISE_TIME;      // initialize timer as time to open door
bool door_is_up = false;                    // True if door is up, false if it is down


void iterate_and_check_timer();
void light_LED(int LED);
void pressure_detected();
void pressure_relieved();
float get_motor_current();
void check_torque_sensor();
void attachInterrupts();


void iterate_and_check_timer() {
    /**
     * If timer is enabled, iterates timer by the time passed based on CYCLE_TIME
     * If the timer ends, stops motor
     * 
     * Does this for either the timer going up or the timer going down
     */

    // Control timer for motor going up
    if (timer_up_en) {
        // Advance by CYCLE_TIME each cycle if up timer is enabled
        timer_up_value -= CYCLE_TIME;

        if (timer_up_value <= 0) {
            // If timer reaches zero, stop motor and reset timer
            cout << "Stopping motor up" << endl;
            light_LED(RED);
            timer_up_en = false;                // disable timer
            timer_up_value = DOOR_RISE_TIME;        // reset timer
            OutputMotorUp = 0;
            door_is_up = true;                      // door has reached top
        }
    }

    // Control timer for motor going down
    else if (timer_down_en) {
        // Advance by CYCLE_TIME each cycle if timer is enabled
        timer_down_value -= CYCLE_TIME;

        if (timer_down_value <= 0) {
            // If timer reaches zero, stop motor and reset timer
            cout << "Stopping motor down" << endl;
            light_LED(RED);
            timer_down_en = false;                   // disable timer
            timer_down_value = DOOR_FALL_TIME;   // reset timer
            OutputMotorDown = 0;
            door_is_up = false;                      // door has reached bottom
        }
    }
}


void light_LED(int LED){
    /**
     * This function turns only the given color LED on
     * Input:
     *   0 -> RED
     *   1 -> GREEN
     *   2 -> BLUE
     */

    RED_LED = 1;
    GREEN_LED = 1;
    BLUE_LED = 1;

    switch(LED) {
        case RED:   RED_LED = 0;    break;
        case GREEN: GREEN_LED = 0;  break;
        case BLUE:  BLUE_LED = 0;   break;
    }
}

// This function will be attached to the start button interrupt.
void pressure_detected()
{
    /**
     * This function starts the motor going up if the door is not already up
     * It also startes the timer for going up
     * It should be called when pressure is detected on the pad
     */

    cout << "Pressure Detected" << endl;
    // Start motor up if the door is down and it is not already going up (timer is active)
    if (!door_is_up && !timer_up_en){
        cout << " - Start Motor Up" << endl;;
        OutputMotorUp = 1;
        timer_up_en = true;          // Start timer to stop motor
        light_LED(GREEN);
    }
}

void pressure_relieved()
{
    /**
     * This function starts the motor going down if the door is up
     * It also starts the timer to count down until the motor
     *  should be stopped
     * It should be called when pressue on pad is relieved
     */

    cout << "Pressure Relieved" << endl;
    // Start motor down if the door is up and it is not already going down (timer is active)
    if (door_is_up && !timer_down_en){
        cout << "- Start Motor Down" << endl;
        OutputMotorDown = 1;
        timer_down_en = true;          // Start timer to stop motor
        light_LED(BLUE);
    }
}

float get_motor_current()
{
    /**
     * This function will determine the motor current in amperes 
     *  1. Read the TorqueSensor value to get the A/D converter value
     *  2. Calculate voltage on the controller input pin (across the 10 Ohm 
     *     resistor) from the motor torque sensor by multiplying the digi value
     *     by the supply voltage
     *  3. Calculate motor current using Ohm's law from votlage and resistance 
     */  

    float motor_current_digi_value = TorqueSensor.read();
    float motor_current_volt_value = V_SUPPLY * motor_current_digi_value;
    float motor_current = motor_current_volt_value / MOTOR_SERIES_RESISTANCE;
    cout << "\rMotor Current: " << motor_current << endl;
    return motor_current;
}

void check_torque_sensor()
{
    /**
     * This function will check if the current accross the motor is too high
     * If it is, it stops the motor whether it is going up or down
    */

    if(get_motor_current() >= MOTOR_CURRENT_LIMIT) {
        cout << "Torque Overload - stopping motor" << endl;
        OutputMotorUp = 0;        // Stop motor if going up   (usual case)
        OutputMotorDown = 0;      // Stop motor if going down (only emergency)
        light_LED(RED);
    }
}

void attachInterrupts() {
    PressureButton0.fall(&pressure_detected);
    PressureButton0.rise(&pressure_relieved);
}

// Standard entry point in C++.
int main(void) {
    // Attach the functions to the hardware interrupt pins.
    attachInterrupts();
    
    // Initialize LED outputs to OFF (LED logic is inverted)
    RED_LED = 1;
    GREEN_LED = 1;
    BLUE_LED = 1;

    // Blink the blue LED once to indicate the code is running.
    BLUE_LED = !BLUE_LED;
    wait(1.0);
    BLUE_LED = !BLUE_LED;

    while(true) {

        // Check the analog inputs.
        check_torque_sensor();

        // Iterate and check timer
        iterate_and_check_timer();

        // Print Current state of ouputs
        cout << "\rOUTPUT MOTOR UP PTC3: " << OutputMotorUp << endl;
        cout << "\rOUTPUT MOTOR DOWN PTC9: " << OutputMotorDown << endl;

        wait(CYCLE_TIME); // Wait <cycle_time> seconds before repeating the loop.
    }
}
// End of HardwareInterruptSeedCode
