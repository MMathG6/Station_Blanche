#include <M5CoreS3.h>

#define DOOR_PIN 18

int doorState = 0;

void ecranPrincipal() {
  CoreS3.Display.clear(BLACK);
  CoreS3.Display.setTextColor(WHITE);
  CoreS3.Display.setTextSize(2);

  CoreS3.Display.setCursor(0, 0);
  CoreS3.Display.println("Station_Blanche");

  CoreS3.Display.setCursor(0, 40);
  CoreS3.Display.println("PORTE:");
}

void setup() {
  Serial.begin(115200);
  delay(500);

  CoreS3.begin();

  pinMode(DOOR_PIN, INPUT_PULLUP);

  ecranPrincipal();

  Serial.println("SYSTEM READY");
}

void loop() {
  doorState = digitalRead(DOOR_PIN);

  String etatPorte = "";

  if (doorState == HIGH) {
    etatPorte = "FERMEE";
  } else {
    etatPorte = "OUVERTE";
  }

  CoreS3.Display.fillRect(120, 40, 200, 25, BLACK);
  CoreS3.Display.setCursor(120, 40);
  CoreS3.Display.print(etatPorte);

  delay(100);
}
