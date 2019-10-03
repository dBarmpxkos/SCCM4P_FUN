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
/* helper functions */

/*
 * Setup a manual ramp from a Start frequency to a Stop frequency in some increment
 * over a ramp time. 
 */
void IncrementFrequencyTest (const float startHz, const float stopHz, const float incHz, const float sweepTimeSec ) {
 
    // Calculate the delay between each increment.
    uint16_t numMsecPerStep = (sweepTimeSec * 1000.0) / ((uint16_t)((stopHz - startHz) / incHz) + 1);
    if ( numMsecPerStep == 0 ) numMsecPerStep = 1;

    // Apply a signal to the output. If phaseReg is not supplied, then
    // a phase of 0.0 is applied to the same register as freqReg
    gen.ApplySignal(SINE_WAVE,REG1,startHz);

    while ( true ) {
      
        gen.SetFrequency(REG1,startHz-incHz);

        for ( float i = startHz ; i <= stopHz; i += incHz ) {
            yield_on_char();
            gen.IncrementFrequency(REG1,incHz);
            delay(numMsecPerStep); 
        }
    }
}

/*
 * Cycle through all of the waveform types. Also cycle the 
 * frequency registers.
 */
void CycleWaveformsTest ( void ) {
  
    WaveformType waveType = SINE_WAVE;
    gen.SetFrequency(REG0,10000.0);   // Load values
    gen.SetFrequency(REG1,1000.0);
    // We don't care about phase for this test
    
    while ( true ) {

        gen.SetWaveform(REG1,waveType);   // Next waveform
        gen.SetWaveform(REG0,waveType);
        gen.SetOutputSource(REG1);        // Output 1000 Hz waveform

        // Hack to allow I'm alive lamp a chance to blink and give a better
        // response to user input
        
        gen.SetOutputSource(REG0);        // Output 10000 Hz waveform
        

        switch ( waveType ) {             // Cycle through all the waveform types
            case SINE_WAVE:
                waveType = TRIANGLE_WAVE;
                break;
            case TRIANGLE_WAVE:
                waveType = SQUARE_WAVE;
                break;
            case SQUARE_WAVE:
                waveType = HALF_SQUARE_WAVE;
                break;
            case HALF_SQUARE_WAVE:
                waveType = SINE_WAVE;
                break; 
        }
    }    
}


/*
 * Fast switching example.
 * I use the FFT display capability on my scope
 */
void SwitchFrequencyRegisterTest ( void ) {

    gen.ApplySignal(SINE_WAVE,REG0,500000);
    gen.ApplySignal(SINE_WAVE,REG1,100000);
    gen.SetPhase(REG1,180);           // Offset second freq by 180 deg
    gen.Reset();

    while ( true ) {                  // This takes time
        
        yield_on_char();                 // This takes more time

        gen.SetOutputSource(REG0);    // This takes about 18 usec
        gen.SetOutputSource(REG1);    // This takes about 18 usec  
        
        // What ends up is REG0 frequency is active a shorter amount of time
        // then REG1 frequency. In the sepctrum, the duty cycle differences will
        // show up (power is lower by 10log(DC))
    }  
}


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

/*
bool serial_switcher(const char choice){
    switch ( choice ) {
        case '1':
            IncrementFrequencyTest();
            break;
        case '2':
            CycleWaveformsTest();
            break;  
        case '3':
            SwitchFrequencyRegisterTest();
            break; 
        case '4':
            PhaseTest();
            break;
        case '5':
            RequestedvsProgrammedValues();
            break;
        case '6':
            outputOn = ! outputOn;
            gen.EnableOutput(outputOn);    // Turn off output
            break;
        default:
            Serial.println(F("*** Invalid command ***"));
            break;                    
    } 
}
*/
/* helper functions */