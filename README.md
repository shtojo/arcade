# arcade
Arcade LED control software using SX1509

LED Control via SX1509 advanced LED controller.
For easy wiring you can use the $6.50 Sparkfun SX1509 mounted on a carrier board: https://www.sparkfun.com/products/13601
Tested with Raspberry Pi running RetroPie.

To build the "led" executable:

1. First, install bcm2835 library on rpi:
   - Get latest "bcm2835-1.xx.tar.gz" file from www.airspayce.com/mikem/bcm2835/
   - Copy to /home/pi then "tar zxvf bcm2835-1.68.tar.gz"  (creates subfolder)
   - cd bcm2835-1.68/ then
     - ./configure
     - make
     - sudo make check
     - sudo make install

2. Then compile led.c with: 
   - gcc -Wall led.c -o led -l bcm2835

Remember to enable i2c in raspi-config
Uses I2C port on rpi pins 3 (SDA) and 5 (SCL)

Button to IO map: A = IO-4, B = IO-5, X = IO-6, Y = IO-7, L = IO-12, R = IO-13

Usage:\n
  led [options]\n
    -r                   reads the entire SX1509 register space\n
    -w <addr> <data...>  hex, write data to address\n
    -l <leds>            light leds [ABXYLR] or 0 for all off\n
    -i <khz>             init and breathe leds (i2c speed in khz)\n
  Examples (run with sudo):\n
    led -i 100           init and breathe leds, i2c 100 khz\n
    led -l BAR           light up buttons BAR\n
    led -l 0             all led buttons off\n
    led -r               read all registers\n
    led -w 04 2a ff      write 2a ff starting at address 04\n
