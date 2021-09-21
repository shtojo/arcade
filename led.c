/*
Shawn's SX1509 LED control

Install bcm2835 library on rpi:
  Get latest "bcm2835-1.xx.tar.gz" file from www.airspayce.com/mikem/bcm2835/
  Copy to /home/pi then "tar zxvf bcm2835-1.68.tar.gz"  (creates subfolder)
  cd bcm2835-1.68/ then
     ./configure
     make
     sudo make check
     sudo make install

Compile led.c with: gcc -Wall led.c -o led -l bcm2835

Remember to enable i2c in rasp-config
Uses I2C port on rpi pins 3 (SDA) and 5 (SCL)

Button to IO map: A = IO-4, B = IO-5, X = IO-6, Y = IO-7, L = IO-12, R = IO-13
*/

#define SLA 0x3e;
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <bcm2835.h>

static const char* usage =
"   Usage: led [options]\n"
"      -r                   reads the entire SX1509 register space\n"
"      -w <addr> <data...>  hex, write data to address\n"
"      -l <leds>            light leds [ABXYLR] or 0 for all off\n"
"      -i <khz>             init and breathe leds (i2c speed in khz)\n"
"   Examples (run with sudo):\n"
"      led -i 100           init and breathe leds, i2c 100 khz\n"
"      led -l BAR           light up buttons BAR\n"
"      led -l 0             all led buttons off\n"
"      led -r               read all registers\n"
"      led -w 04 2a ff      write 2a ff starting at address 04\n\n";

void init(int khz);
void light_leds(char *leds);
void led_config(char on, char off, char risefall);
void led_on(char led);
void led_off(char led);
void check_rc(int rc);
void usage_error(void);
int toint(char *str, int base, int min, int max);

char *command;

int main(int argc, char *argv[])
{
    char buf[0x80];
    command = *argv;

    if(geteuid() != 0)
    {
        printf("Please run with sudo for root privileges.\n\n");
        return 1;
    }

    if (!bcm2835_init())
    {
        printf("bcm2835_init error!\n");
        return 1;
    }

    if (!bcm2835_i2c_begin())
    {
        printf("bcm2835_i2c_begin error! is i2c enabled?\n");
        return 1;
    }

    // Set the library to use slave address 0x3e (default for SX1509)
    bcm2835_i2c_setSlaveAddress(0x3e);

    // READ
    if (strcmp(argv[1], "-r") == 0)
    {
        if (argc != 2) usage_error();
        char adr[1] = {0};
        printf("Reading SX1509...");
        check_rc(bcm2835_i2c_read_register_rs(adr, buf, 0x80));
        for (int i = 0; i < 0x80; i++)
        {
            if (i % 8 == 0) printf("\n%02X: ", i);
            printf("%02X ", buf[i] & 0xff);
        }
        printf("\n\n");
    }
    // WRITE
    else if (strcmp(argv[1], "-w") == 0)
    {
        if (argc < 3) usage_error();
        for (int i = 0; i < argc - 2; i++)
        {
            buf[i] = toint(argv[i+2], 16, 0x00, 0xff);
            if (i == 0) printf("Writing %02X: ", buf[i]);
            else printf("%02X ", buf[i]);
        }
        check_rc(bcm2835_i2c_write(buf, argc-2));
    }
    // LEDS
    else if (strcmp(argv[1], "-l") == 0)
    {
        if (argc != 3) usage_error();
        light_leds(argv[2]);
    }
    // INIT
    else if (strcmp(argv[1], "-i") == 0)
    {
        if (argc != 3) usage_error();
        int khz = toint(argv[2], 10, 0, 1000);
        init(khz);
    }
    else
    {
        usage_error();
    }

    bcm2835_close();
    return 0;
}

// initializes and leaves leds breathing
void init(int khz)
{
    char rst[2] = {0x7d, 0x12};
    char buf[39] = {0x00,
        0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // input buffers off
        0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, // open drain outputs
        0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, // outputs undriven
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x50, 0xb8, // 500 khz pwm, log
        0xff, 0xff, 0x00, 0x00, 0x00, 0x10};            // pwm, keypad engine sleep

    printf("initializing led controller...");
    bcm2835_i2c_set_baudrate(khz * 1000);

    // reset the SX1509
    check_rc(bcm2835_i2c_write(rst,2));
    rst[1] = 0x34;
    check_rc(bcm2835_i2c_write(rst,2));
    bcm2835_delay(100);

    // initialize the SX1509
    check_rc(bcm2835_i2c_write(buf, 39));

    // on = 2 = 131 ms, off time = 15 = 98 ms, risefall = 8 = 2.08 s
    led_config(2, 10, 8);

    // led timing is controlled by when the output enable registers are written (0x10,0x11)
    // make leds visible, stagger
    led_on('B');
    led_on('Y');
    bcm2835_delay(300);
    led_on('A');
    led_on('X');
    bcm2835_delay(300);
    led_on('L');
    led_on('R');
}

void light_leds(char *leds)
{
    int i;
    int led_count = strlen(leds);
    char buf[3] = {0x10, 0xff, 0xff};
    check_rc(bcm2835_i2c_write(buf, 3));
    led_config(0, 0, 1);  // static control with fast rise/fall

    // do quick pattern across all leds
    bcm2835_delay(100);
    led_on('B');
    led_on('Y');
    bcm2835_delay(100);
    led_on('A');
    led_on('X');
    bcm2835_delay(100);
    led_on('L');
    led_on('R');
    led_off('B');
    led_off('Y');
    bcm2835_delay(100);
    led_off('A');
    led_off('X');
    bcm2835_delay(100);
    led_off('L');
    led_off('R');
    bcm2835_delay(200);

    // blink the selected buttons then leave on
    if (strcmp(leds, "0") != 0)
    {
        for (i = 0; i < led_count; i++) led_on(leds[i]);
        bcm2835_delay(200);
        for (i = 0; i < led_count; i++) led_off(leds[i]);
        bcm2835_delay(200);
        for (i = 0; i < led_count; i++) led_on(leds[i]);
    }
}

// Turn on one LED, leave others as is (read-modify-write)
void led_on(char led)
{
    // A = IO-4, B = IO-5, X = IO-6, Y = IO-7, L = IO-12, R = IO-13
    char buf[3] = {0x10, 0x00, 0x00};
    check_rc(bcm2835_i2c_read_register_rs(buf, buf+1, 2));  // read existing
    if (led == 'A') buf[2] &= 0xef;
    else if (led == 'B') buf[2] &= 0xdf;
    else if (led == 'X') buf[2] &= 0xbf;
    else if (led == 'Y') buf[2] &= 0x7f;
    else if (led == 'L') buf[1] &= 0xef;
    else if (led == 'R') buf[1] &= 0xdf;
    else usage_error();
    check_rc(bcm2835_i2c_write(buf, 3));
}

// Turn on one LED, leave others as is (read-modify-write)
void led_off(char led)
{
    // A = IO-4, B = IO-5, X = IO-6, Y = IO-7, L = IO-12, R = IO-13
    char buf[3] = {0x10, 0x00, 0x00};
    check_rc(bcm2835_i2c_read_register_rs(buf, buf+1, 2));  // read existing
    if (led == 'A') buf[2] |= 0x10;
    else if (led == 'B') buf[2] |= 0x20;
    else if (led == 'X') buf[2] |= 0x40;
    else if (led == 'Y') buf[2] |= 0x80;
    else if (led == 'L') buf[1] |= 0x10;
    else if (led == 'R') buf[1] |= 0x20;
    else usage_error();
    check_rc(bcm2835_i2c_write(buf, 3));
}

// Configure all leds in a single operation
void led_config(char on, char off, char risefall)
{
    char buf[43] = {0x35,
        on, 0xff, off << 3, risefall, risefall,  // led 4
        on, 0xff, off << 3, risefall, risefall,  // led 5
        on, 0xff, off << 3, risefall, risefall,  // led 6
        on, 0xff, off << 3, risefall, risefall,  // led 7
        0x00, 0xff, 0x00,
        0x00, 0xff, 0x00,
        0x00, 0xff, 0x00,
        0x00, 0xff, 0x00,
        on, 0xff, off << 3, risefall, risefall,  // led 12
        on, 0xff, off << 3, risefall, risefall}; // led 13
    check_rc(bcm2835_i2c_write(buf, 43));
}

// Checks i2c operation return value, show message and abort if error
// rc - return code from bcm2835 library call
void check_rc(int rc)
{
    switch(rc) {
    case BCM2835_I2C_REASON_OK:
        return;
    case BCM2835_I2C_REASON_ERROR_NACK:
        printf("no response\n\n");
        break;
    case BCM2835_I2C_REASON_ERROR_DATA :
        printf("data transfer error\n\n");
        break;
    case BCM2835_I2C_REASON_ERROR_CLKT :
        printf("clock-stretch timeout\n\n");
        break;
    default:
        printf("unknown error!\n\n");
    }
    bcm2835_close();
    exit(1);  // abort if error
}

// Print usage message and abort
void usage_error(void)
{
    printf("Bad command: %s", command);
    printf(usage);
    bcm2835_close();
    exit(1);
}

// Converts numeric string to value (positive only)
// str - string to convert
// base - base (10 = dec, 16 = hex)
// min - minimum allowed value
// max - maximum allowed value
// returns: converted number or -1 if error
int toint(char *str, int base, int min, int max)
{
    if (!str) {
        fprintf(stderr, "Bad argument in toint (null string)!\n\n");
        return -1;
    }
    errno = 0;
    char *end = str;
    long int result = strtol(str, &end, base);
    if ((result < min) || (result > max) || (errno) || (*end)) result = -1;
    return result;
}

