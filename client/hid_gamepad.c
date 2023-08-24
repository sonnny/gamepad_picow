//https://github.com/fluffymadness/btstack/blob/develop/example/hid_gamepad_example.c

//
// datasheet for directional joystick
// https://datasheet.lcsc.com/lcsc/1811151552_ALPSALPINE-RKJXM1015004_C97432.pdf
//
// picow pin connection to directional joystick
//   gpio2 - A
//   gpio3 - B
//   gpio4 - C
//   gpio5 - D
//   gpio6 - push (see datasheet)
//
//   buttons
//   gpio7 - front right button
//   gpio8 - front left button
//
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "hardware/adc.h"
#include "btstack.h"
#include "hid_gamepad.h"

//#define BTSTACK_FILE__ "hid_gamepad.c"

static const char hid_device_name[] = "Bluetooth Gamepad";
static const char service_name[] = "Wireless Gamepad";
static uint8_t hid_service_buffer[250];
static uint8_t device_id_sdp_service_buffer[100];
static btstack_packet_callback_registration_t hci_event_callback_registration;
static uint16_t hid_cid;


const uint8_t hid_descriptor_gamepad[] = {
    0x05, 0x01,                   // USAGE_PAGE (Generic Desktop)
    0x09, 0x05,                   // USAGE (Game Pad)
    0xa1, 0x01,                   // COLLECTION (Application)
    0xa1, 0x02,                   //    COLLECTION (Logical)
    0x85, 0x30,                    //      REPORT_ID (48)
    
    0x75, 0x08,                   //      REPORT_SIZE (8)
    0x95, 0x02,                   //      REPORT_COUNT (2)
    0x05, 0x01,                   //      USAGE_PAGE (Generic Desktop)
    0x09, 0x30,                   //      USAGE (X)
    0x09, 0x31,                   //      USAGE (Y)
    0x15, 0x81,                   //      LOGICAL_MINIMUM (-127)
    0x25, 0x7f,                   //      LOGICAL_MAXIMUM (127)
    0x81, 0x02,                   //      INPUT (Data, Var, Abs)
    
    0x75, 0x01,                   //      REPORT_SIZE (1)
    0x95, 0x08,                   //      REPORT_COUNT (8)
    0x15, 0x00,                   //      LOGICAL_MINIMUM (0)
    0x25, 0x01,                   //      LOGICAL_MAXIMUM (1)
    0x05, 0x09,                   //      USAGE_PAGE (Button)
    0x19, 0x01,                   //      USAGE_MINIMUM (Button 1)
    0x29, 0x08,                   //      USAGE_MAXIMUM (Button 8)
    0x81, 0x02,                   //      INPUT (Data, Var, Abs)      
    0xc0,                         //   END_COLLECTION
    0xc0                          // END_COLLECTION
  };
typedef struct
{
  uint8_t x;
  uint8_t y;
  uint8_t buttons;
}
gamepad_report_t;

static gamepad_report_t joystick;

void send_report_joystick(void){

       adc_select_input(0);
        uint adc_x_raw = adc_read();
        adc_select_input(1);
        uint adc_y_raw = adc_read();
           
        const uint bar_width = 100; // scale reading to 1..100
        const uint adc_max = (1 << 12) - 1;
        joystick.x = adc_x_raw * bar_width / adc_max; // center 45..50
        joystick.y = adc_y_raw * bar_width / adc_max; // center 45..50
       // printf("x: %u\n",x);
       // printf("y: %u\n\n",y);
        
        // check buttons
        joystick.buttons=0;
        int mask=0;
        int sum=0;
        for(int i=2; i<10; i++){ // picow gpio
		  switch(i){
		    case 2: mask = 0x01; break;
		    case 3: mask = 0x02; break;
		    case 4: mask = 0x04; break;
		    case 5: mask = 0x08; break;
		    case 6: mask = 0x10; break;
		    case 7: mask = 0x20; break;
		    case 8: mask = 0x40; break;
		    case 9: mask = 0x80; break;}
		    
          if (!gpio_get(i)) joystick.buttons += mask;}
          
       // printf("button status: %u\n",buttons);
          
       // if(!gpio_get(20)) printf("button: 20 pressed!\n");   
    
    
    uint8_t report[] = {0xa1, 0x30,joystick.x, joystick.y,joystick.buttons};  
    //send report
    hid_device_send_interrupt_message(hid_cid, &report[0], sizeof(report));
    hid_device_request_can_send_now_event(hid_cid);
}

static void packet_handler(uint8_t packet_type, uint16_t channel, uint8_t * packet, uint16_t packet_size){
    UNUSED(channel);
    UNUSED(packet_size);
    uint8_t status;
     if(packet_type == HCI_EVENT_PACKET)
    {
        switch (packet[0]){
            case BTSTACK_EVENT_STATE:
                    if (btstack_event_state_get_state(packet) != HCI_STATE_WORKING) return;
                    break;

            case HCI_EVENT_USER_CONFIRMATION_REQUEST:
                    // ssp: inform about user confirmation request
                    log_info("SSP User Confirmation Request with numeric value '%06"PRIu32"'\n", hci_event_user_confirmation_request_get_numeric_value(packet));
                    log_info("SSP User Confirmation Auto accept\n");
                    break;
            case HCI_EVENT_HID_META:
                switch (hci_event_hid_meta_get_subevent_code(packet)){
                    case HID_SUBEVENT_CONNECTION_OPENED:
                        status = hid_subevent_connection_opened_get_status(packet);
                        if (status) {
                            // outgoing connection failed
                            printf("Connection failed, status 0x%x\n", status);
                            hid_cid = 0;
                            return;
                        }
                        hid_cid = hid_subevent_connection_opened_get_hid_cid(packet);
                        log_info("HID Connected\n");
              ////// turn on led here
                        cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN,1);
                        hid_device_request_can_send_now_event(hid_cid);
                        break;
                    case HID_SUBEVENT_CONNECTION_CLOSED:
                        log_info("HID Disconnected\n");
                        hid_cid = 0;
                        break;
                    case HID_SUBEVENT_CAN_SEND_NOW:  
                        if(hid_cid!=0){ //Solves crash when disconnecting gamepad on android
                         // send_report_joystick();
                         // hid_device_request_can_send_now_event(hid_cid);
                        }
                        break;
                    default:
                        break;
                }
                break;
            default:
                break;
        }
    }
}

void bt_main(void){

    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN,0);

    // init adc
    adc_init();
    adc_gpio_init(26);
    adc_gpio_init(27);
    
    // init buttons
    for(int i=2; i<10; i++){
	  gpio_init(i); gpio_set_dir(i,GPIO_IN); gpio_pull_up(i);}
	  
	gpio_init(20); gpio_set_dir(20,GPIO_IN); gpio_pull_up(20);

    gap_discoverable_control(1);
    gap_set_class_of_device(0x2508);
    gap_set_local_name(hid_device_name);

    // L2CAP
    l2cap_init();
    // SDP Server
    sdp_init();
    memset(hid_service_buffer, 0, sizeof(hid_service_buffer));
    
    hid_sdp_record_t hid_params={
	  0x2508, 33,
	  0,1,
	  1,1,
      0,
      0xFFFF, 0xFFFF, 3200,
	  hid_descriptor_gamepad,
	  sizeof(hid_descriptor_gamepad),
	  hid_device_name
	};
    
    
    hid_create_sdp_record(hid_service_buffer, 0x10000, &hid_params);
    //hid_create_sdp_record(hid_service_buffer, 0x10000, 0x2508, 0, 0, 0, 0, reportMap, sizeof(reportMap), service_name);
    sdp_register_service(hid_service_buffer);

     // HID Device
    hid_device_init(0, sizeof(hid_descriptor_gamepad), hid_descriptor_gamepad);

    hci_event_callback_registration.callback = &packet_handler;
    hci_add_event_handler(&hci_event_callback_registration);

    hid_device_register_packet_handler(&packet_handler);

    // turn on!
    hci_power_control(HCI_POWER_ON);
    return 0;
}
