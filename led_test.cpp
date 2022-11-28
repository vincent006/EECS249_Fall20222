#include <iostream>
#include <chrono>
#include <thread>
#include <wiringPi.h>

#define PIN_LED 17
#define PIN_BUTTON 18

int main (int argc, char **argv)
{
    wiringPiSetupGpio();

    pinMode(PIN_LED, OUTPUT);
    pinMode(PIN_BUTTON, INPUT);

    printf("LED and button pins have beens setup.\n");
    
    while (1)
    {   
        if (digitalRead(PIN_BUTTON) == HIGH) {
            digitalWrite(PIN_LED, HIGH);
        }
        else {
            digitalWrite(PIN_LED, LOW);
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}