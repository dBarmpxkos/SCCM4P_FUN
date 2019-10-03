#include "functions.h"

void setup(){

	Serial.begin(9600);
	gen.Begin();
	gen.EnableOutput(false);

}

void loop(){

if (monitor_serial() == 0)
    waveform_settings();
}
