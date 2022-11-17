// Last edit 11/15/2022
#include "mbed.h"
#include <iostream>

AnalogIn TemperatureSensor(PTB1);
DigitalOut RED_LED(LED1);
DigitalOut GREEN_LED(LED2);
DigitalOut BLUE_LED(LED3);
DigitalOut OUTPUT(PTC2);

#define Vsupply 3.3f //microcontroller voltage supply 3.3V


//variables for temperature sensor
float TemperatureSensorDigiValue; //the A/D converter value read by the controller input pin
float TemperatureSensorVoltValue; //the voltage on the controller input pin (across the 10k resistor) from the temperature sensor voltage divider
float ThermistorResistance; //computed from the voltage drop value across the thermistor
float ThermistorTemperature; //approximate ambient temperature measured by thermistor
#define ThermistorBiasResistor 10000.0f //Bias resistor (lower leg of voltage divider) for thermistor


// Variables to hold control reference values.
float TemperatureLimit = 10.0; //enter a temperature in Celsius here for temperature deactivation; NOTE: room temperature is 25C

float getThermistorTemperature(void)
{
    TemperatureSensorDigiValue = TemperatureSensor.read(); //read the TemerpatureSensor A/D value
    TemperatureSensorVoltValue = Vsupply*TemperatureSensorDigiValue; //convert to voltage
    ThermistorResistance = ThermistorBiasResistor*((Vsupply - TemperatureSensorVoltValue) / TemperatureSensorVoltValue); //voltage divider equation to determine LDR resistance
        //----------------------------------------------
    ThermistorTemperature = ((ThermistorResistance - 10000.0)/(-320.0)) + 25.0; //temperature of the thermistor computed by a linear approximation of the device response

    return ThermistorTemperature;
}

//This function will check for a temperature triggered deactivation of the motor
void CheckTemperatureSensor(void)
{
    if(getThermistorTemperature() <= TemperatureLimit) {
    cout << "Thermistor START!" << endl;
    OUTPUT = 0.91;
    RED_LED = 0; // ON
    }
    else {
    RED_LED = 1; // OFF
    OUTPUT = 0;
    }

}

// Standard entry point in C++.
int main(void)
{
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
        CheckTemperatureSensor();
        cout << "\rCurrent Temperature Value: " << getThermistorTemperature() << endl;
        wait(100.0); // Wait 1 second before repeating the loop.
    }
}
