// Leave menue after inactivity
const int menueTimeout = 5000;
// Number of items in the Menue
const int NumMenueItems = 4 ;
// Indexing the MenueItems Array
const int MenueItemName = 0;
const int MenueItemUnit = 1;
// Indexing values of parameter array
const int paramMin = 0;
const int paramMax = 1;
const int paramStepSize = 2;
const int columns = 3;
const int paramSetpoint = 4;
// Definition of parameters
const int UhrzeitMin = 1;
const int UhrzeitMax = 2400;
const int UhrzeitStepSize = 15;
const int UhrzeitCols = 4;
const int UhrzeitSoll = 1600;
const int TempMin = 16;
const int TempMax = 26;
const int TempStepSize = 1;
const int TempCols = 2;
const int TempSoll = 18;
const int DauerMin = 30;
const int DauerMax = 90;
const int DauerStepSize = 5;
const int DauerCols = 2;
const int DauerSoll = 60;
const int HystereseMin = 2;
const int HystereseMax = 5;
const int HystereseStepSize = 1;
const int HystereseCols = 1;
const int HystereseSoll = 3;

const int debounceTime = 300; 
int buttonStateEnter = released;   // variable for reading the pushbutton status
int buttonStateDown = released;   // variable for reading the pushbutton status
int buttonStateUp = released;    // variable for reading the pushbutton status
int lastbuttonStateEnter = released;   // variable for reading the pushbutton status
int lastbuttonStateDown = released;   // variable for reading the pushbutton status
int lastbuttonStateUp = released;    // variable for reading the pushbutton status
int inMenue = 0; // Did whe press enter?
long waitTime = 0; // Zeit zu warten bis menue verlassen und gepeichert wird

int Menuepoint = NumMenueItems;
String MenueItems[NumMenueItems][2] = {{"Startzeit: ","h"},
                                       {"Temp:", "C"},
                                       {"Dauer: ","min"},
                                       {"Hyterese:", "C"}};
                               
int parameters[NumMenueItems][5] = {{UhrzeitMin,UhrzeitMax,UhrzeitStepSize,UhrzeitCols,UhrzeitSoll},
                                    {TempMin,TempMax,TempStepSize,TempCols,TempSoll},
                                    {DauerMin,DauerMax,DauerStepSize,DauerCols,DauerSoll},
                                    {HystereseMin,HystereseMax,HystereseStepSize,HystereseCols,HystereseSoll}};
void Menue() {
  buttonStateEnter = digitalRead(buttonPinEnter);
  buttonStateDown = digitalRead(buttonPinDown);
  buttonStateUp = digitalRead(buttonPinUp);

  // Enter Menue point by pressing Enter
  if (buttonStateEnter == pressed && lastbuttonStateEnter == released && (millis() - waitTime) > debounceTime){
    waitTime = millis();
    //inMenue = (inMenue == 0) ? 1 : 0;
    if (inMenue == 0) {
      inMenue = 1;
    } else {
      Menuepoint = NumMenueItems;
      inMenue = 0;
    }
  }
  
  // go DOWN in menue
  if (buttonStateDown == pressed && lastbuttonStateDown == released && inMenue != 1){
    waitTime = millis();
    if (Menuepoint == NumMenueItems) {
      Menuepoint = 0;
    } else {
      Menuepoint++;
    }
  }
  
  // go UP in menue
  else if(buttonStateUp == pressed && lastbuttonStateUp == released && inMenue != 1){
    waitTime = millis();
    if (Menuepoint == 0) {
      Menuepoint = NumMenueItems;
    } else {
      Menuepoint--;
    }
  }
  
  // DECREMENT setpoint
  else if (buttonStateDown == pressed && inMenue == 1 && (millis() - waitTime) > debounceTime){
    waitTime = millis();
    if (parameters[Menuepoint][paramSetpoint] > parameters[Menuepoint][paramMin]) {
      parameters[Menuepoint][paramSetpoint] -= parameters[Menuepoint][paramStepSize];
    }
  } 
  // INCREMENT setpoint
  else if (buttonStateUp == pressed && inMenue == 1 && (millis() - waitTime) > debounceTime){
    waitTime = millis();
    if (parameters[Menuepoint][paramSetpoint] < parameters[Menuepoint][paramMax]) {
      parameters[Menuepoint][paramSetpoint] += parameters[Menuepoint][paramStepSize];
    }
  }
 
  // Hop out the menue after menueTimeout
  if ((millis() - waitTime) > menueTimeout) {
    //speichern()
    Menuepoint = NumMenueItems;
    inMenue = 0;
    waitTime = millis();
  }
  
  // Save buttonState for next round of loop
  lastbuttonStateDown = buttonStateDown;
  lastbuttonStateUp = buttonStateUp;
  
  // draw the Menue
  for (int menueIndex = 0; menueIndex<NumMenueItems; menueIndex++) {
    if (Menuepoint == menueIndex) {
      u8x8.inverse();
    } else {
      u8x8.noInverse();
    }
    u8x8.setCursor(0,menueIndex*2);
    u8x8.print(MenueItems[menueIndex][MenueItemName]);
    u8x8.setCursor(u8x8.getCols()-parameters[menueIndex][columns]-MenueItems[menueIndex][MenueItemUnit].length(),menueIndex*2);
    u8x8.print(u8x8_u16toa(parameters[menueIndex][paramSetpoint], parameters[menueIndex][columns]));
    u8x8.print(MenueItems[menueIndex][MenueItemUnit]); 
  }
  
  delay(20);
}
