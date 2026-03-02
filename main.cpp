#include <M5CoreS3.h>

void setup() {
  Serial.begin(115200);
  delay(500);

  CoreS3.begin();

  CoreS3.Display.clear(BLACK);
  CoreS3.Display.setTextColor(WHITE);
  CoreS3.Display.setTextSize(2);
  CoreS3.Display.setCursor(0, 0);
  CoreS3.Display.println("Station_Blanche");

  Serial.println("SYSTEM READY");
}

void loop() {
}
