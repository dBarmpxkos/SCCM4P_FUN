import serial
import serial.tools.list_ports as list_ports
import string
import time
import json 

# CH340 product ID
USB_product_ID 	= 29987
device 			= None

# times to try to connect to uart 
triesToConnect_UART = 10

# waveform settings / tokens
waveformToken = '>W'
waveformSettings = '{"reg":0,"en":1,"wave":1,"freq":10000.00,"ampl":0,"phase":0,"offset":0}'

def get_com_list():
	ports = list(serial.tools.list_ports.comports())
	for p in ports:
		if p.pid == USB_product_ID:
			return p.device

def serial_init(port, device):
	counter = 0
	if port:
		while hasattr(device, 'close') is False:
			try:
				device = serial.Serial(port, 9600, timeout= 5)
				print('uart succesfully initialized')
				while device.readline().decode("utf-8")[0] != '>':
					print('waiting token')
				time.sleep(2.5)
			except serial.SerialException: 
				print('trying to connect')
				time.sleep(0.1)
				counter += 1
				if counter == triesToConnect_UART:
					print('Check device selection')
					break
		return device


device = serial_init(get_com_list(), device)
device.write(waveformToken.encode())
device.write(waveformSettings.encode())
time.sleep(10)
waveformSettings = '{"reg":0,"en":1,"wave":0,"freq":500.00,"ampl":0,"phase":0,"offset":0}'
device.write(waveformToken.encode())
device.write(waveformSettings.encode())