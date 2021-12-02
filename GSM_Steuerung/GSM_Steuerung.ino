#include <SoftwareSerial.h>
#include <Regexp.h>
#define RX_PIN 11
#define TX_PIN 10

const unsigned long SECOND = 1000;
const unsigned long MINUTE = 60*SECOND;
const long getBalanceIntervall = 120*MINUTE;
const long getSignalQualityIntervall = 120*MINUTE;
long getBalancewaitTime = 0;
long getSignalQualitywaitTime = 0; 
String balance, SignalQuality;
const byte numChars = 64;
char receivedChars[numChars];   // an array to store the received data
boolean newData = false;


//Create software serial object to communicate with SIM800L
SoftwareSerial GSM(TX_PIN, RX_PIN); //SIM800L Tx & Rx is connected to Arduino #3 & #2

void setup()
{
  
  //Begin serial communication with Arduino and Arduino IDE (Serial Monitor)
  Serial.begin(9600);
  
  //Begin serial communication with Arduino and SIM800L
  GSM.begin(9600);

  GSM.print(F("AT+CNMI=2,2,0,0,0\r\n ")); // Donmt store message receeived
  delay(500);
 
  GSM.print(F("AT+CMGF=1\r\n ")); // Set text mode
  delay(500);
  
  getBalance();
  delay(500);

  getSignalQuality();
  delay(500);

}

void loop()
{
  if ((millis() - getBalancewaitTime) > getBalanceIntervall) {
    getBalance();
    getBalancewaitTime = millis();
  }
  if ((millis() - getSignalQualitywaitTime) > getSignalQualityIntervall) {
    getSignalQuality();
    getSignalQualitywaitTime = millis();
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
    // Serial.println(receivedChars);
    MatchState ms;
    ms.Target (receivedChars);  // set its address
    char result = ms.Match ("Kontostand:%s%d+,%d%d%sEUR$");
    if (result > 0) {
      balance = String(receivedChars).substring(ms.MatchStart,ms.MatchStart+ms.MatchLength);
      Serial.println(balance);
    } 
    result = ms.Match ("^.CSQ:%s%d%d,%d%s$");
    if (result > 0) {
      SignalQuality = String(receivedChars).substring(ms.MatchStart,ms.MatchStart+ms.MatchLength);
      Serial.println(SignalQuality);
    } 
    newData = false;
  }
}

void getBalance() {
  GSM.print(F("AT+CUSD=1,\"*106#\"\r\n ")); // Get the Kontostand
}

void getSignalQuality() {
  GSM.print(F("AT+CSQ\r\n ")); // Get the SIgnal quality
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
