/* The Irwin 2G/3G v0.1 - StingRay/IMSI Catcher detector */
/* Licensed under GNU General Public License v3          */

/*  
 *** IMPORTANT WARNING: DO NOT USE TOO MUCH RAM ( Adafruit Feather FONA has 1024 bytes)!!!
 *** CTRL+r to rebuild in Arduino and make sure the "bytes of dynamic memory" < 1024 bytes or possible crash!!!
 *** If you use different arduino's, make sure you have at least 1KB of RAM/EEPROM.
 *** DOUBLE CHECK YOUR BUILD SIZE BEFORE UPLOADING!!
*/

/* - PARTS LIST -
 * Adafruit Feather 32u4 FONA - https://www.adafruit.com/product/3027 - $44.95
 * Lithium Ion Polymer Battery (3.7v 1200mAh) - https://www.adafruit.com/product/258 - $9.95
 * SMA to uFL/u.FL/IPX/IPEX RF Adapter Cable - https://www.adafruit.com/product/851 - $3.95
 * Mini GSM/Cellular Quad-Band Antenna (2dBi SMA Plug) - https://www.adafruit.com/product/1859 - $4.95
 * Total USD: $63.80
 */

/* - LED output indicators - 
 * Green LED: Power is ON.
 * Blue LED: Network is trying to establish a connection when blinking. LED disabled by default to save power.
 * Orange LED: USB port is connected and/or battery is charging. LED turns off when battery is full.
 * Red LED: Blinks 3 times when a rogue tower is detected. Blinks 1 time when a rogue tower was seen, but is now gone.
 * USB Serial: Will give you output to the serial port if ShowList, ShowRaw, or ShowCount is true. 
 */

/* - Arduino Setup -
 * Library: Adafruit FONA Library by Adafruit version 1.3.3
 * Library: SoftwareSerial Library version 1.0
 * Board: "Adafruit Feather 32u4"
 * Programmer: "AVR ISP"
 * Port: Depends on the OS you are using.
 */

/* - Results Verification -
 * You can lookup scan results with https://cellidfinder.com/
 */

/* - Acronyms  - 
 * MCC - Mobile Country Code
 * MNC - Mobile Network Code
 * LAC - Location Area Code
 * CID - Cell IDentification
 */

#include "Adafruit_FONA.h"
#include <SoftwareSerial.h>

/* DON'T FUCK WITH THIS UNLESS YOU KNOW WHAT YOU ARE DOING */
#define MaxMemory 1024      // The total amount of SRAM on the board. Adafruit Feather 32u4 FONA is 1024 bytes.
#define MBS 360             // Modem Buffer Size. Don't fuck with this. JUST DON'T.
#define FONA_RX 9
#define FONA_TX 8
#define FONA_RI 7
#define FONA_RST 4
SoftwareSerial fonaSS = SoftwareSerial(FONA_TX, FONA_RX);
SoftwareSerial *fonaSerial = &fonaSS;
Adafruit_FONA fona = Adafruit_FONA(FONA_RST);

bool rogueSeen   = false;
bool towerSeen   = false;
bool baselineSet = false;
uint8_t  CBAND = 0;                   // Place holder for which cellular band to scan.
uint16_t totalIter = 0;               // Total iterations is used with AdapCount to establish baseline.
uint16_t CT[6];                       // Only 6 digits are read from the results.

// **** User configurable options ****
#define AdapCount 43200     // The number of ScanSleep times the loop has ran before a "baseline" is established. (Default 12 hours)
#define ScanSleep 1000      // The number of milliseconds between rescan. (Default 1 second.)
#define ScanResetCount 30   // The number of ScanSleep times without seeing a tower, then reboot. Less than 10 is not good.

// **** The mode is either "Scan", "Adaptive", "Blacklist" or "Whitelist".
// **** Must choose only one OpMode! Uncomment LMode line when using "Whitelist" or "Blacklist".
#define OpMode "Scan"       // "Scan" should be used for site surveys and to establish a baseline.
//#define OpMode "Adaptive"   // "Adaptive" mode will get a "baseline" detection after AdapCount is hit. Takes a LOOONG time. (hours/days)
//#define OpMode "Blacklist"  // "Blacklist" has a defined list of towers that should not be near you. Home baseline needed.
//#define OpMode "Whitelist"    // "Whitelist" has a defined list of approved towers that are not rogue. WarDriving helps figure this out.
//#define LMode true            // If OpMode == "Whitelist" or "Blacklist", uncomment this line. If OpMode == "Scan" or "Adaptive" then comment this line.

// **** User configurable options ****
#define ShowList  true     // ShowList prints the list of the tower heard. Whitelist or Blacklist make this list the rogue towers.
#define CodeOut   false    // IF ShowList == true and CodeOut == true, then output the tower list in a copy/paste format for code.
#define ShowRaw   false    // ShowRaw will show the raw output from the SIM800 modem.
#define ShowCount true     // ShowCount will show the number of iterations the program has done and the OpMode.
#define ShowBatt  true     // ShowBatt will show the battery voltage and percentage left.
#define ShowBand  true     // ShowBand will show the Cellular Network Band that the modem is set to use.
//#define MaxBuffer          // This will not log or use the MCC and MNC in detection. Comment this line to disable it.

//=======================================================================
// Whitelist is an nested array in the format of {MCC,MNC,LAC,CID}, or the first 4 numbers from the raw output.
// WhitelistLite is a nested array in the format of {LAC,CID}. Used when MaxBuffer is true.
//=======================================================================
// Buenos Aires, Argentina
uint16_t WhitelistLite[][2] = {
//  {452,20206},
//  {452,36480},
//  {452,16480},
//  {452,26480},
//  {4428,1059},
//  {4428,1060},
//  {4428,1061},
//  {4428,8698},
//  {4428,8699},
//  {5024,55},
//  {5024,56},
//  {5024,57},
//  {5024,59},
  {5024,5055},
  {5024,5056},
  {5024,5057},
  {5024,5061},
  {5024,5066},
  {5024,5298},
  {5024,5601},
  {5024,5602},
  {7153,10792},
  {7172,16480},
  {7153,10205},
  {7153,16478},
  {7172,10206},
  {7172,20206},
  {7172,36480},
  {7153,30017}, 
  {7167,10199}, 
  {7172,20218},
  {7172,26480},
  {7172,30206},
};

uint16_t Whitelist[][4] = {
//  {722,31,452,20206},
//  {722,31,452,16480},
//  {722,31,452,26480},
//  {722,31,452,36480},
//  {722,7,4428,1059},
//  {722,7,4428,1060},
//  {722,7,4428,1061},
//  {722,7,4428,8698},
//  {722,7,4428,8699},
//  {722,34,5024,55},
//  {722,34,5024,56},
//  {722,34,5024,57},
//  {722,34,5024,59},
  {722,34,5024,5055},
  {722,34,5024,5056},
  {722,34,5024,5057},
  {722,34,5024,5061},
  {722,34,5024,5066},
  {722,34,5024,5298},
  {722,34,5024,5601},
  {722,34,5024,5602},
  {722,31,7153,10792},
  {722,310,7172,16480},
  {722,310,7153,10205},
  {722,310,7153,16478},
  {722,310,7172,10206},
  {722,310,7172,20206},
  {722,310,7172,36480},
  {722,310,7153,30017}, 
  {722,310,7167,10199}, 
  {722,310,7172,20218},
  {722,310,7172,26480},
  {722,310,7172,30206},
};
//=======================================================================

// THIS IS FOR AUTOMATIC MEMORY MANAGEMENT. MUST BE AFTER THE WHITELIST ARRAYS.
#ifdef MaxBuffer
  uint8_t WLNumTowers = (sizeof(WhitelistLite) / (sizeof(uint16_t) * 2));
  #ifdef LMode
    uint8_t MaxTowers = (uint8_t)(((MaxMemory - 835) - sizeof(WhitelistLite) ) / 8);
  #endif
  #ifndef LMode
    uint8_t MaxTowers = (uint8_t)((MaxMemory - 628) / 8);
  #endif
#endif
#ifndef MaxBuffer
  uint8_t WLNumTowers = (sizeof(Whitelist) / (sizeof(uint16_t) * 4));
  #ifdef LMode
    uint8_t MaxTowers = (uint8_t)((MaxMemory - 835 - sizeof(Whitelist)) / 12);
  #endif
  #ifndef LMode
    uint8_t MaxTowers = (uint8_t)((MaxMemory - 628) / 12);
  #endif
#endif
#ifdef LMode
  uint16_t LTTowersLite[(int)((MaxMemory - 835 ) / 8)][4];   // Remember up to <MaxTowers> towers TOTAL. Lite info.
  uint16_t LTTowers[(int)((MaxMemory - 835 ) / 12)][6];      // Remember up to <MaxTowers> towers TOTAL. Full info.  
#else
  uint16_t LTTowersLite[(int)((MaxMemory - 628 ) / 8)][4];   // Remember up to <MaxTowers> towers TOTAL. Lite info.
  uint16_t LTTowers[(int)((MaxMemory - 628 ) / 12)][6];       // Remember up to <MaxTowers> towers TOTAL. Full info.
#endif

void printLTBuffer() {
  #ifndef MaxBuffer
    if ((LTTowers[0][0] != 0) && (LTTowers[0][1] != 0) && (LTTowers[0][2] != 0) && (LTTowers[0][3] != 0)) {
      if ( CodeOut==true && (strcmp(OpMode,"Scan")==0) )  {
        Serial.println(F("{MCC,MNC,LAC,CID}")); // When MaxBuffer == false
      } else {
        Serial.println(F("MCC,MNC,LAC,CID,RX,COUNT")); // When MaxBuffer == false
      }
    }
  #endif
  #ifdef MaxBuffer
    if ((LTTowersLite[0][0] != 0) && (LTTowersLite[0][1] != 0) && (LTTowersLite[0][2] != 0) && (LTTowersLite[0][3] != 0)) {
      if ( (CodeOut == true) && (strcmp(OpMode,"Scan")==0) )  {
        Serial.println(F("{LAC,CID}"));  // When MaxBuffer == true
      } else {
        Serial.println(F("LAC,CID,RX,COUNT"));  // When MaxBuffer == true
      }
    }
  #endif
  for (int c=0;c<MaxTowers;c++) {
    #ifdef MaxBuffer
      if ((LTTowersLite[c][0] != 0) && (LTTowersLite[c][1] != 0) && (LTTowersLite[c][2] != 0) && (LTTowersLite[c][3] != 0)) {
        if ( CodeOut==true && (strcmp(OpMode,"Scan")==0) ) Serial.print(F("{"));
        for (int d=0;d<4;d++) {
          if ( CodeOut==false ) {
            if (d<=1) {
              Serial.print(LTTowersLite[c][d]);
              if (d==0) Serial.print(F(","));
            }
          } else {
            Serial.print(LTTowersLite[c][d]);
            if (d<=2) Serial.print(F(","));
          }
        }
        if ( CodeOut==true && (strcmp(OpMode,"Scan")==0) ) Serial.print(F("},"));
        Serial.println(F(""));
      }
    #endif
    #ifndef MaxBuffer
      if ((LTTowers[c][0] != 0) && (LTTowers[c][1] != 0) && (LTTowers[c][2] != 0) && (LTTowers[c][3] != 0)) {
        if ( CodeOut==true && (strcmp(OpMode,"Scan")==0) ) {
          Serial.print(F("{"));
        }
        for (int d=0;d<6;d++) {
          if ( CodeOut==false ) {
            Serial.print(LTTowers[c][d]);
            if (d<=4) Serial.print(F(","));
          } else {
            if (d<=3) {
              Serial.print(LTTowers[c][d]);
              if (d<=2) Serial.print(F(","));
            }
          }
        }
        if ( CodeOut==true && (strcmp(OpMode,"Scan")==0) ) Serial.print(F("},"));
        Serial.println(F(" "));
      }
    #endif
  }
}

void addToLTB() {
  bool found = false;
  int lastLine = -1;
  for (int i=0;i<MaxTowers;i++) {
    #ifdef MaxBuffer
      if ( LTTowersLite[i][0] != 0 ) { lastLine = i; } // UPDATE WHICH WAS THE LAST LINE THAT HAD DATA
      if ( (CT[2]==LTTowersLite[i][0]) && (CT[3]==LTTowersLite[i][1]) ) {
        found = true;
        if (CT[0] != 0) { 
          // Update seen counter.
          if (LTTowersLite[i][3] == 65535) { LTTowersLite[i][3] = 1; } else { LTTowersLite[i][3]++; }
          LTTowersLite[i][2]=CT[5];   // Update the RX level for this tower
        }
      }
    #else
      if ( LTTowers[i][0] != 0 ) { lastLine = i; } // UPDATE WHICH WAS THE LAST LINE THAT HAD DATA
      if ( (CT[0]==LTTowers[i][0]) && (CT[1]==LTTowers[i][1]) && (CT[2]==LTTowers[i][2]) && (CT[3]==LTTowers[i][3]) ) {
        found = true;
        if (CT[0] != 0) { 
          // Update seen counter.
          if (LTTowers[i][5] == 65535) { LTTowers[i][5] = 1; } else { LTTowers[i][5]++; }
          LTTowers[i][4]=CT[4];   // Update the RX level for this tower
        }
      }      
    #endif
  }
  if ( (found==false) && (lastLine < MaxTowers) && (CT[0] != 0) && (baselineSet==false) ) {
    #ifdef MaxBuffer
      LTTowersLite[lastLine+1][0] = CT[2];
      LTTowersLite[lastLine+1][1] = CT[3];
      LTTowersLite[lastLine+1][2] = CT[5];
      LTTowersLite[lastLine+1][3] = 1;      
    #else
      LTTowers[lastLine+1][0] = CT[0];
      LTTowers[lastLine+1][1] = CT[1];
      LTTowers[lastLine+1][2] = CT[2];
      LTTowers[lastLine+1][3] = CT[3];
      LTTowers[lastLine+1][4] = CT[5];
      LTTowers[lastLine+1][5] = 1;
    #endif
  }
  if (totalIter > AdapCount) {
    if ((strcmp(OpMode,"Adaptive")==0) && (baselineSet==false)) {
      baselineSet = true;
    }
  }
}

void checkLT() {
  bool goodTower = false;
  for (int i=0;i<MaxTowers;i++) {
    if ( (LTTowers[i][0]==CT[0]) && (LTTowers[i][1]==CT[1]) && (LTTowers[i][2]==CT[2]) && (LTTowers[i][3]==CT[3]) ) {
      goodTower = true;
      break;
    }
  }
  // WARNING: EVIL STING RAY NEAR BY! GET OUT OF THE WATER!!
  if ((goodTower==false) && (CT[0]!=0)) {
    stingAlert();
  }
}

void checkWL() {
  bool goodTower = false;
  for (int i=0;i<WLNumTowers;i++) {
    #ifdef MaxBuffer
      if ( (WhitelistLite[i][0]==CT[2]) && (WhitelistLite[i][1]==CT[3]) ) {
        goodTower = true;
        break;
      }
    #else
      if ( (Whitelist[i][0]==CT[0]) && (Whitelist[i][1]==CT[1]) && (Whitelist[i][2]==CT[2]) && (Whitelist[i][3]==CT[3]) ) {
        goodTower = true;
        break;
      }
    #endif
  }
  // WARNING: EVIL STING RAY NEAR BY! GET OUT OF THE WATER!!
  if ( (goodTower==false) && (CT[0]!=0) ){
    stingAlert();
    addToLTB();     // Don't enable without changing the MaxTower lower, like 1.
  }
}

void checkBL() {
  bool goodTower = true;
  for (int i=0;i<WLNumTowers;i++) {
    #ifdef MaxBuffer
//    if (MaxBuffer==true) {
      if ( (WhitelistLite[i][0]==CT[2]) && (WhitelistLite[i][1]==CT[3]) ) {
        goodTower = false;
        break;
      }      
//    } else {
    #else
      if ( (Whitelist[i][0]==CT[0]) && (Whitelist[i][1]==CT[1]) && (Whitelist[i][2]==CT[2]) && (Whitelist[i][3]==CT[3]) ) {
        goodTower = false;
        break;
      }
//    }
    #endif
  }
  // WARNING: EVIL STING RAY IS FOLLOWING YOU!! GET OUT OF THE WATER!!
  if ( (goodTower==false) && (CT[0]!=0) ){
    stingAlert();
    addToLTB();     // Don't enable without changing the MaxTower lower, like 1.
  }
}

void stingAlert() {
  rogueSeen = true;
  if (CodeOut == false) {
    Serial.print(F("[!] "));
    Serial.print(CT[0]);
    Serial.print(F(","));
    Serial.print(CT[1]);
    Serial.print(F(","));
    Serial.print(CT[2]);
    Serial.print(F(","));
    Serial.print(CT[3]);
    Serial.print(F(","));
    Serial.print(CT[4]);
    Serial.print(F(","));
    Serial.println(CT[5]);
  } else {
    #ifdef MaxBuffer
      Serial.print(F("{"));
      Serial.print(CT[2]);
      Serial.print(F(","));
      Serial.print(CT[3]);
      Serial.println(F("},"));
    #else
      Serial.print(F("{"));
      Serial.print(CT[0]);
      Serial.print(F(","));
      Serial.print(CT[1]);
      Serial.print(F(","));
      Serial.print(CT[2]);
      Serial.print(F(","));
      Serial.print(CT[3]);
      Serial.println(F("},"));
    #endif
  }
  fastBlink(3);
}

int HexToInt(char str[]) {
  return strtol(str, 0, 16);
}

int IntToInt(char str[]) {
  return strtol(str, 0, 10);
}

void fastBlink(uint8_t b) {
  if (b<=0) { b=1; }
  for (uint8_t i=0;i<b;i++) {
    digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
    delay(200);
    digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW
    delay(100);
  }
}

void fonaReset() {
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  digitalWrite(FONA_RST, LOW);   // turn the LED on (HIGH is the voltage level)
  delay(2000);                   // wait for a second
  digitalWrite(FONA_RST, HIGH);  // turn the LED off by making the voltage LOW
  delay(2000);
  fona.println(F("ATZ"));  delay(250);
  fona.println(F("ATE0")); delay(250);  
  fona.println(F("AT+CNETLIGHT=0")); delay(250);  // DISABLE BLUE (NETWORK) LED. Saves power.
  changeBand(0); delay(250);
  fona.println(F("AT+CBAND?")); delay(250);  
  fona.println(F("AT+CENG=3,1")); delay(250);
  delay(1000);
  while (fona.available()) {
    fona.read();
    delay(2);
  }
  delay(100);
  digitalWrite(LED_BUILTIN, LOW);
}

void changeBand(uint8_t band){
  CBAND = band;
  if (band == 0) { fona.println(F("AT+CBAND=\"ALL_BAND\"")); }
  if (band == 1) { fona.println(F("AT+CBAND=\"PGSM_MODE\"")); } // DOCS SAY THIS MODE IS SUPPORTED, BUT IT IS NOT WORKING. DO NOT USE.
  if (band == 2) { fona.println(F("AT+CBAND=\"DCS_MODE\"")); }
  if (band == 3) { fona.println(F("AT+CBAND=\"PCS_MODE\"")); }
  if (band == 4) { fona.println(F("AT+CBAND=\"EGSM_MODE\"")); }
  if (band == 5) { fona.println(F("AT+CBAND=\"EGSM_DCS_MODE\"")); }
  if (band == 6) { fona.println(F("AT+CBAND=\"EGSM_PCS_MODE\"")); }
  if (band == 7) { fona.println(F("AT+CBAND=\"GSM850_MODE\"")); }
  if (band == 8) { fona.println(F("AT+CBAND=\"GSM850_PCS_MODE\"")); }
  if (band >= 9) { fona.println(F("AT+CBAND=\"ALL_BAND\"")); } // DEFAULT IF THE INPUT IS TOO HIGH.
  // Clears the buffer output
  while (fona.available()) {
    // EXTRA DEBUG INFO ON BOOT
    // char c = fona.read();
    // Serial.print(c);
    fona.read();
    delay(1);
  }
}

void setup() {  
  pinMode(FONA_RST, OUTPUT);
  Serial.begin(9600);
  fonaSerial->begin(4800);
  if (!fona.begin(*fonaSerial)) {
    while (1);
  }
  if ( (strcmp(OpMode,"Whitelist")==0) or (strcmp(OpMode,"Blacklist")==0) ) {
    #ifdef MaxBuffer
      Serial.print(F("Bytes in WhitelistLite: "));
      Serial.println(sizeof(WhitelistLite));
      Serial.print(F("Number of towers in WhitelistLite: "));
      Serial.println((sizeof(WhitelistLite) / (sizeof(uint16_t) * 2)));
    #else
      Serial.print(F("Bytes in Whitelist: "));
      Serial.println(sizeof(Whitelist));
      Serial.print(F("Number of towers in Whitelist: "));
      Serial.println((sizeof(Whitelist) / (sizeof(uint16_t) * 4)));
    #endif
  }
  Serial.print(F("MaxMemory: "));
  Serial.println(MaxMemory);
  #ifdef MaxBuffer
    uint16_t checkMemory = sizeof(WhitelistLite) + 835;
  #else
    uint16_t checkMemory = sizeof(Whitelist) + 835;
  #endif
  Serial.print(F("Memory Usage: "));
  Serial.println(checkMemory);
  if ( (strcmp(OpMode,"Whitelist")==0) or (strcmp(OpMode,"Blacklist")==0) ) {
    if (checkMemory >= MaxMemory) {
      Serial.println(F("WARNING: YOUR COMPILED CODE COULD USE TOO MUCH RAM! Try enabling MaxBuffer!"));
      Serial.println(F("WARNING: This happens when you have too many entries in the Whitelist."));
      Serial.println(F("WARNING: Strange effects could happen because of this."));
      Serial.println(F("WARNING: Setting MaxTower to 1 just to be safe. Change this at your own risk!"));
      MaxTowers = 1;
    }
  }
  Serial.print(F("MaxTowers: "));
  Serial.println(MaxTowers);

  fona.println(F("ATZ"));  delay(250);
  fona.println(F("ATE0")); delay(250);  
  fona.println(F("AT+CNETLIGHT=0")); delay(250);  // DISABLE BLUE (NETWORK) LED. Saves power.
  changeBand(0); delay(250);
  fona.println(F("AT+CBAND?")); delay(250);  
  fona.println(F("AT+CENG=3,1")); delay(250);
  // Cleans the buffer output before running.
  while (fona.available()) {
    // EXTRA DEBUG INFO ON BOOT
    // char c = fona.read();
    // Serial.print(c);  // Extra debug on setup
    fona.read();
    delay(1);
  }
}

void loop() {
    int rCount;
    while (true) {
      if (totalIter == 65535) {
        totalIter=1;
      } else {
        totalIter++;
      }
      
      char rb[MBS] = " ";
      String temp;
      rCount = 0;
      fona.println(F("AT+CENG?"));
      delay(10);

      // Read from the FONA up to 3 times. This is a fix for when the CPU
      // doesn't get all the data from the FONA because of lag on the FONA.
      // This ensures that you get all the data back from the FONA.
      for (int i=0; i<3; i++) {
        while (fona.available()) {
          char c = fona.read();  // Read from FONA for 1 byte
          rb[rCount] = c;        // Add the byte to the char array
          if (ShowRaw == true) { Serial.print(c); }
          rCount++;
          delay(1);
        }
        delay(1);
      }

      if (ShowCount==true) {
        Serial.print(F("["));
        Serial.print(totalIter);
        Serial.print(F("]  OpMode: "));
        Serial.print(F(OpMode));
      }
      if (ShowBatt==true) {
        uint16_t vbat;
        uint16_t vbatp;
        fona.getBattVoltage(&vbat);
        fona.getBattPercent(&vbatp);
        Serial.print(F("  VBat: ")); Serial.print(vbat); Serial.print(F("mV"));
        Serial.print(F("  VPct: ")); Serial.print(vbatp); Serial.print(F("%"));
      }
      if (ShowBand==true) {
        Serial.print(F("  Band: "));
        if (CBAND==0) { Serial.print(F("ALL_BAND")); }
        if (CBAND==1) { Serial.print(F("PGSM_MODE")); }
        if (CBAND==2) { Serial.print(F("DCS_MODE")); }
        if (CBAND==3) { Serial.print(F("PCS_MODE")); }
        if (CBAND==4) { Serial.print(F("EGSM_MODE")); }
        if (CBAND==5) { Serial.print(F("EGSM_DCS_MODE")); }
        if (CBAND==6) { Serial.print(F("EGSM_PCS_MODE")); }
        if (CBAND==7) { Serial.print(F("GSM850_MODE")); }
        if (CBAND==8) { Serial.print(F("GSM850_PCS_MODE")); }
      }
      if ((ShowCount==true) or (ShowBatt==true) or (ShowBand==true)) { 
        Serial.println(F(""));
      }      
      for (int b=0;b<(MBS-10);b++) { 
        if ( (rb[b]=='+') && (rb[b+1]=='C') && (rb[b+2]=='E') && (rb[b+3]=='N') && (rb[b+4]=='G') && (rb[b+5]==':') && (rb[b+6]==' ') && (rb[b+8]==',' && (rb[b+9]=='"')) ) {
          b = b+10;
          int lCount = 0;
          int cc = 0;
          char tt[5] = "";

          // Mobile country code, in decimal format
          while (b<(MBS-10)) {
            if (rb[b]==',') { uint16_t tNum = IntToInt(tt); CT[lCount] = tNum; break; } else { tt[cc]=rb[b]; }
            b++;
            cc++;
          }
          b++;
          lCount++;

          // Mobile network code, in decimal format
          cc = 0;
          tt[0]=' ';tt[1]=' ';tt[2]=' ';tt[3]=' ';tt[4]=' ';
          while (b<(MBS-10)) {
            if (rb[b]==',') { 
              if (rb[b+1]=='0') {
                tt[cc]=rb[b+1];
              }
              uint16_t tNum = IntToInt(tt); 
              CT[lCount] = tNum;
              break; 
            } else { 
              tt[cc]=rb[b];
            }
            b++;
            cc++;
          }
          b++;
          lCount++;

          // Location area code, in hexadecimal format
          cc = 0;
          tt[0]=' ';tt[1]=' ';tt[2]=' ';tt[3]=' ';tt[4]=' ';
          while (b<(MBS-10)) {
            if (rb[b]==',') { char cA[5]; temp.toCharArray(cA,5); CT[lCount] = HexToInt(cA); temp = ""; break; } else { temp += rb[b]; }
            b++;
          }
          b++;
          lCount++;
          
          // Cell id, in hexadecimal format
          while (b<(MBS-10)) {
            if (rb[b]==',') { char cA[5]; temp.toCharArray(cA,5); CT[lCount] = HexToInt(cA); temp = ""; break; } else { temp += rb[b]; }
            b++;
          }
          b++;
          lCount++;

          // Base station identity code, in decimal format
          cc = 0;
          tt[0]=' ';tt[1]=' ';tt[2]=' ';tt[3]=' ';tt[4]=' ';
          
          while (b<(MBS-10)) {
            if (rb[b]==',') { uint16_t tNum = IntToInt(tt); CT[lCount] = tNum; break; } else { tt[cc]=rb[b]; }
            b++;
            cc++;
          }
          b++;
          lCount++;

          // Receive level, in decimal format
          cc = 0;
          tt[0]=' ';tt[1]=' ';tt[2]=' ';tt[3]=' ';tt[4]=' ';
          while (b<(MBS-10)) {
            if (rb[b]==',') { uint16_t tNum = IntToInt(tt); CT[lCount] = tNum; break; } else { tt[cc]=rb[b]; }
            b++;
            cc++;
          }
          b = b-10;

          if (CT[0]!=0) {
            towerSeen = true;
          }
          
          if (strcmp(OpMode,"Adaptive")==0) { addToLTB(); checkLT(); }
          if (strcmp(OpMode,"Whitelist")==0) { checkWL(); }
          if (strcmp(OpMode,"Blacklist")==0) { checkBL(); }
          if (strcmp(OpMode,"Scan")==0) { addToLTB(); }
        }
        // IF no towers have been found in the first xx seconds, reset the modem.
        if ( (totalIter==ScanResetCount) && (CT[0]==0) && (LTTowers[0][0]==0) && (towerSeen==false) ) {
          fonaReset();
          totalIter=0;
        }
      }

      // Dump the list of seen towers.
      if (ShowList==true) { printLTBuffer(); }

      // Wipe the buffer clean.
      for (int n=0;n<MBS;n++) {
        rb[n] = (char)NULL; 
      }

      if (rogueSeen==true) { 
        fastBlink(1);
      }

      delay(ScanSleep);
    }
}
