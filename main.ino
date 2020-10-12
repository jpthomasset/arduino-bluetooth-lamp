#include <SoftwareSerial.h>
#include "FastLED.h"


#define BT_NAME "Lampe_2"
#define BT_PIN "0406"
#define BT_INIT_DELAY 1000
#define BT_DELAY 500

/**********************/
/* Bluetooth commands */
/**********************/
#define CMD_NONE -1
#define CMD_LED       'L'  /* Set per led color (no device update)     "L<id_led:byte><R:byte><G:byte><B:byte>"                        */
#define CMD_LED_RANGE 'R'  /* Set incl. range color (no device update) "R<id_led_start:byte><id_led_end:byte><R:byte><G:byte><B:byte>" */
#define CMD_LED_ALL   'A'  /* Set all leds color (immediate effect)    "A<R:byte><G:byte><B:byte>"                                     */
#define CMD_OFF       'F'  /* Switch off all leds (immediate effect)   "F"                                                             */
#define CMD_SHOW      'S'  /* Update device from latest defined value  "S"                                                             */


#define BT_WAIT_CHAR(io) if(!waitForNextChar(io, 1000)) return
#define BT_READ_CHAR(io, chr) if(!waitForNextChar(io, 1000)) return; chr = io.read();
#define NUM_LEDS 5
#define LED_PIN 6

CRGB leds[NUM_LEDS];
SoftwareSerial hc06(2,3);


bool waitForNextChar(SoftwareSerial &io, int timeoutms) {
  int time = 0;
  while(!io.available()) {
    time = time + 10;
    if(time > timeoutms) {
      return false;   
    }
    delay(10);
  }
  return true;
  
}

bool checkResponse(SoftwareSerial &io, String expected) {
  int nbread;
  int index = 0;
  char c;
  while(waitForNextChar(io, 5000)) {
    c = io.read();

    if(c != expected[index]) {
      return false;      
    }
    
    index++;
    if(expected[index] == '\0') {
      return true;
    }
      
  }
  Serial.println("Timeout reading");
  return false;
}

int getCommand(SoftwareSerial &io) {
   if(io.available()) {
    return io.read();
   } else {
    return CMD_NONE;
   }
}


void setup(){
  //Initialize Serial Monitor
  Serial.begin(9600);
  
  //Initialize Bluetooth Serial Port
  hc06.begin(9600);
  delay(BT_INIT_DELAY);
  Serial.println("Initializing bluetooth name " BT_NAME);
  hc06.write("AT+NAME" BT_NAME);
  delay(BT_DELAY);
  if(!checkResponse(hc06, "OKsetname")) {
    Serial.println("Unable to set Bluetooth name");
    return;
  }
  Serial.println("Initializing bluetooth pin");
  hc06.write("AT+PIN" BT_PIN);
  delay(BT_DELAY);
  if(!checkResponse(hc06, "OKsetPIN")) {
    Serial.println("Unable to set Bluetooth pin");
    return;
  }
  Serial.println("Bluetooth initialized");

  // Initialize LED
  FastLED.addLeds<APA104, LED_PIN, GRB>(leds, NUM_LEDS);
}

void loop(){
  int cmd = getCommand(hc06);
  switch(cmd) {
    case CMD_NONE:
      break;
    
    case CMD_LED: {
      uint8_t ledid;
      BT_READ_CHAR(hc06, ledid);
      BT_READ_CHAR(hc06, leds[ledid].r);
      BT_READ_CHAR(hc06, leds[ledid].g);
      BT_READ_CHAR(hc06, leds[ledid].b);
    }
    break;

    case CMD_LED_RANGE: {
      uint8_t ledid, ledid_end, red, green, blue;
      BT_READ_CHAR(hc06, ledid);
      BT_READ_CHAR(hc06, ledid_end);
      BT_READ_CHAR(hc06, red);
      BT_READ_CHAR(hc06, green);
      BT_READ_CHAR(hc06, blue);

      for(;ledid<=ledid_end;ledid++) {
        leds[ledid].r = red;
        leds[ledid].g = green;
        leds[ledid].b = blue;
      }
    }
    break;
  
    case CMD_LED_ALL: {
      uint8_t red, green, blue;
      BT_READ_CHAR(hc06, red);
      BT_READ_CHAR(hc06, green);
      BT_READ_CHAR(hc06, blue);

      for(uint8_t ledid=0;ledid < NUM_LEDS;ledid++) {
        leds[ledid].r = red;
        leds[ledid].g = green;
        leds[ledid].b = blue;
      }  
      FastLED.show();
    }

    case CMD_OFF: {
      for(uint8_t ledid=0;ledid < NUM_LEDS;ledid++) {
        leds[ledid].r = 0;
        leds[ledid].g = 0;
        leds[ledid].b = 0;
      }  
      FastLED.show();
    }

    case CMD_SHOW: {
      Serial.println("Showing led");
      FastLED.show();
    }
    break;
      
  }
}
