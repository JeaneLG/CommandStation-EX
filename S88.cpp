#include "S88.h"
#include "Sensors.h"

S88Master::S88Master(byte dataPin, byte clkPin, byte psPin, byte resetPin, byte numModules, int firstVpin) : (firstVpin, InputsPerModule*numModules)
{
  s88Module = numModules;
  data = new byte[s88Module];
  s88sendon = '0';
  s88RCount = 0;
  s88RMCount = 0;
  s88DataPin = dataPin;
  s88ClkPin = clkPin;
  s88PsPin = psPin;
  s88ResetPin = resetPin;
  
  // add S88Master to list of devices
  addDevice(this);
}

void S88Master::_begin()
{
  pinMode(s88ResetPin, OUTPUT);    //Reset
  pinMode(s88PsPin, OUTPUT);      //PS/LOAD
  pinMode(s88ClkPin, OUTPUT);      //Clock
  digitalWrite(s88ResetPin, LOW);
  digitalWrite(s88PsPin, LOW);      //init
  digitalWrite(s88ClkPin, LOW);
  
  pinMode(s88DataPin, INPUT);    //Dateneingang

  createSensors();
  _deviceState = DEVSTATE_NORMAL;
}

void S88Master::_loop(unsigned long currentMicros)
{
  if (s88RCount == 3)    //Load/PS Leitung auf 1, darauf folgt ein Schiebetakt nach 10 ticks!
    digitalWrite(s88PsPin, HIGH);
  if (s88RCount == 4)   //Schiebetakt nach 5 ticks und S88Module > 0
    digitalWrite(s88ClkPin, HIGH);       //1. Impuls
  if (s88RCount == 5)   //Read Data IN 1. Bit und S88Module > 0
    s88readData();    //LOW-Flanke während Load/PS Schiebetakt, dann liegen die Daten an
  if (s88RCount == 9)    //Reset-Plus, löscht die den Paralleleingängen vorgeschaltetetn Latches
    digitalWrite(s88ResetPin, HIGH);
  if (s88RCount == 10)    //Ende Resetimpuls
    digitalWrite(s88ResetPin, LOW);
  if (s88RCount == 11)    //Ende PS Phase
    digitalWrite(s88PsPin, LOW);
  if (s88RCount >= 12 && s88RCount < 10 + (s88Module * 8) * 2) {    //Auslesen mit weiteren Schiebetakt der Latches links
    if (s88RCount % 2 == 0)      //wechselnder Taktimpuls/Schiebetakt
      digitalWrite(s88ClkPin, HIGH);  
    else s88readData();    //Read Data IN 2. bis (Module*8) Bit
  }
  s88RCount++;      //Zähler für Durchläufe/Takt
  if (s88RCount >= 10 + (s88Module * 8) * 2) {  //Alle Module ausgelesen?
    s88RCount = 0;                    //setzte Zähler zurück
    s88RMCount = 0;                  //beginne beim ersten Modul von neuem
    //init der Grundpegel
    digitalWrite(s88PsPin, LOW);    
    digitalWrite(s88ClkPin, LOW);
    digitalWrite(s88ResetPin, LOW);
    if (s88sendon == 's')  //Änderung erkannt
      s88sendon = 'i';      //senden
  }
  
  delayUntil(LOOP_STEP_TIME_MICROS + currentMicros); // set time when this method is called next
}

int S88Master::_read(VPIN vpin)
{
  VPIN absPin = vpin - _firstVpin;
  byte Modul = absPin / InputsPerModule;
  byte Port = absPin % InputsPerModule;
  return bitRead(data[Modul],Port);
}

void S88Master::_display() {
  DIAG(F("S88Master Configured on VPins:%d-%d"), (int)_firstVpin, (int)_firstVpin+_nPins-1);
}

void S88Master::createSensors()
{
  int pin = _firstVpin;
  for (; pin<_firstVpin+_nPins; pin++) Sensor::create(pin, pin, 1);
}

//Einlesen des Daten-Bit und Vergleich mit vorherigem Durchlauf
void S88Master::s88readData() {
  digitalWrite(s88ClkPin, LOW);  //LOW-Flanke, dann liegen die Daten an
  byte Modul = s88RMCount / 8;
  byte Port = s88RMCount % 8;
  byte getData = digitalRead(s88DataPin);  //Bit einlesen
  if (bitRead(data[Modul],Port) != getData) {     //Zustandsänderung Prüfen?
    bitWrite(data[Modul],Port,getData);          //Bitzustand Speichern
    s88sendon = 's';  //Änderung vorgenommen. (SET)
  }
  s88RMCount++;
}
