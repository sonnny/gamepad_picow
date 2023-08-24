
#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "hid_gamepad.h"

int main(){
  stdio_init_all();
  cyw43_arch_init();
  
  bt_main();
  gap_set_local_name("PicoW Gamepad");
  for(;;){
    send_report_joystick();
    sleep_ms(100);
  }
  
  return 0;
  
}
