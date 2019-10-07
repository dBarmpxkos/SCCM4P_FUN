#include <AD9833.h>        
#include <ArduinoJson.h>

#define FNC_PIN       10
#define LED_PIN       13

AD9833 gen(FNC_PIN);

/* helper functions */
static void yield_on_char(void){
    if ( serialEventRun ) serialEventRun();
    if ( Serial.available() ) return;
}

static void flush_serial_input(void){
    if ( serialEventRun ) serialEventRun();
    do { 
        Serial.read(); 
        delay(50); 
    } while ( Serial.available() > 0 );
}
 
static void blink_LED(void){
    digitalWrite(LED_PIN, digitalRead(LED_PIN) == HIGH ? LOW : HIGH);
}
/* helper functions */

bool waveform_settings(void) {
    StaticJsonDocument<200> JSONSettings;

    DeserializationError error = deserializeJson(JSONSettings, Serial);
    if (error) {
        Serial.print(F("[err]: deserializeJson() failed: "));
        Serial.println(error.c_str());
        return;
    }

    bool enable = JSONSettings["en"];
    Serial.print(F("[JSONSettings][en]: ")); Serial.println(enable);
    if (!enable)
        gen.EnableOutput(enable);
    
    else {
        Registers outputRegister;
        unsigned int incomingRegister = JSONSettings["reg"];
        Serial.print(F("[JSONSettings][reg]: ")); Serial.println(incomingRegister);
        switch ( incomingRegister ) {
            case 0:
                outputRegister = REG0;
                break;
            case 1:
                outputRegister = REG1;
                break;  
            default:
                Serial.println(F("[err]: error in register setting"));
                break;                    
        } 

        WaveformType waveType;
        unsigned int incomingWave = JSONSettings["wave"];
        Serial.print(F("[JSONSettings][waveType]: ")); Serial.println(incomingWave);
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
                Serial.println(F("[err]: error in waveform setting"));
                break;                    
        } 

        float frequency = JSONSettings["freq"];
        Serial.print(F("[JSONSettings][frequency]: ")); Serial.println(frequency);

        float phase     = JSONSettings["phase"];
        Serial.print(F("[JSONSettings][phase]: ")); Serial.println(phase);

        gen.ApplySignal(waveType, outputRegister, frequency, 
                        outputRegister, phase);
        gen.SetOutputSource(outputRegister);
        gen.EnableOutput(enable);

    }
}


bool currentsource_settings(void) {

    return 1;

}  

int8_t monitor_serial(void) {

    int8_t settingsType = -1;

    if (Serial.available() > 0) {
        char cmd = Serial.read();
        /* if first char is indicating that settings are coming */
        if (cmd == '>') {   
            Serial.println(F("[ser]: settings flag received"));
            while (!Serial.available()){
                delay(5);
            }
            cmd = Serial.read();
            /* read next char; W: waveform settings - S: current source settings */
            switch (cmd) {
                case 'W':
                    settingsType = 0;
                    waveform_settings();
                    Serial.println(F("[ser][run]: waveform_settings()"));
                    break;
                case 'S':
                    settingsType = 1;
                    currentsource_settings();
                    Serial.println(F("[ser][run]: currentsource_settings()"));
                    break;
                default:
                    Serial.println(F("[err]: error in settings type"));
                    break;
            }
        }
    }
    return settingsType;
}

/* helper functions */