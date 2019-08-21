//Status-led color codes:
//Purple: No Ethernet hardware detected.
//Red: No Ethernet cable connected.
//Orange: Not connected to server.
//Green: Connected to server, waiting for instructions.

#include <Ethernet.h>
#include <FastLED.h>
#include <SPI.h>

#define PIN 6
#define LED_COUNT 40
#define LED_COUNT_VIX 40

CRGB leds[LED_COUNT];

EthernetClient client;
byte clientMac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress clientIp(192, 168, 1, 200);
IPAddress serverIp(192, 168, 1, 25);
//IPAddress serverIp(192, 168, 1, 27);
int serverPort = 10002;

char startChecksum[] = "abc";
char endChechsum[] = "def";

boolean connected = false;
int statusLed = 0;

int curLed = 0;
boolean startReceived = false;
boolean toggle = false;

void setup() {
  //Initialize leds, use show() to turn off blue starting leds.
  FastLED.addLeds<WS2812B, PIN>(leds, LED_COUNT);
  leds[statusLed] = CRGB(0, 127, 255); //Cyan
  FastLED.show();

  leds[1] = CRGB(127, 0, 127); //Purple
  FastLED.show();
    
  //Start the Ethernet
  Ethernet.begin(clientMac, clientIp);

  leds[2] = CRGB(127, 0, 127); //Purple
  FastLED.show();

  //Check for Ethernet hardware to be present.
  if (Ethernet.hardwareStatus() == EthernetNoHardware) {
    leds[statusLed] = CRGB(127, 0, 127); //Purple
    FastLED.show();
    while (true) {
      delay(1); // do nothing, no point running without Ethernet hardware
    }
  }

  leds[3] = CRGB(127, 0, 127); //Purple
  FastLED.show();

  //Let the Ethernet Shield initialize for a moment.
  delay(1000); 
}

void loop() {
  if (ensureConnection()) {
    //loopTest();
    loopShow();
  }
}

void loopShow() {
  while (!startReceived && client.available() >= 3) {
    byte r = client.read();
    byte g = client.read();
    byte b = client.read();
    
    if (r == startChecksum[0] && g == startChecksum[1] && b == startChecksum[2]) {
      startReceived = true;
      curLed = 0;
      break;
    }
  }
  
  while (startReceived && client.available() >= 3) {
    byte r = client.read();
    byte g = client.read();
    byte b = client.read();

    if (curLed >= LED_COUNT_VIX) {
      startReceived = false;
      if (r == 'd' && g == 'e' && b == 'f') {
        FastLED.show();
      }
      break;
    } else {
      leds[curLed] = CRGB(r, g, b);
    }
    curLed++;
  }
}

/*void loopTest() {
  leds[ir] = CRGB(255, 0, 0);
  leds[ig] = CRGB(0, 255, 0);
  leds[ib] = CRGB(0, 0, 255);
  leds[io] = CRGB(0, 0, 0);
  FastLED.show();

  ir = (ir + 1) % LED_COUNT;
  ig = (ig + 1) % LED_COUNT;
  ib = (ib + 1) % LED_COUNT;
  io = (io + 1) % LED_COUNT;

  delay(500);
}*/

boolean ensureConnection() {
  //Check whether the Ethernet cable is connected; warn user if not.
  if (Ethernet.linkStatus() == LinkOFF) {
    connected = false;
    setAllLeds(0, 0, 0);
    leds[statusLed] = CRGB(255, 0, 0); //Red
    FastLED.show();
    return false;
  }
  
  // if the server's disconnected, stop the client:
  if (connected) {
    if (client.connected()) {
      return true;
    } else {
      client.stop();
      connected = false;
      setAllLeds(0, 0, 0);
      leds[statusLed] = CRGB(255, 127, 0); //Orange
      FastLED.show();
      return false;
    }
  } else {
    //Try to (re-)connect.
    if (client.connect(serverIp, serverPort)) {
      connected = true;
      setAllLeds(0, 0, 0);
      leds[statusLed] = CRGB(0, 255, 0); //Green
      FastLED.show();
      return true;
    } else {
      connected = false;
      setAllLeds(0, 0, 0);
      leds[statusLed] = CRGB(255, 127, 0); //Orange
      FastLED.show();
      return false;
    }
  }
}

void setAllLeds(int r, int g, int b) {
  for (int i = 0; i < LED_COUNT; i++) {
    leds[i] = CRGB(0, 0, 0);
  }
}
