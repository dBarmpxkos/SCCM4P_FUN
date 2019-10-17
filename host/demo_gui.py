import serial
import serial.tools.list_ports as list_ports
import string

# CH340 product ID
USB_product_ID 	= 29987
device = None

def get_com_list():
	ports = list(serial.tools.list_ports.comports())
	for p in ports:
		if p.pid == USB_product_ID:
			return p.device

def serial_init(port, device):
	counter = 0
	if port is not None:
		while hasattr(device, 'close') is False:
			try:
				device = serial.Serial(port, 9600, timeout= 2)
				print('GREAT SUCCESS!!')
			except serial.SerialException: 
				print('Trying to connect')
				time.sleep(0.1)
				counter += 1
				if counter == 2:
					print('Check device selection')
					break
		return device


serial_init(get_com_list(), device)