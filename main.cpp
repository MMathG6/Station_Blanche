#include <M5CoreS3.h>
#include <Wiegand.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "wifi_config.h"

/**
 * Définition des broches utilisées sur le M5Stack CoreS3.
 * D0_PIN et D1_PIN servent à lire les données du lecteur RFID avec le protocole Wiegand.
 * DOOR_PIN permet de lire l’état du capteur de porte.
 * PUSH_PIN permet de commander la gâche électrique.
 */
#define D0_PIN 8
#define D1_PIN 9
#define DOOR_PIN 18
#define PUSH_PIN 17

/**
 * Objet utilisé pour gérer la lecture des badges RFID.
 */
WIEGAND wg;

/**
 * Adresse de l’API Flask sur le Raspberry Pi.
 * API_SERVER sert à vérifier si un badge est autorisé.
 * API_LOG sert à envoyer les logs d’accès vers la base de données MariaDB.
 */
const char* API_SERVER = "http://192.168.2.113:5050/check-badge/";
const char* API_LOG = "http://192.168.2.113:5050/log-acces";

/**
 * lastBadgeTime mémorise le moment où l’accès est autorisé.
 * accessWindow correspond au temps pendant lequel une ouverture de porte est considérée normale.
 */
unsigned long lastBadgeTime = 0;
const unsigned long accessWindow = 10000;

/**
 * État actuel du capteur de porte.
 */
int doorState = 0;

/**
 * Variables permettant de gérer l’ouverture de la gâche sans bloquer le programme.
 */
bool ouvertureActive = false;
unsigned long ouvertureStart = 0;

/**
 * Indique si le système attend actuellement la saisie du code PIN.
 */
bool attentePIN = false;

/**
 * Variables liées au code PIN.
 * pinAttendu vient de la base de données.
 * pinSaisi est entré par l’utilisateur sur l’écran tactile.
 */
String pinAttendu = "";
String pinSaisi = "";

/**
 * Informations de l’utilisateur en cours d’authentification.
 */
String prenomActuel = "";
String uidBadgeActuel = "";
int idUserActuel = 0;

/**
 * Connexion au réseau WiFi.
 * Les identifiants sont stockés dans le fichier wifi_config.h.
 */
static bool setWifi() {
  WiFi.mode(WIFI_STA);
  WiFi.setSleep(false);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  Serial.print("Connexion WiFi");

  uint32_t start = millis();

  while (WiFi.status() != WL_CONNECTED && millis() - start < 20000) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("WiFi CONNECTE");
    Serial.print("IP : ");
    Serial.println(WiFi.localIP());
    return true;
  }

  Serial.println("WiFi ECHEC");
  return false;
}

/**
 * Activation de la gâche électrique.
 * Le GPIO passe à LOW pour déclencher l’ouverture.
 * La remise à HIGH est faite plus tard dans loop().
 */
static void ouvrirPorte() {
  digitalWrite(PUSH_PIN, LOW);
  ouvertureActive = true;
  ouvertureStart = millis();
}

/**
 * Envoi d’un log d’accès vers l’API.
 * L’API insère ensuite ces informations dans la table acces_log.
 */
static void envoyerLogAcces(int idUser, String methodeAuth, String resultat, String porte) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi non connecte, log impossible");
    return;
  }

  HTTPClient http;

  http.begin(API_LOG);
  http.addHeader("Content-Type", "application/json");

  /**
   * Création du JSON envoyé à l’API.
   * Les données sont volontairement simples pour être insérées facilement en base.
   */
  JsonDocument doc;
  doc["id_user"] = idUser;
  doc["methode_auth"] = methodeAuth;
  doc["resultat"] = resultat;
  doc["porte"] = porte;

  String body;
  serializeJson(doc, body);

  int httpCode = http.POST(body);

  Serial.print("Code HTTP log : ");
  Serial.println(httpCode);

  http.end();
}

/**
 * Vérification du badge avec l’API Flask.
 * Le M5Stack envoie l’UID du badge.
 * L’API renvoie les informations utilisateur si le badge est autorisé.
 */
static bool verifierBadgeAPI(String uidBadge, String &prenom, String &pin, int &idUser) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi non connecte");
    return false;
  }

  HTTPClient http;

  /**
   * Construction de l’URL de vérification.
   * L’UID du badge est ajouté à la fin de l’URL.
   */
  String url = String(API_SERVER) + uidBadge;

  http.begin(url);

  int httpCode = http.GET();

  if (httpCode <= 0) {
    Serial.print("Erreur HTTP : ");
    Serial.println(httpCode);
    http.end();
    return false;
  }

  String payload = http.getString();

  Serial.print("Code HTTP verification badge : ");
  Serial.println(httpCode);

  http.end();

  /**
   * Si l’API ne répond pas en 200, le badge est refusé.
   */
  if (httpCode != 200) {
    return false;
  }

  JsonDocument doc;

  DeserializationError error = deserializeJson(doc, payload);

  if (error) {
    Serial.print("Erreur JSON : ");
    Serial.println(error.c_str());
    return false;
  }

  bool authorized = doc["authorized"] | false;

  if (!authorized) {
    return false;
  }

  /**
   * Récupération des données utilisateur.
   * Le PIN n’est pas affiché dans le terminal pour des raisons de sécurité.
   */
  prenom = doc["prenom"].as<String>();
  pin = doc["pin"].as<String>();
  idUser = doc["id_user"] | 0;

  return true;
}

/**
 * Affiche le PIN saisi sous forme d’étoiles.
 * Le PIN réel n’est jamais affiché sur l’écran.
 */
void afficherPIN() {
  CoreS3.Display.fillRect(40, 35, 240, 30, BLACK);
  CoreS3.Display.drawRect(40, 35, 240, 30, WHITE);

  CoreS3.Display.setTextColor(WHITE);
  CoreS3.Display.setTextSize(2);
  CoreS3.Display.setCursor(55, 42);

  for (int i = 0; i < pinSaisi.length(); i++) {
    CoreS3.Display.print("*");
  }
}

/**
 * Dessine un bouton tactile à l’écran.
 */
void dessinerBouton(int x, int y, int w, int h, String texte, uint16_t couleur) {
  CoreS3.Display.fillRoundRect(x, y, w, h, 8, couleur);
  CoreS3.Display.drawRoundRect(x, y, w, h, 8, WHITE);

  CoreS3.Display.setTextColor(BLACK);
  CoreS3.Display.setTextSize(2);

  int textX = x + 20;

  if (texte == "OK") {
    textX = x + 15;
  }

  if (texte == "<") {
    textX = x + 22;
  }

  CoreS3.Display.setCursor(textX, y + 9);
  CoreS3.Display.print(texte);

  CoreS3.Display.setTextColor(WHITE);
}

/**
 * Affiche le clavier PIN tactile.
 * Le bouton rouge supprime le dernier chiffre.
 * Le bouton vert valide le PIN.
 */
void dessinerClavier() {
  CoreS3.Display.clear(BLACK);
  CoreS3.Display.setTextColor(WHITE);
  CoreS3.Display.setTextSize(2);

  CoreS3.Display.setCursor(90, 5);
  CoreS3.Display.print("CODE PIN");

  afficherPIN();

  dessinerBouton(40, 65, 60, 35, "1", LIGHTGREY);
  dessinerBouton(130, 65, 60, 35, "2", LIGHTGREY);
  dessinerBouton(220, 65, 60, 35, "3", LIGHTGREY);

  dessinerBouton(40, 105, 60, 35, "4", LIGHTGREY);
  dessinerBouton(130, 105, 60, 35, "5", LIGHTGREY);
  dessinerBouton(220, 105, 60, 35, "6", LIGHTGREY);

  dessinerBouton(40, 145, 60, 35, "7", LIGHTGREY);
  dessinerBouton(130, 145, 60, 35, "8", LIGHTGREY);
  dessinerBouton(220, 145, 60, 35, "9", LIGHTGREY);

  dessinerBouton(40, 185, 60, 35, "<", RED);
  dessinerBouton(130, 185, 60, 35, "0", LIGHTGREY);
  dessinerBouton(220, 185, 60, 35, "OK", GREEN);
}

/**
 * Interface principale affichée au démarrage.
 * L’UID du badge n’est pas affiché sur l’écran pour éviter qu’un utilisateur le voie.
 */
void ecranPrincipal() {
  CoreS3.Display.clear(BLACK);
  CoreS3.Display.setTextColor(WHITE);
  CoreS3.Display.setTextSize(2);

  CoreS3.Display.setCursor(0, 0);
  CoreS3.Display.println("Station_Blanche");

  CoreS3.Display.setCursor(0, 40);
  CoreS3.Display.println("PORTE:");
}

/**
 * Validation du code PIN.
 * Si le PIN est correct, la porte s’ouvre.
 * Si le PIN est incorrect, l’accès est refusé.
 */
void validerPIN() {
  if (pinSaisi == pinAttendu) {
    CoreS3.Display.clear(GREEN);
    CoreS3.Display.setTextColor(BLACK);
    CoreS3.Display.setTextSize(2);
    CoreS3.Display.setCursor(40, 100);
    CoreS3.Display.println("ACCES AUTORISE");

    Serial.println("PIN correct");

    /**
     * L’accès est considéré comme autorisé seulement après validation du PIN.
     */
    lastBadgeTime = millis();
    ouvrirPorte();

    envoyerLogAcces(
      idUserActuel,
      "BADGE + PIN",
      "ACCES AUTORISE",
      "OUVERTE"
    );

    delay(1000);

    attentePIN = false;
    pinSaisi = "";

    ecranPrincipal();
  } else {
    CoreS3.Display.clear(RED);
    CoreS3.Display.setTextColor(WHITE);
    CoreS3.Display.setTextSize(2);
    CoreS3.Display.setCursor(50, 100);
    CoreS3.Display.println("PIN INCORRECT");

    Serial.println("PIN incorrect");

    envoyerLogAcces(
      idUserActuel,
      "BADGE + PIN",
      "PIN INCORRECT",
      "FERMEE"
    );

    M5.Speaker.tone(600, 500);

    delay(1000);

    pinSaisi = "";

    dessinerClavier();
  }
}

/**
 * Gestion des appuis sur le clavier tactile.
 * Les coordonnées X et Y permettent de savoir quelle touche a été appuyée.
 */
void gererClavierTactile() {
  CoreS3.update();

  auto touch = CoreS3.Touch.getDetail();

  if (!touch.wasPressed()) {
    return;
  }

  int x = touch.x;
  int y = touch.y;

  String touche = "";

  /**
   * Touches 1, 2 et 3.
   */
  if (y >= 65 && y <= 100) {
    if (x >= 40 && x <= 100) {
      touche = "1";
    } else if (x >= 130 && x <= 190) {
      touche = "2";
    } else if (x >= 220 && x <= 280) {
      touche = "3";
    }
  }

  /**
   * Touches 4, 5 et 6.
   */
  else if (y >= 105 && y <= 140) {
    if (x >= 40 && x <= 100) {
      touche = "4";
    } else if (x >= 130 && x <= 190) {
      touche = "5";
    } else if (x >= 220 && x <= 280) {
      touche = "6";
    }
  }

  /**
   * Touches 7, 8 et 9.
   */
  else if (y >= 145 && y <= 180) {
    if (x >= 40 && x <= 100) {
      touche = "7";
    } else if (x >= 130 && x <= 190) {
      touche = "8";
    } else if (x >= 220 && x <= 280) {
      touche = "9";
    }
  }

  /**
   * Bouton rouge, touche 0 et bouton vert.
   */
  else if (y >= 185 && y <= 220) {
    if (x >= 40 && x <= 100) {
      if (pinSaisi.length() > 0) {
        pinSaisi.remove(pinSaisi.length() - 1);
      }

      afficherPIN();
      return;
    }

    else if (x >= 130 && x <= 190) {
      touche = "0";
    }

    else if (x >= 220 && x <= 280) {
      validerPIN();
      return;
    }
  }

  /**
   * Ajout de la touche au PIN saisi.
   */
  if (touche != "") {
    if (pinSaisi.length() < 8) {
      pinSaisi += touche;
      afficherPIN();
    }
  }
}

/**
 * Initialisation du système.
 */
void setup() {
  Serial.begin(115200);
  delay(500);

  CoreS3.begin();

  CoreS3.Display.setTextSize(2);
  CoreS3.Display.clear(BLACK);

  /**
   * Capteur de porte configuré en entrée.
   */
  pinMode(DOOR_PIN, INPUT_PULLUP);

  /**
   * Gâche électrique configurée en sortie.
   * HIGH correspond à l’état de repos.
   */
  pinMode(PUSH_PIN, OUTPUT);
  digitalWrite(PUSH_PIN, HIGH);

  /**
   * Initialisation du lecteur RFID Wiegand.
   */
  wg.begin(D0_PIN, D1_PIN);

  M5.Speaker.setVolume(200);

  ecranPrincipal();

  CoreS3.Display.setCursor(0, 160);
  CoreS3.Display.println("WiFi...");

  if (setWifi()) {
    CoreS3.Display.fillRect(0, 160, 320, 25, BLACK);
    CoreS3.Display.setCursor(0, 160);
    CoreS3.Display.println("WiFi OK");
  } else {
    CoreS3.Display.fillRect(0, 160, 320, 25, BLACK);
    CoreS3.Display.setCursor(0, 160);
    CoreS3.Display.println("WiFi ECHEC");
  }

  Serial.println("SYSTEM READY");
}

/**
 * Boucle principale du programme.
 */
void loop() {
  static String lastDoorText = "";

  /**
   * Fermeture automatique de la gâche après 3 secondes.
   */
  if (ouvertureActive && millis() - ouvertureStart >= 3000) {
    digitalWrite(PUSH_PIN, HIGH);
    ouvertureActive = false;
  }

  /**
   * Si un PIN est attendu, on gère uniquement le clavier tactile.
   */
  if (attentePIN) {
    gererClavierTactile();
    delay(50);
    return;
  }

  doorState = digitalRead(DOOR_PIN);

  /**
   * Lecture du badge RFID.
   */
  if (wg.available()) {
    uint32_t code = wg.getCode();

    char uidBuffer[9];
    sprintf(uidBuffer, "%08X", code);

    String uidBadge = String(uidBuffer);

    uidBadgeActuel = uidBadge;

    /**
     * L’UID du badge est affiché uniquement dans le Serial Monitor.
     */
    Serial.print("Badge : ");
    Serial.println(uidBadge);

    String prenom = "";
    String pin = "";
    int idUser = 0;

    bool badgeAutorise = verifierBadgeAPI(uidBadge, prenom, pin, idUser);

    if (badgeAutorise) {
      Serial.println("Badge autorise");

      prenomActuel = prenom;
      pinAttendu = pin;
      pinSaisi = "";
      idUserActuel = idUser;

      /**
       * Après validation du badge, le clavier PIN s’affiche.
       */
      attentePIN = true;

      dessinerClavier();
    } else {
      CoreS3.Display.fillRect(0, 200, 320, 40, BLACK);
      CoreS3.Display.setCursor(0, 200);
      CoreS3.Display.print("ACCES REFUSE");

      Serial.println("Badge refuse");

      envoyerLogAcces(
        0,
        "BADGE",
        "BADGE REFUSE",
        "FERMEE"
      );

      M5.Speaker.tone(600, 300);
    }
  }

  String currentDoorText;

  /**
   * Analyse de l’état de la porte.
   */
  if (doorState == HIGH) {
    currentDoorText = "FERMEE";
  } else {
    if (millis() - lastBadgeTime < accessWindow) {
      currentDoorText = "OUVERTE";
    } else {
      currentDoorText = "FORCEE";

      envoyerLogAcces(
        idUserActuel,
        "CAPTEUR",
        "PORTE FORCEE",
        "FORCEE"
      );

      M5.Speaker.tone(1000, 300);
    }
  }

  /**
   * Mise à jour de l’affichage uniquement si l’état de la porte change.
   */
  if (currentDoorText != lastDoorText) {
    CoreS3.Display.fillRect(120, 40, 200, 25, BLACK);
    CoreS3.Display.setCursor(120, 40);
    CoreS3.Display.print(currentDoorText);
    lastDoorText = currentDoorText;
  }

  delay(50);
}
