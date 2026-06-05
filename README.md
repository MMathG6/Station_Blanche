# Station Blanche

## Présentation du projet

Station Blanche est un système de contrôle d'accès développé dans le cadre du BTS CIEL.

L'objectif du projet est de sécuriser l'accès à une salle grâce à une authentification à deux facteurs :

1. Badge RFID
2. Code PIN

L'utilisateur présente son badge RFID au lecteur. Le badge est vérifié par une API Flask hébergée sur un Raspberry Pi. Si le badge est valide, l'utilisateur doit saisir son code PIN sur l'écran tactile du M5Stack CoreS3.

Si les deux facteurs d'authentification sont corrects, la gâche électrique est activée et l'accès est autorisé.

---

# Fonctionnalités

## Gestion des badges RFID

* Lecture des badges RFID via protocole Wiegand
* Vérification des badges via API Flask
* Gestion des badges autorisés et refusés

## Authentification

* Authentification par badge RFID
* Authentification par code PIN
* Double authentification (Badge + PIN)

## Gestion des accès

* Ouverture de la gâche électrique
* Détection de porte ouverte
* Détection de porte forcée

## Journalisation

* Enregistrement des accès autorisés
* Enregistrement des accès refusés
* Enregistrement des ouvertures forcées

---

# Architecture du projet

## M5Stack CoreS3

Rôle :

* Lecture des badges RFID
* Affichage de l'interface utilisateur
* Saisie du code PIN
* Communication avec l'API Flask
* Gestion de la gâche électrique

## Raspberry Pi

Rôle :

* Hébergement de l'API Flask
* Communication avec la base de données MariaDB

## Base de données MariaDB

Rôle :

* Stockage des utilisateurs
* Stockage des badges
* Stockage des journaux d'accès

---

# Technologies utilisées

## Matériel

* M5Stack CoreS3
* Lecteur RFID Wiegand
* Raspberry Pi
* Gâche électrique
* Capteur de porte

## Logiciel

* Arduino Framework
* C++
* Python
* Flask
* MariaDB
* Git
* GitHub

---

# Structure du projet

```text
station-blanche/
│
├── README.md
├── main.cpp
│
├── API_BADGES/
│   ├── api_badge.py
│   └── requirements.txt
```

---

# Installation

## API Flask

Installation des dépendances :

```bash
pip install -r requirements.txt
```

Lancement de l'API :

```bash
python3 api_badge.py
```

## Base de données MariaDB

La structure de la base de données est décrite dans la documentation technique du projet.

## M5Stack CoreS3

Configurer :

```cpp
wifi_config.h
```

Puis téléverser le programme sur le M5Stack CoreS3.

---

# Guide utilisateur

1. Présenter un badge RFID devant le lecteur.
2. Attendre l'affichage du clavier PIN.
3. Saisir le code PIN associé au badge.
4. Si le code PIN est correct, l'accès est autorisé.
5. Si le code PIN est incorrect, l'accès est refusé.
6. Toutes les actions sont enregistrées dans la base de données.

---

# Procédure de test

## Test 1 : Badge autorisé

Résultat attendu :

* Affichage du clavier PIN
* Demande du code PIN

## Test 2 : PIN correct

Résultat attendu :

* Affichage "ACCES AUTORISE"
* Activation de la gâche électrique

## Test 3 : PIN incorrect

Résultat attendu :

* Affichage "PIN INCORRECT"
* Refus d'accès

## Test 4 : Badge inconnu

Résultat attendu :

* Affichage "ACCES REFUSE"

## Test 5 : Porte forcée

Résultat attendu :

* Détection d'ouverture forcée
* Enregistrement dans les logs

---

# Base de données

## Tables principales

### users

Contient les utilisateurs du système.

### badges

Contient les badges RFID associés aux utilisateurs.

### acces_log

Contient l'historique complet des accès.

---

# Comptes et accès

## Raspberry Pi

Adresse IP :

```text
192.168.2.113
```

## API Flask

Port :

```text
5050
```

## Base de données MariaDB

Nom de la base :

```text
station_blanche
```

### Identifiants

Les identifiants nécessaires à la démonstration seront communiqués séparément aux professeurs.

---

# Documents fournis

Les documents suivants sont fournis avec le projet :

* README.md
* Code source M5Stack (`main.cpp`)
* API Flask (`api_badge.py`)
* Script SQL de création de la base de données (`station_blanche.sql`)
* Historique GitHub du projet
* Documentation technique
* Procédure de test

---

# Livrables fournis

* Code source M5Stack
* Code source API Flask
* Documentation du projet
* Procédure de test
* Dépôt GitHub du projet

---

# Auteur

**Mathéo GODARD**

BTS CIEL

Projet réalisé dans le cadre de l'épreuve de projet BTS CIEL.

