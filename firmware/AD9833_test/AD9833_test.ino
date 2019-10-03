#include "functions.h"

void setup(){

	Serial.begin(9600);
	gen.Begin();
	gen.EnableOutput(false);

}

void loop(){

    StaticJsonDocument<200> JSONSettings;

    while (!Serial.available()){
        delay(50);
        blink_LED();
    }

    DeserializationError error = deserializeJson(JSONSettings, Serial);
    if (error) {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.c_str());
        return;
    }

    bool enable = JSONSettings["en"];

    if (!enable)
        gen.EnableOutput(enable);
    
    else {
        
        Registers outputRegister;
        unsigned int incomingRegister;
        switch ( incomingRegister ) {
            case 0:
                outputRegister = REG0;
                break;
            case 1:
                outputRegister = REG1;
                break;  
            default:
                Serial.println(F("Error in register setting"));
                break;                    
        } 

        WaveformType waveType;
        unsigned int incomingWave;
        switch ( incomingWave ) {
            case 0:
                waveType = SINE_WAVE;
                break;
            case 1:
                waveType = TRIANGLE_WAVE;
                break;
            case 2:
                waveType = SQUARE_WAVE;
                break;  
            default:
                Serial.println(F("Error in waveform setting"));
                break;                    
        } 

        float frequency = JSONSettings["freq"];
        float phase     = JSONSettings["phase"];
        gen.SetOutputSource(outputRegister);
        gen.ApplySignal(waveType, outputRegister, frequency, 
                        outputRegister, phase);
    }

}
