#include <U8x8lib.h>
#define pressed LOW
#define released HIGH
#define buttonPinEnter 9
#define buttonPinDown 8
#define buttonPinUp 7
//#define buttonPinPlus 6
U8X8_SH1106_128X64_NONAME_HW_I2C u8x8( /* reset=*/ U8X8_PIN_NONE);

void setup() {
  Serial.begin(9600);
  u8x8.begin();
  u8x8.setFont(u8x8_font_chroma48medium8_r);
  pinMode(buttonPinEnter, INPUT_PULLUP);
  pinMode(buttonPinDown, INPUT_PULLUP);
  pinMode(buttonPinUp, INPUT_PULLUP);
}
