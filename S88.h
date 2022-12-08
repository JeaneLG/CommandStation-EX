/**
 * S88 Master.
 */
#ifndef S88_h
#define S88_h

#include <Arduino.h>

#include "IODevice.h"

// Defines the frequency of the S88 Bus
#define LOOP_STEP_TIME_MICROS 4 // 250kHz

//--------------------------------------------------------------
//Pinbelegungen am Dekoder:
  //Eingänge:
#define S88DataPin 2      //S88 Data IN
  //Ausgänge:
#define S88ClkPin 4    //S88 Clock
#define S88PSPin 5    //S88 PS/LOAD
#define S88ResetPin 6    //S88 Reset

#define FIRST_VPIN_S88 880
#define InputsPerModule 8

class S88Master;
static S88Master *first = NULL;

class S88Master : public IODevice
{
  public:
    static void create(byte dataPin=S88DataPin, byte clkPin=S88ClkPin, byte psPin=S88PSPin, byte resetPin=S88ResetPin, byte numModules=3, int firstVpin=FIRST_VPIN_S88) {
      S88Master *next = first;
      first = new S88Master(dataPin, clkPin, psPin, resetPin, numModules, firstVpin);
      first->next = next;
    }

  protected:
    void _begin() override;
    void _loop(unsigned long currentMicros) override;
    // supports only read operations
    void _write(VPIN vpin, int value) override {};
    int _read(VPIN vpin) override;
    void _display() override;

  private:
    S88Master *next = NULL;
    byte s88Module;    //Anzahl der 8x Module
                       //maximal 62 x 8 Port Module = 31 Module à 16 Ports
    byte *data;        //Zustandsspeicher
    /*
    '0' = keine
    's' = Änderungen vorhanden, noch nicht fertig mit Auslesen
    'i' = Daten vollständig, senden an PC
    */
    char s88sendon;       //Bit Änderung
    
    int s88RCount = 0;    //Lesezähler 0-39 Zyklen
    int s88RMCount = 0;   //Lesezähler Modul-Pin

    byte s88DataPin;
    byte s88ClkPin;
    byte s88PsPin;
    byte s88ResetPin;
    
    S88Master(byte dataPin, byte clkPin, byte psPin, byte resetPin, byte numModules, int firstVpin);
    /**
     * Creates a sensor for each S88 module input.
     */
    void createSensors();
    void s88readData();
};

#endif
