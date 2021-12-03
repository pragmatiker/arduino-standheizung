#include <SoftwareSerial.h>
#include <Regexp.h>
#define RX_PIN 11
#define TX_PIN 10
#define RELAY 2
#define MOBILENR "+491713262788"
//#define SIMULATE_BY_SERIAL

const unsigned long SECOND = 1000;
const unsigned long MINUTE = 60*SECOND;

const long getStatsIntervall = 5*SECOND;
long getStatsWaitTime = 0;

const unsigned long relayImpulseTime = 5 * SECOND; // How long will relay be on
const unsigned long  relayImpulseDebounce = 2 * MINUTE; // How long to ignore new impulse
long relayImpulseWaitTime = 0;
long relayOffAt = 0;
boolean relayImpulse = false;

String netTime, SignalQuality, balance, smsCMD;

const byte numChars = 64;
char receivedChars[numChars];
boolean newData = false;

//Create software serial object to communicate with SIM800L
SoftwareSerial GSM(TX_PIN, RX_PIN); //SIM800L Tx & Rx is connected to Arduino #3 & #2

void setup()
{
  pinMode(RELAY, OUTPUT);
   
  //Begin serial communication with Arduino and Arduino IDE (Serial Monitor)
  Serial.begin(9600);
  
  //Begin serial communication with Arduino and SIM800L
  GSM.begin(9600);

  // Enable synchronize RTC via GSM Network
  GSM.print(F("AT+CTZU=3\r\n "));
  delay(500);
  
  // Do soft restart
  GSM.print(F("AT+CFUN=1,1\r\n "));
  delay(2000);

   // Subscribe to network registration updates
  GSM.print(F("AT+CREG=1\r\n "));
  delay(500);

  // Dont store received SMS
  GSM.print(F("AT+CNMI=2,2,0,0,0\r\n "));
  delay(500);
  
  // Set text mode
  GSM.print(F("AT+CMGF=1\r\n "));
  delay(500);

  // Get Network time
  GSM.print(F("AT+CCLK?\r\n "));
  delay(500);
}

void loop()
{
  if ((millis() - getStatsWaitTime) > getStatsIntervall) {
    getStats();
    getStatsWaitTime = millis();
  }
  recvWithEndMarker();
  showNewData();
  pulseRelay();
}

//+CUSD: 0, "Kontostand: 0,57 EUR
//Bonus Guthaben: 0,00 EUR", 0
//AT+CUSD=1,"*106#"
//+CSQ: 17,5
void showNewData() {
  if (newData == true) {
    // match state object
    //Serial.println(receivedChars);
    MatchState ms;
    char result;
 
    ms.Target (receivedChars);  // set its address

    result = ms.Match ("^+CCLK:.*$");
    if (result > 0) {
      netTime = String(receivedChars).substring(ms.MatchStart,ms.MatchStart+ms.MatchLength);
      netTime += "\n";
      //Serial.println(netTime);
    } 
    
    result = ms.Match ("Kontostand:%s%d+,%d%d%sEUR$");
    if (result > 0) {
      balance = String(receivedChars).substring(ms.MatchStart,ms.MatchStart+ms.MatchLength);
      balance += "\n";
      //Serial.println(balance);
    } 
    
    result = ms.Match ("^.CSQ:%s%d%d,%d%s$");
    if (result > 0) {
      SignalQuality = String(receivedChars).substring(ms.MatchStart,ms.MatchStart+ms.MatchLength);
      SignalQuality += "\n";
      //Serial.println(SignalQuality);
    }
 
    result = ms.Match ("^FLAMME_AN");
    if (result > 0) {
      smsCMD = String(receivedChars).substring(ms.MatchStart,ms.MatchStart+ms.MatchLength);
      smsCMD += "\n";
      //Serial.println(smsCMD);
      if ((digitalRead(RELAY) == LOW) && (millis() - relayImpulseWaitTime) >= relayImpulseDebounce) {
        relayImpulse = true;
        relayImpulseWaitTime = millis();
        Serial.println(smsCMD);
      }
    } 

    result = ms.Match ("^SEND_STATS");
    if (result > 0) {
      smsCMD = String(receivedChars).substring(ms.MatchStart,ms.MatchStart+ms.MatchLength);
      smsCMD += "\n";
      Serial.println(smsCMD);
      char str[150];
      strcpy(str, smsCMD.c_str());
      strcat(str, netTime.c_str());
      strcat(str, balance.c_str());
      strcat(str, SignalQuality.c_str());
      sendSMS(str, MOBILENR);
    }
       
    newData = false;
  }
}

void getStats() {
  GSM.print(F("AT+CCLK?\r\n ")); // Get time
  GSM.print(F("AT+CUSD=1,\"*106#\"\r\n ")); // Get prepayed balance
  GSM.print(F("AT+CSQ\r\n ")); // Get the Signal quality
}

void pulseRelay() {
  if ((digitalRead(RELAY) == LOW) && relayImpulse == true) {
    digitalWrite(RELAY, HIGH);
    relayOffAt = millis() + relayImpulseTime;    
    relayImpulse = false;
    char str[150];
    strcpy(str, smsCMD.c_str());
    strcat(str, netTime.c_str());
    strcat(str, balance.c_str());
    strcat(str, SignalQuality.c_str());
    sendSMS(str, MOBILENR);
  }
  
  if (digitalRead(RELAY) == HIGH) {
    if (millis() >= relayOffAt) {
      digitalWrite(RELAY, LOW);
    }
  } 
}

void sendSMS(String message, String mobile) {
#ifdef SIMULATE_BY_SERIAL
  Serial.println("AT+CMGS=\"" + mobile + "\"\r"); //Mobile phone number to send message
  Serial.println(message);                        // This is the message to be sent.
  Serial.println((char)26);                       // ASCII code of CTRL+Z to finalized the sending of sms
#else // actual
  GSM.println("AT+CMGF=1");                    //Sets the GSM Module in Text Mode
  delay(500);
  GSM.println("AT+CMGS=\"" + mobile + "\"\r"); //Mobile phone number to send message
  delay(500);
  GSM.println(message);                        // This is the message to be sent.
  delay(500);
  GSM.println((char)26);                       // ASCII code of CTRL+Z to finalized the sending of sms
  delay(500);
#endif // SIMULATE_BY_SERIAL
}

void recvWithEndMarker() {
    static byte ndx = 0;
    char endMarker = '\n';
    char rc;
    
    while (GSM.available() > 0 && newData == false) {
        rc = GSM.read();
        
        if (rc != endMarker) {
            receivedChars[ndx] = rc;
            ndx++;
            if (ndx >= numChars) {
                ndx = numChars - 1;
            }
        }
        else {
            receivedChars[ndx] = '\0'; // terminate the string
            ndx = 0;
            newData = true;
        }
    }
}
