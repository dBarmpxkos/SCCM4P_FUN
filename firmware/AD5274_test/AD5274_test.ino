#include <Systronix_AD5274.h>

Systronix_AD5274 AD5274(AD5274_BASE_ADDR_FLOAT);


int16_t read_from_ad5274 = 0;
uint16_t data_16_to_write = 0;
int8_t status = 0;

void setup() {
	  Serial.begin(115200);
	  AD5274.begin();
}


void loop() {

	// unlock RDAC
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

	// write and then read RDAC register
	data_16_to_write = 0x000;
	status += write_and_read_rdac (data_16_to_write);
	delay(2000);

	while (true) {

		data_16_to_write += 4;
		Serial.println(data_16_to_write);
		status += write_and_read_rdac (data_16_to_write);
		delay(100);

		if (status > 0)	{
			Serial.print(" RDAC read/write errors: ");
			Serial.println(status);
		}

	}

}

/* RDAC register for resistance setting */
int8_t write_and_read_rdac (uint16_t data_16_to_write){
    int8_t status = 0;

    status += AD5274.command_write(AD5274_RDAC_WRITE, data_16_to_write);
    read_from_ad5274 = AD5274.command_read(AD5274_RDAC_READ, 0x00);
    return status;
}
/* RDAC register for resistance setting */
