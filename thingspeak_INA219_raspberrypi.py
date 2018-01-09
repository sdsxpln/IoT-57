import sys
import RPi.GPIO as GPIO
from time import sleep
from ina219 import INA219
import urllib2

ina = INA219(shunt_ohms=0.1,
             max_expected_amps=0.6,
             address=0x40)

ina.configure(voltage_range=ina.RANGE_16V,
              gain=ina.GAIN_AUTO,
              bus_adc=ina.ADC_128SAMP,
              shunt_adc=ina.ADC_128SAMP)


myAPI = "1F5QMYOLT8HZRK7W"
def getSensorData():
    i = ina.current()
    v = ina.voltage()
    p = ina.power()
   return (str(v), str(i), str(p))
def main():
   print 'starting...'
   baseURL = 'https://api.thingspeak.com/update?api_key=%s' % myAPI
   while True:
       try:
           v, i ,p = getSensorData()
           f = urllib2.urlopen(baseURL +
                               "&field1=%s&field2=%s" % (v, i, p))
           print f.read()
           f.close()
           sleep(5)
       except:
           print 'exiting.'
           break
# call main 
if __name__ == '__main__':
   main()
