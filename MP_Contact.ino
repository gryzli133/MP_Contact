/*
- v0.3 - moved classes to library folder
- v0.4 - add wait(100) to presentation and setup
- v0.5 - add MP_NONE_PIN and setValue(bool newValue) - use as virtual sensor is now possible
*/

#define IS_ACK false //is to acknowlage

// Enable MySensors debug prints to serial monitor
#define MY_DEBUG

// Enable MP_DEBUG_Contact debug prints to serial monitor
//#define MP_DEBUG_Contact

// Enable prints with loop cycle time
//#define MP_DEBUG_CYCLE_TIME

// Use a bit lower baudrate for serial prints on ESP8266 than default in MyConfig.h
//#define MY_BAUD_RATE 9600
//#define MY_BAUD_RATE 115200

#define MY_TRANSPORT_WAIT_READY_MS 10000


// !!!! CHOOSE ONLY ONE !!!!

//#include "ESPGateway.h" // for ESP8266 WiFi gateway -> set your WiFi in ESPGateway.h tab!
#include "SerialGateway.h" // for Serial gateway
//#include "RF24Gateway.h" // for RF24 radio gateway
//#include "MQTTGateway.h" // for MQTT Ethernet gateway
//#include "PJON.h" // for PJON wired gateway

#include <Bounce2.h>
#include <MySensors.h>
#include <SPI.h>

#define MY_NODE_ID 25
//#define WITH_ELEKTROZACZEP

#define WAIT_PRESENTATION_TIME 100

#include <MP_Contact.h>

/* define your Contractor objects here
Contact( int childId,                   // unique ID number
            int Contact,                // used PIN number
            int debaunce,               // default 50 ms
            bool invertedContact,       // 0: Tripped = VCC; 1: Tripped = GND;
            unsigned long refreshTime,  // default 60s
            const char *descr)          // 
*/


uint32_t lastCycle;
uint32_t minCycle = 0;
uint32_t maxCycle = 60000;
int cycleCount=0;

int ledPin = 13;

MP_Contact contacts[] =
{
  {1, 22, 50, 0, 300, "Sabotaz Garderoba"},
  {2, 23, 50, 0, 300, "Kontaktron Garderoba"},
  {3, 24, 50, 0, 300, "Sabotaz Sypialnia - L"},
  {4, 25, 50, 0, 300, "Kontaktron Sypialnia - L"},
  {5, 26, 50, 0, 300, "Sabotaz Sypialnia - R"},
  {6, 27, 50, 0, 300, "Kontaktron Sypialnia - R"},
  {7, 28, 50, 0, 300, "Sabotaz Pokoj 1 - L"},
  {8, 29, 50, 0, 300, "Kontaktron Pokoj 1 - L"},
  {9, 30, 50, 0, 300, "Sabotaz Pokoj 1 - R"},
  {10, 31, 50, 0, 300, "Kontaktron Pokoj 1 - R"},
  {11, 32, 50, 0, 300, "Sabotaz Pokoj 1 Balkonowe"},
  {12, 33, 50, 0, 300, "Kontaktron Pokoj 1 Balkonowe"},
  {13, 34, 50, 0, 300, "Sabotaz Pokoj 2 Balkonowe"},
  {14, 35, 50, 0, 300, "Kontaktron Pokoj 2 Balkonowe"},
  {15, 36, 50, 0, 300, "Sabotaz Pokoj 3 Balkonowe"},
  {16, 37, 50, 0, 300, "Kontaktron Pokoj 3 Balkonowe"},
  {17, 38, 50, 0, 300, "Sabotaz lazienka gora"},
  {18, 39, 50, 0, 300, "Kontaktron lazienka gora"},
  {19, 40, 50, 0, 300, "Sabotaz Strych"},
  {20, 41, 50, 0, 300, "Kontaktron Strych"}
  
};

const int contactCount = sizeof(contacts) / sizeof(MP_Contact);

#include <MP_SwitchRelay.h>

/* define your SwitchRelay objects here
 SwitchRelay(int childId      // Cover/Roller Shutter device ID for MySensors and Controller (Domoticz, HA etc)
              , int setIdUp     // Roll time UP setpoint device ID for MySensors and Controller (Domoticz, HA etc)
              , int setIdDown   // Roll time DOWN setpointdevice ID for MySensors and Controller (Domoticz, HA etc)
              , int initId      // Initialization device ID for MySensors and Controller (Domoticz, HA etc)
              , int buttonUp    // Button Pin for UP
              , int buttonDown  // Button Pin for DOWN
              , int relayUp     // Relay Pin for UP
              , int relayDown   // Relay Pin for DOWN
              , uint8_t initTimeUp          // Initial Roll time UP
              , uint8_t initTimeDown        // Initial Roll time DOWN
              , uint8_t initCalibrationTime // Initial calibration time (time that relay stay ON after reach 0 or 100%)
              , int debaunceTime            // Time to debounce button -> standard = 50
              , bool invertedRelay          // for High level trigger = 0; for Low level trigger = 1
              )
*/

#ifdef WITH_ELEKTROZACZEP

MP_SwitchRelay lights[] =
{
  //A15
  {61, 4, 14, 50, 0, "Elektrozaczep furtka glowna"},
  {62, 5, 15, 50, 0, "Elektrozaczep brama wjazdowa"},
  {63, 6, 16, 50, 0, "Elektrozaczep furtka pod wiata"},
  {64, 7, 17, 50, 0, "Elektrozaczep furtka za wiata"},
  {65, 8, 18, 50, 0, "Elektrozaczep drzwi glowne"},
  {66, 9, 19, 50, 0, "Elektrozaczep drzwi biuro"},
  {67, 10, 20, 50, 0, "Elektrozaczep drzwi garaz"},
  {68, 11, 21, 50, 0, "Rezerwa 32"}
};

const int lightsCount = sizeof(lights) / sizeof(MP_SwitchRelay);

uint32_t millisFurtka = 0;
uint32_t minFurtka = 1000;
uint32_t maxFurtka = 60000;
uint32_t emergencyFurtka = 5000;
bool lastValueFurtka = 0;
bool emergencyModeFurtka = 0;

#endif

void setup() 
{ 

  // Setup locally attached sensors
  wait(MY_NODE_ID * 100);
  
  for(int i = 0; i < contactCount; i++)
  {
    contacts[i].Init();
    contacts[i].SyncController(); 
    contacts[i].Update();
    wait(WAIT_PRESENTATION_TIME);
  }
  
  
  #ifdef WITH_ELEKTROZACZEP
  for(int i = 0; i < lightsCount; i++)
  {
    lights[i].Update();
    lights[i].SyncController(); 
    wait(WAIT_PRESENTATION_TIME);
  }
  #endif
  pinMode(ledPin, OUTPUT);
}

void presentation()  
{   
  // Send the sketch version information to the gateway and Controller
  sendSketchInfo("MP_Contact", "0.5");
  for(int i = 0; i < contactCount; i++)
  {
    contacts[i].Present(); 
    wait(WAIT_PRESENTATION_TIME);
  }
  #ifdef WITH_ELEKTROZACZEP
  for(int i = 0; i < lightsCount; i++)
  {
    lights[i].Present(); 
    wait(WAIT_PRESENTATION_TIME);
  }
  #endif
}

void loop() 
{ 
  uint8_t openCount = 0;
  for(int i = 0; i < contactCount; i++)
  {
    contacts[i].Update(); 
    openCount += !contacts[i].isTripped();
  }

  if (openCount > 0)
  {
    digitalWrite(ledPin, (millis()/500)%2);
  }
  else
  {
    digitalWrite(ledPin, true);
  }
  
  #ifdef WITH_ELEKTROZACZEP
  for(int i = 0; i < lightsCount; i++)
  {
    lights[i].Update(); 
  }

  if(lights[0].GetValue() == 1 && lastValueFurtka == 0)
  {
    millisFurtka = millis();
    emergencyModeFurtka = contacts[41].isTripped();
  }
  
  lastValueFurtka = lights[0].GetValue();

  if(lights[0].GetValue() == 1 && ((emergencyModeFurtka == 0 && contacts[41].isTripped() && millis() - millisFurtka > minFurtka) || (emergencyModeFurtka && millis() - millisFurtka > emergencyFurtka) || millis() - millisFurtka > maxFurtka))
  {
    lights[0].SwitchOff();
    emergencyModeFurtka = 0;
  }
  #endif
  

  

  #ifdef MP_DEBUG_CYCLE_TIME
  if(cycleCount>1000)
  {
    Serial.println(String(minCycle));
    Serial.println(String(maxCycle));
    cycleCount = 0;
    minCycle = 60000;
    maxCycle = 0;
  }
  minCycle = min(millis() - lastCycle , minCycle);
  maxCycle = max(millis() - lastCycle , maxCycle);
  cycleCount++;
  lastCycle = millis();
  #endif
  
}
#ifdef WITH_ELEKTROZACZEP
void receive(const MyMessage &message) 
{
  for(int i = 0; i < lightsCount; i++)
  {
    lights[i].Receive(message); 
  }
}
#endif
