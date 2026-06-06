# Procédure de démarrage et de démonstration

## Projet : Station Blanche

### 1. Matériel nécessaire

* Raspberry Pi hébergeant l'API Flask
* M5Stack CoreS3
* Lecteur RFID Wiegand
* Badge RFID enregistré
* Badge RFID non enregistré
* Réseau WiFi local

---

### 2. Démarrage du Raspberry Pi

1. Mettre sous tension le Raspberry Pi.
2. Ouvrir un terminal.
3. Vérifier que le service de surveillance USB est actif.
4. Vérifier que la base de données MariaDB est démarrée.

Commande de vérification :

```bash
sudo systemctl status mariadb
```

---

### 3. Démarrage de l'API Flask

Se placer dans le dossier du projet :

```bash
cd API_BADGES
```

Lancer l'API :

```bash
python3 api_badge.py
```

Résultat attendu :

```text
Running on http://0.0.0.0:5050
```

---

### 4. Démarrage du M5Stack CoreS3

1. Alimenter le M5Stack.
2. Vérifier la connexion au réseau WiFi.
3. Vérifier l'affichage de l'écran principal.

Résultat attendu :

```text
Station_Blanche
PORTE : FERMEE
WiFi OK
```

---

### 5. Démonstration

#### Test 1 : Badge autorisé

1. Présenter un badge enregistré.
2. Vérifier l'apparition du clavier PIN.

Résultat attendu :

```text
CODE PIN
```

---

#### Test 2 : PIN correct

1. Saisir le code PIN associé.
2. Valider.

Résultat attendu :

```text
ACCES AUTORISE
```

La gâche électrique s'ouvre.

---

#### Test 3 : PIN incorrect

1. Présenter un badge valide.
2. Saisir un mauvais code PIN.

Résultat attendu :

```text
PIN INCORRECT
```

---

#### Test 4 : Badge non enregistré

1. Présenter un badge inconnu.

Résultat attendu :

```text
ACCES REFUSE
```

---

#### Test 5 : Porte forcée

1. Ouvrir la porte sans authentification.
2. Vérifier la détection d'ouverture forcée.

Résultat attendu :

```text
FORCEE
```

---

### 6. Vérification des journaux d'accès

Connexion à MariaDB :

```bash
mysql -u root -p
```

Sélection de la base :

```sql
USE station_blanche;
```

Affichage des journaux :

```sql
SELECT * FROM acces_log;
```

Résultat attendu :

Présence des accès autorisés, refusés et ouvertures forcées réalisés pendant la démonstration.

---

### 7. Arrêt du système

1. Fermer l'API Flask.
2. Éteindre le Raspberry Pi proprement :

```bash
sudo shutdown now
```

3. Débrancher le M5Stack.
