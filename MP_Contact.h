#ifndef MP_Contact_h
#define MP_Contact_h

#ifndef IS_ACK
#define IS_ACK false
#endif

#define MP_PIN_NONE 255         // if there is no need for PIN, then it will be used

//#include <MP_Button.h>
#include <Bounce2.h>

class MP_Contact
{ 
  uint8_t child_id;
  uint8_t ContactPin;
  bool Tripped;
  uint8_t value;
  uint8_t oldValue = 255;
  uint32_t last_update;
  uint32_t refresh_time;
  const char * description;
  Bounce debouncer = Bounce();
  
  public:
  MP_Contact(int childId, int Contact, int debaunce, bool invertedContact, unsigned long refreshTime, const char *descr) : msg(childId, V_TRIPPED)
  {
    // constructor - like "setup" part of standard program
    child_id = childId;
    ContactPin = Contact;
    Tripped = !invertedContact;
    refresh_time = refreshTime * 1000;
    description = descr;
	if(ContactPin != MP_PIN_NONE)
	{
		digitalWrite(ContactPin, !Tripped);
		pinMode(ContactPin, INPUT_PULLUP);
		debouncer.attach(ContactPin);
		debouncer.interval(debaunce);
		debouncer.update();
	}  
	value = !Tripped;
  }  

  MyMessage msg;
  
  void Init()
  {
	if(ContactPin != MP_PIN_NONE)
	{
		digitalWrite(ContactPin, HIGH);
		pinMode(ContactPin, INPUT_PULLUP);
	}
  }
  
  void Update()
  {
	if(ContactPin != MP_PIN_NONE)
	{  
		debouncer.update();
		value = debouncer.read();
	}
	// Send in the new value.
	if(value != oldValue)
	{
		SyncController();
	}
	else
	{
		if(millis() - last_update >= refresh_time)
		{
			SyncController();
		}
	}   
	oldValue = value; 
  }

  uint8_t isTripped()
  {
    return value==Tripped ? 1 : 0;
  }
  
  uint8_t setValue(bool newValue)
  {
	  value = newValue ? Tripped : !Tripped;
  }

  void SyncController()
  {
    send(msg.set(value==Tripped ? 1 : 0));
    last_update = millis();
  }
  
  void Present()
  {
    present(child_id, S_DOOR, description, IS_ACK);
  }
};
#endif