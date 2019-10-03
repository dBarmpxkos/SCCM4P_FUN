#include <AD9833.h>        
#include <ArduinoJson.h>

#define FNC_PIN       10       // Any digital pin. Used to enable SPI transfers (active LO  
#define LED_PIN       13      // I'm alive blinker 

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

int8_t monitor_serial(void) {

    int8_t settingsType = -1;
    if (Serial.available() > 0) {

        char cmd = Serial.read();
        if (cmd == '>') {
            cmd = Serial.read();

            switch (cmd) {
                case 'W':
                    settingsType = 0;
                    break;
                case 'S':
                    settingsType = 1;
                    break;
                default:
                    Serial.println(F("Error in settings type"));
                    break;
            }
        }
    }
    return settingsType;
}

bool waveform_settings(void) {
    StaticJsonDocument<200> JSONSettings;

    DeserializationError error = deserializeJson(JSONSettings, Serial);
    if (error) {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.c_str());
        return;
    }
    bool enable = JSONSettings["en"];

    if (!enable)
        gen.EnableOutput(false);
    
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
        gen.ApplySignal(waveType, outputRegister, frequency, 
                        outputRegister, phase);
        gen.SetOutputSource(outputRegister);

    }
}

/* helper functions */

/*
 * Show the requested versus actual programmed values for frequency and phase
 * Also show resolution, max frequency (based on refFrequency)
 */
void RequestedvsProgrammedValues ( void ) {
  
    float requestedFrequency, programmedFrequency;
    char  buffer[20];   // 14 characters actually needed for display    

    gen.ApplySignal(SINE_WAVE,REG0,1000.0);
    
    while ( true ) {
      
        flush_serial_input();
  
        Serial.println(F("\nEnter frequency ('Q' to quit) >"));
        while ( !Serial.available() ){
            blink_LED();
        }   

        if ( toupper(Serial.peek()) == 'Q' ) {
            // Need an extra <CR> ?
            flush_serial_input();    // why isn't this flushing input?
            return;
        }
        requestedFrequency = Serial.parseFloat();
        gen.SetFrequency(REG0,requestedFrequency);
        programmedFrequency = gen.GetActualProgrammedFrequency(REG0);
        Serial.print(F("Requested :"));
        dtostrf(requestedFrequency,14,5,buffer); 
        Serial.print(buffer);
        Serial.print(F("   Actual :"));
        dtostrf(programmedFrequency,14,5,buffer); 
        Serial.println(buffer);       
    }
}