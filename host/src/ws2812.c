// ws2812.c

#include <stdint.h>
#include <stdio.h>
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "ws2812.pio.h"

static uint32_t pixels[]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
static uint32_t previous_pixel = 0;

// stick direction
// up -            17
// down -          20
// left -          18
// right -         24
// upper left -    19
// upper right -   25
// lower left -    22
// lower right -   28
// center button - 16
// front right   - 32
// front left    - 64
uint8_t map_button(uint8_t button_value){
	uint8_t value=0;
	switch(button_value){
		// directional stick
		case 17: value=8; break;
		case 20: value=0; break;
		case 18: value=4; break;
		case 24: value=12; break;
		case 19: value=6; break;
		case 25: value=10; break;
		case 22: value=2; break;
		case 28: value=13; break;
		
		// buttons
		case 16: value=1; break;
		case 32: value=3; break;
		case 64: value=5; break;
	}
	return value;}

void display_pixel(uint32_t pixel_grb) {
    pio_sm_put_blocking(pio0, 0, pixel_grb << 8u);}

void update_pixels(){
	for(int i=0; i<16; i++) display_pixel(pixels[i]);}
	
void clear_pixels(){
	for(int i=0; i<16; i++) display_pixel(0);}	
	
void ws2812_init(){
   PIO pio = pio0;
   int sm = 0;
   uint offset = pio_add_program(pio, &ws2812_program);
   ws2812_program_init(pio, sm, offset, 15, 800000, false);
   update_pixels();}

void ws2812_update(uint8_t index){
  uint8_t temp;
  temp = map_button(index);
  if(previous_pixel != temp) pixels[previous_pixel]=0;
  if((temp == 1) || (temp == 3) || (temp == 5)) {pixels[temp]=0x00000008;}
  else {pixels[temp]=0x00000800;}
  update_pixels();
  previous_pixel = temp;
}
