#include <M5CoreS3.h>
#include <Wiegand.h>

#define D0_PIN 8
#define D1_PIN 9
#define DOOR_PIN 18
#define PUSH_PIN 17

WIEGAND wg;

int doorState = 0;

bool ouvertureActive = false;
unsigned long ouvertureStart = 0;

void ecranPrincipal() {
  CoreS3.Display.clear(BLACK);
  CoreS3.Display.setTextColor(WHITE);
  CoreS3.Display.setTextSize(2);

  CoreS3.Display.setCursor(0, 0);
  CoreS3.Display.println("Station_Blanche");

  CoreS3.Display.setCursor(0, 40);
  CoreS3.Display.println("PORTE:");
}

void ouvrirPorte() {
  digitalWrite(PUSH_PIN, LOW);
  ouvertureActive = true;
  ouvertureStart = millis();
}

void setup() {
  Serial.begin(115200);
  delay(500);

  CoreS3.begin();

  pinMode(DOOR_PIN, INPUT_PULLUP);
  pinMode(PUSH_PIN, OUTPUT);
  digitalWrite(PUSH_PIN, HIGH);

  wg.begin(D0_PIN, D1_PIN);

  ecranPrincipal();

  Serial.println("SYSTEM READY");
}

void loop() {
  doorState = digitalRead(DOOR_PIN);

  if (ouvertureActive && millis() - ouvertureStart >= 3000) {
    digitalWrite(PUSH_PIN, HIGH);
    ouvertureActive = false;
  }

  if (wg.available()) {
    uint32_t code = wg.getCode();

    char uidBuffer[9];
    sprintf(uidBuffer, "%08X", code);

    String uidBadge = String(uidBuffer);

    Serial.print("Badge : ");
    Serial.println(uidBadge);

    ouvrirPorte();
  }

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
