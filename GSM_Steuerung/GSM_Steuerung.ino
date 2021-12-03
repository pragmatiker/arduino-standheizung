#include <SoftwareSerial.h>
#include <Regexp.h>
#define RX_PIN 11
#define TX_PIN 10
#define FLAMME_AN 2
#define FLAMME_AUS 3
//#define SIMULATE_BY_SERIAL

const unsigned long SECOND = 1000;
const unsigned long MINUTE = 60*SECOND;
const long getStatsIntervall = 5*SECOND;
long getStatswaitTime = 0;
String netTime;
String SignalQuality = "bad";
String balance = "Hoden";
String smsCMD;

const byte numChars = 64;
char receivedChars[numChars];
char nl = '\n';
boolean newData = false;


//Create software serial object to communicate with SIM800L
SoftwareSerial GSM(TX_PIN, RX_PIN); //SIM800L Tx & Rx is connected to Arduino #3 & #2

void setup()
{
  pinMode(FLAMME_AN, OUTPUT);
  pinMode(FLAMME_AUS, OUTPUT);
  digitalWrite(FLAMME_AN, 1);
  digitalWrite(FLAMME_AUS, 1);
  
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
  
  digitalWrite(FLAMME_AN, 0);
  digitalWrite(FLAMME_AUS, 0);
}

void loop()
{
  if ((millis() - getStatswaitTime) > getStatsIntervall) {
    getStats();
    getStatswaitTime = millis();
  }
  recvWithEndMarker();
  showNewData();
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
      Serial.println(netTime);
    } 
    
    result = ms.Match ("Kontostand:%s%d+,%d%d%sEUR$");
    if (result > 0) {
      balance = String(receivedChars).substring(ms.MatchStart,ms.MatchStart+ms.MatchLength);
      Serial.println(balance);
    } 
    
    result = ms.Match ("^.CSQ:%s%d%d,%d%s$");
    if (result > 0) {
      SignalQuality = String(receivedChars).substring(ms.MatchStart,ms.MatchStart+ms.MatchLength);
      Serial.println(SignalQuality);
    }
 
    result = ms.Match ("^FLAMME_AN");
    if (result > 0) {
      smsCMD = String(receivedChars).substring(ms.MatchStart,ms.MatchStart+ms.MatchLength);
      Serial.println(smsCMD);
      digitalWrite(FLAMME_AN, HIGH);
      digitalWrite(FLAMME_AUS, LOW);
    } 
    
    result = ms.Match ("^FLAMME_AUS");
    if (result > 0) {
      smsCMD = String(receivedChars).substring(ms.MatchStart,ms.MatchStart+ms.MatchLength);
      Serial.println(smsCMD);
      digitalWrite(FLAMME_AN, LOW);
      digitalWrite(FLAMME_AUS, HIGH);
    }

    result = ms.Match ("^SEND_STATS");
    if (result > 0) {
      smsCMD = String(receivedChars).substring(ms.MatchStart,ms.MatchStart+ms.MatchLength);
      Serial.println(SignalQuality);
      char str[150];
      strcpy(str, netTime.c_str());
      strcat(str, balance.c_str());
      strcat(str, SignalQuality.c_str());
      sendSMS(str,"+491713262788");
    }
       
    newData = false;
  }
}

void getStats() {
  GSM.print(F("AT+CCLK?\r\n ")); // Get time
  GSM.print(F("AT+CUSD=1,\"*106#\"\r\n ")); // Get prepayed balance
  GSM.print(F("AT+CSQ\r\n ")); // Get the Signal quality
}

void sendSMS(String message, String mobile) {
#ifdef SIMULATE_BY_SERIAL
  Serial.println("AT+CMGS=\"" + mobile + "\"\r"); //Mobile phone number to send message
  Serial.println(message);                        // This is the message to be sent.
  Serial.println((char)26);                       // ASCII code of CTRL+Z to finalized the sending of sms
#else // actual
  GSM.println("AT+CMGF=1");                    //Sets the GSM Module in Text Mode
  delay(1000);
  GSM.println("AT+CMGS=\"" + mobile + "\"\r"); //Mobile phone number to send message
  delay(1000);
  GSM.println(message);                        // This is the message to be sent.
  delay(1000);
  GSM.println((char)26);                       // ASCII code of CTRL+Z to finalized the sending of sms
  delay(1000);
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
