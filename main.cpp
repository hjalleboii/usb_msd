/* 
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Ha Thach (tinyusb.org)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "bsp/board.h"
#include "tusb.h"
#include "PrintfMacros.h"
#include "button.h"
//--------------------------------------------------------------------+
// MACRO CONSTANT TYPEDEF PROTYPES
//--------------------------------------------------------------------+

/* Blink pattern
 * - 250 ms  : device not mounted
 * - 1000 ms : device mounted
 * - 2500 ms : device is suspended
 */
enum  {
  BLINK_NOT_MOUNTED = 250,
  BLINK_MOUNTED = 1000,
  BLINK_SUSPENDED = 2500,
};

static uint32_t blink_interval_ms = BLINK_NOT_MOUNTED;

#include "FAT12.h"


extern uint8_t msc_disk0[64][512];

extern "C"{
void send_host_update(void);
}
void led_blinking_task(void);

/*------------- MAIN -------------*/
int main(void)
{
  board_init();
  stdio_init_all();
  
 

  //fs.Format("TinyUSB 0  ",B512,1,false);
  // init device stack on configured roothub port

  FAT12 fs((uint8_t*)msc_disk0,sizeof(msc_disk0));
  fs.Format("TINYCOC",B512,1,false);
  fs.Mount();
  //fs.CreateDir("minskars","txt",{0});
  FileHandle fff;
  //auto cfr = fs.CreateFile("BAJSKK","TXT",{0},&fff);

  fs.CreateLongFileNameEntry("ingen seger utan varan neger.txr",32,{0},&fff);
  PRINT_i(fff.dirindex);
  PRINT_i(fff.direntry);
  //fs.DeleteFile(fff);
  //printf("Create File: %i\n",cfr);
  FileHandle dir;

  
  

  tud_init(BOARD_TUD_RHPORT);

  
  
  RP2040_Button_init(0);
  RP2040_Button_init(1);
  
  bool done = false;
  bool pressed = false;
  bool pressed2 = false;
  while (1)
  {
    tud_task(); // tinyusb device task
    /*if(!gpio_get(12) && !done)
    { 
      done = true;
      board_led_on();

    }else{
      board_led_off();
      done = false;
    }*/
    if(RP2040_Button_get(0)){
      if(!pressed){
        for(unsigned int i = 0; i <8; i++){
          fs.SectorSerialDump(i);
        }

        //fs.DeleteFile(&fff);
        //send_host_update();
      }
    
      board_led_write(1);
      pressed = true;
    }else{
      pressed = false;
    }
    if(RP2040_Button_get(1)){
      if(!pressed2){
/*
        auto file_res = fs.Open(fff,FILE_MODE_READ);
        fs.SectorSerialDump(3);
        if(file_res.Ok()){
          file_res.val;

          uint8_t a=0;
          memcpy(&a,fs.disk+1536, 1);

          printf("what %u\n",a);
          fs.Close(&file_res.val);
        }*/
      }
    
      board_led_write(1);
      pressed2 = true;
    }else{
      pressed2 = false;
    }
    led_blinking_task();
  }

  return 0;
}

//--------------------------------------------------------------------+
// Device callbacks
//--------------------------------------------------------------------+

// Invoked when device is mounted
void tud_mount_cb(void)
{
  blink_interval_ms = BLINK_MOUNTED;
}

// Invoked when device is unmounted
void tud_umount_cb(void)
{
  blink_interval_ms = BLINK_NOT_MOUNTED;
}

// Invoked when usb bus is suspended
// remote_wakeup_en : if host allow us  to perform remote wakeup
// Within 7ms, device must draw an average of current less than 2.5 mA from bus
void tud_suspend_cb(bool remote_wakeup_en)
{
  (void) remote_wakeup_en;
  blink_interval_ms = BLINK_SUSPENDED;
}

// Invoked when usb bus is resumed
void tud_resume_cb(void)
{
  blink_interval_ms = BLINK_MOUNTED;
}

//--------------------------------------------------------------------+
// BLINKING TASK
//--------------------------------------------------------------------+
void led_blinking_task(void)
{
  static uint32_t start_ms = 0;
  static bool led_state = false;

  // Blink every interval ms
  if ( board_millis() - start_ms < blink_interval_ms) return; // not enough time
  start_ms += blink_interval_ms;

  board_led_write(led_state);
  led_state = 1 - led_state; // toggle
}

