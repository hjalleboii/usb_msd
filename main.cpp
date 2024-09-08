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
  printf("FAT12: %u\n",fs.Mount());
  fs.Format("TINYCOC",B512,1,false);
  //fs.CreateDir("minskars","txt",{0});
  FileHandle fff;
  int cfr = fs.CreateFile("BAJSKK","TXT",{0},&fff);

  printf("Create File: %i\n",cfr);
  int cdir = fs.CreateDir("TEST","BBB",{0});
  PRINT_i(cdir);
  for(unsigned int i = 0; i < 10; i++){
    fs.SectorSerialDump(i);
  }
  

  tud_init(BOARD_TUD_RHPORT);

  


  
  gpio_init(0);
  gpio_set_dir(0,GPIO_IN);
  gpio_pull_up(0);
  bool done = false;
  bool pressed = false;
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
    if(!gpio_get(0)){
      if(!pressed){
        fs.SectorSerialDump(1);
        fs.DeleteFile(&fff);
        send_host_update();
      }
    
      board_led_write(1);
      pressed = true;
    }else{
      pressed = false;
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

