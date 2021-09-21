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

```
Usage:  
  led [options]  
      -r </tab>  reads the entire SX1509 register space  
      -w <addr> <data...>  hex, write data to address  
      -l <leds>            light leds [ABXYLR] or 0 for all off  
      -i <khz>             init and breathe leds (i2c speed in khz)  
  Examples (run with sudo):  
      led -i 100           init and breathe leds, i2c 100 khz  
      led -l BAR           light up buttons BAR  
      led -l 0             all led buttons off  
      led -r               read all registers  
      led -w 04 2a ff      write 2a ff starting at address 04  
```

## How do I get my button LEDs to light only the buttons used for each game?
```
You can run your own code at game-start and game-end by creating these two files:  
    */opt/retropie/configs/all/runcommand-onstart.sh*  
    */opt/retropie/configs/all/runcommand-onend.sh*  
These receive the game information as arguments. The onstart script looks for a simple text 
file '/home/pi/led/config' that contains game emulator, game name and button LEDs to light up. 
See the scripts and the simple text configuration file in this repo.  
The format is:  
    <emulator name> <rom name without extension> <leds to light>
    example:   arcade 1943 AB
The <led> options are: [ABXYLR] or 0
arcade name and leds are case in-sensitive, file name is case sensitive
```

## How do I get the LED “breathe” sequence to run while the machine is loading?
```
Start the LED “breathe” sequence before splash screen and stop after 
by calling this LED control application as described here:  
1. Edit */opt/retropie/supplementary/splashscreen/asplashscreen.sh*  
2. Just before the “if” block (that shows the video/image) add:   
   - */home/pi/led/led -i 200 &*   # run led breathe sequence while loading  
3. After that block (after “fi”), add:   
   - */home/pi/led/led -l BA*   # stop sequence and light the select & back buttons  
4. Reboot and verify operation  
```
