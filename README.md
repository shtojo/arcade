# arcade
Arcade LED control software using SX1509

LED Control via SX1509 advanced LED controller.
For easy wiring you can use the $6.50 Sparkfun SX1509 mounted on a carrier board: https://www.sparkfun.com/products/13601
Tested with Raspberry Pi running RetroPie.

To build the "led" executable:

First, install bcm2835 library on rpi:
 Get latest "bcm2835-1.xx.tar.gz" file from www.airspayce.com/mikem/bcm2835/
 Copy to /home/pi then "tar zxvf bcm2835-1.68.tar.gz"  (creates subfolder)
 cd bcm2835-1.68/ then
    ./configure
    make
    sudo make check
    sudo make install

Then compile led.c with: 
    gcc -Wall led.c -o led -l bcm2835

Remember to enable i2c in raspi-config
Uses I2C port on rpi pins 3 (SDA) and 5 (SCL)

Button to IO map: A = IO-4, B = IO-5, X = IO-6, Y = IO-7, L = IO-12, R = IO-13

Usage: 
  led [options]
    -r                   reads the entire SX1509 register space
      -w <addr> <data...>  hex, write data to address
      -l <leds>            light leds [ABXYLR] or 0 for all off
      -i <khz>             init and breathe leds (i2c speed in khz)
   Examples (run with sudo):
      led -i 100           init and breathe leds, i2c 100 khz
      led -l BAR           light up buttons BAR
      led -l 0             all led buttons off
      led -r               read all registers
      led -w 04 2a ff      write 2a ff starting at address 04
  
  
  
