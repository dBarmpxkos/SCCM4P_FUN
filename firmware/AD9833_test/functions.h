#include <AD9833.h>
#include <Systronix_AD5274.h>        
#include <ArduinoJson.h>

#define FNC_PIN       53
#define LED_PIN       13
/*  D13  : SCLK
    D11  : SDA
*/
AD9833 gen(FNC_PIN);
Systronix_AD5274 AD5274(AD5274_BASE_ADDR_FLOAT);

/* helper functions ------------------------------------------ */
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
/* helper functions ------------------------------------------ */


/* AD9833 functions ------------------------------------------ */

/* json parser for waveform settings */
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
/* json parser for waveform settings */

/* AD9833 functions ------------------------------------------ */


/* AD5274 functions ------------------------------------------ */

/* RDAC register for resistance setting */
int8_t write_and_read_rdac (uint16_t data_16_to_write){
    int8_t status = 0;
    int16_t read_from_ad5274 = 0;

    status += AD5274.command_write(AD5274_RDAC_WRITE, data_16_to_write);
    read_from_ad5274 = AD5274.command_read(AD5274_RDAC_READ, 0x00);
    return status;
}
/* RDAC register for resistance setting */

/* json parser for resistance settings */
bool currentsource_settings(void) {
    StaticJsonDocument<200> JSONSettings;
    uint16_t data_16_to_write = 0;
    int16_t read_from_ad5274 = 0;

    int8_t status = 0;

    DeserializationError error = deserializeJson(JSONSettings, Serial);
    if (error) {
        Serial.print(F("[err]: deserializeJson() failed: "));
        Serial.println(error.c_str());
        return;
    }
    byte resistancePercent = 0;

    if      (JSONSettings["resPerc"] <= 0) resistancePercent = 0;
    else if (JSONSettings["resPerc"] > 99) resistancePercent = 100;
    else    resistancePercent = JSONSettings["resPerc"];

    Serial.print(F("[JSONSettings][resistancePercent]: ")); Serial.println(resistancePercent);

    status = AD5274.command_write(AD5274_CONTROL_WRITE, AD5274_RDAC_WIPER_WRITE_ENABLE);
    read_from_ad5274 = AD5274.command_read(AD5274_CONTROL_READ, 0x00);
    if (read_from_ad5274 & AD5274_RDAC_WIPER_WRITE_ENABLE){
        Serial.print("RDAC unlock successful: ");
        Serial.println(read_from_ad5274);
    } 
    else {
        Serial.print("RDAC unlock failed: ");
        Serial.println(read_from_ad5274);
    }


    data_16_to_write = map(resistancePercent, 0, 100, 0, 1023);
    Serial.println(data_16_to_write, HEX);

    status += write_and_read_rdac (data_16_to_write);
    if (status > 0) {
        Serial.print(" RDAC read/write errors: ");
        Serial.println(status);
    }
}
/* json parser for resistance settings */

/* AD5274 functions ------------------------------------------ */


/* top level functions ------------------------------------------ */
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
/* top level functions ------------------------------------------ */
