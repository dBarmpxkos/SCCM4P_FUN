#include <AD9833.h>        

AD9833 gen(FNC_PIN);

void setup(){
	Serial.begin(9600);

	gen.Begin();
	gen.EnableOutput(false);

}

void loop()
{

}

/*
 * Setup a manual ramp from a Start frequency to a Stop frequency in some increment
 * over a ramp time. 
 */
void IncrementFrequencyTest ( void ) {

    float startHz = 1000, stopHz = 5000, incHz = 1, sweepTimeSec = 5.0;
 
    // Calculate the delay between each increment.
    uint16_t numMsecPerStep = (sweepTimeSec * 1000.0) / ((uint16_t)((stopHz - startHz) / incHz) + 1);
    if ( numMsecPerStep == 0 ) numMsecPerStep = 1;

    // Apply a signal to the output. If phaseReg is not supplied, then
    // a phase of 0.0 is applied to the same register as freqReg
    gen.ApplySignal(SINE_WAVE,REG1,startHz);

    while ( true ) {
      
        gen.SetFrequency(REG1,startHz-incHz);

        for ( float i = startHz ; i <= stopHz; i += incHz ) {
            YIELD_ON_CHAR
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
        DELAY_WITH_YIELD
        
        gen.SetOutputSource(REG0);        // Output 10000 Hz waveform
        
        DELAY_WITH_YIELD

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
        
        YIELD_ON_CHAR                 // This takes more time

        gen.SetOutputSource(REG0);    // This takes about 18 usec
        gen.SetOutputSource(REG1);    // This takes about 18 usec  
        
        // What ends up is REG0 frequency is active a shorter amount of time
        // then REG1 frequency. In the sepctrum, the duty cycle differences will
        // show up (power is lower by 10log(DC))
    }  
}

