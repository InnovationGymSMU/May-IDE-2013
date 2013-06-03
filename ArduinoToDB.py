import serial
from sys import exit
import sys
import os
import requests
from datetime import datetime
import time

weathPort = serial.Serial()
ser = serial.Serial()

weatherReadable = False
arduinoReadable = False

## Weather Station Port
try:
    weathPort = serial.Serial('/dev/serial/by-id/usb-FTDI_FT232R_USB_UART_A901L8L0-if00-port0', 9600)
    weathPort.close()
    weathPort.open()
except serial.SerialException:
    print "Could not open weather port."
else:
    weatherReadable = True

## Arduino Port
try:
    ser = serial.Serial('/dev/ttyACM0', 115200)
    ser.close()
    ser.open()
except serial.SerialException:
    print "Could not open Arduino port."
else:
    arduinoReadable = True
#stabalize input
time.sleep(15)
ser.flushInput()
print "starting"
ser.flushInput()
weathPort.flushInput()
while True:
	#print arduinoReadable
    if arduinoReadable:
        #Get values from Arduino
        try:
                from_serial = ser.readline()
        except serial.SerialException:
                print "Arduino read error"
        else:
            #print from_serial
            if len(from_serial) > 60:
                print from_serial
                #values = from_serial.split('*')
                #Split values using * as delimiter
                values = from_serial.split('*')
                if int(values[1]) != 3:
                    nID = values[1]
                    micVal = []
                    #recreate mic data as int
                    micVal.append(int(values[2]) | (int(values[3]) * 256))
                    micVal.append(int(values[4]) | (int(values[5]) * 256))
                    micVal.append(int(values[6]) | (int(values[7]) * 256))
                    for x in range(0, 3):
                        sID = x + 1
                        sVal = micVal[x]
                        payload= {'node_id' : str(nID), 'sensor_id' : str(sID), 'sensor_value' : str(sVal)}
                        try:
                            r = requests.get(r'http://ec2-72-44-42-85.compute-1.amazonaws.com/bcWorkshop/inputData.php', params=payload)
    	                except requests.RequestException:
            	            print "Request Exception"
                        except requests.ConnectionError:
                            print "Connection Error"
                        #else:
                                #if r.text != 1:
                                        #print "Connection down"
                    #light sensor
                    sID = 6;
                    sVal = int(values[8])
                    payload= {'time' : time, 'node_id' : nID, 'sensor_id' : sID, 'sensor_value' : sVal}
                    try:
                        r = requests.get(r'http://ec2-72-44-42-85.compute-1.amazonaws.com/bcWorkshop/inputData.php', params=payload)
                    except requests.RequestException:
                       print "Request Exception"
                    except requests.ConnectionError:
                        print "Connection Error"
        
