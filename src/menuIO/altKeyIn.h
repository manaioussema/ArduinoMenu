/* -*- C++ -*- */
/**************
Sept. 2014 Rui Azevedo - ruihfazevedo(@rrob@)gmail.com

quick and dirty keyboard driver
metaprog keyboard driver where N is the number of keys
all keys are expected to be a pin (buttons)
we can have reverse logic (pull-ups) by entering negative pin numbers
ex: -A0 means: pin A0 normally high, low when button pushed (reverse logic)

***/

#ifndef RSITE_KEYIN_DEF_H
  #define RSITE_KEYIN_DEF_H
  #include "../menuDefs.h"

  namespace Menu {

    #include "altKeyMapDef.h"

    //if you hold/repeat a key for this ammount of time we will consider it an escape
    #ifndef ESCAPE_TIME
    #define ESCAPE_TIME 1500
    #endif
    //emulate a stream keyboard, this is not using interrupts as a good driver should do
    // AND is not using a buffer either!
    template <int N>
    class keyIn:public menuIn {
    public:
      keyMap* keys;
      int lastkey, buttonState[N], lastButtonState[N];
      unsigned long pressMills=0, lastDebounceTime[N], debounceDelay=70; //debounceDelay in ms
      keyIn<N>(keyMap k[]):keys(k),lastkey(-1) {}
      void begin() {
        for(int n=0;n<N;n++)
          pinMode(keys[n].pin,keys[n].mode);

      }
      int available(void) {
        //MENU_DEBUG_OUT<<"available"<<endl;
        int ch=peek();
        if (lastkey==-1) {
          lastkey=ch;
          pressMills=millis();
        } else if (ESCAPE_TIME&&millis()-pressMills>ESCAPE_TIME) return 1;
        if (ch==lastkey) return 0;
        return 1;
        /*int cnt=0;
        for(int n=0;n<N;n++) {
          int8_t pin=keys[n].pin;
          if (digitalRead(pin<0?-pin:pin)!=(pin<0) ) cnt++;
        }
        return cnt;*/
      }
      int peek(void) {
        //MENU_DEBUG_OUT<<"peek"<<endl;
	int reading,retour;
        for(int n=0;n<N;n++) {
          int8_t pin=keys[n].pin;
	  uint8_t mode = keys[n].mode == INPUT_PULLUP ? LOW : HIGH;
          reading = digitalRead(pin);
          if (reading != lastButtonState[n])
             lastDebounceTime[n] = millis();
          if ((millis() - lastDebounceTime[n]) > debounceDelay) {
             if (reading != buttonState[n]) {
               buttonState[n] = reading;
                if (buttonState[n] == mode)
                   retour = keys[n].code;break;
                else retour = -1;
             }
          }
          lastButtonState[n] = reading;
         }
         return retour;
      }
      int read() {
        //MENU_DEBUG_OUT<<"read"<<endl;
        int ch=peek();
        if (ch==lastkey) return -1;
        int tmp=lastkey;
        bool longPress=ESCAPE_TIME&&millis()-pressMills>ESCAPE_TIME;
        //MENU_DEBUG_OUT<<"read lastkey="<<lastkey<<" ch="<<ch<<endl;
        //MENU_DEBUG_OUT<<"down time:"<<millis()-pressMills<<endl;
        pressMills=millis();
        lastkey=ch;
        return longPress?options->getCmdChar(escCmd):tmp;//long press will result in escape
      }
      void flush() {}
      size_t write(uint8_t v) {return 0;}
    };

  }//namespace Menu

#endif
