#include "functions.h"

void setup(){

	Serial.begin(9600);
	AD5274.begin();
	gen.Begin();
	gen.EnableOutput(false);

}

void loop(){

    monitor_serial();
    
}
