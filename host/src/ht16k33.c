#include "hardware/gpio.h"
#include "hardware/i2c.h"
                     //0   1     2     3     4     5     6     7     8     9
uint8_t table[] = { 0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07, 0x7f, 0x67}; 

void i2c_write_byte(uint8_t val) {
    i2c_write_blocking(i2c0, 0x70, &val, 1, false);}

void ht16k33_display(uint8_t x, uint8_t y) {
   uint8_t buf[11];
    uint8_t x_tens, x_ones, y_tens, y_ones;	

if (x > 9){
	x_tens = x / 10;
	x_ones = x - (x_tens * 10);
} else {
	x_tens = 0;
	x_ones = x;
}

if (y > 9){
	y_tens = y / 10;
	y_ones = y - (y_tens * 10);
} else {
	y_tens = 0;
	y_ones = y;
}

buf[0]=0;
buf[1]=table[x_tens];
buf[2]=table[x_tens];
buf[3]=table[x_ones];
buf[4]=table[x_ones];
buf[5]=0;
buf[6]=0;
buf[7]=table[y_tens];
buf[8]=0;
buf[9]=table[y_ones];
buf[10]=0;
i2c_write_blocking(i2c0,0x70,buf,count_of(buf),false);
} 

void ht16k33_init() {
	
    // init i2c0 400khz on pins 16 and 17
    i2c_init(i2c0,400*1000);
    gpio_set_function(16,GPIO_FUNC_I2C); gpio_set_function(17,GPIO_FUNC_I2C);
    gpio_pull_up(16); gpio_pull_up(17);
	
    // init ht16k33 see pico i2c example
    i2c_write_byte(0x21);
    i2c_write_byte(0xA0);
    i2c_write_byte(0x80 | 0x1);
}
