# Station_Blanche
Station Blanche
Présentation

Station Blanche est un système de contrôle d'accès développé dans le cadre du BTS CIEL.

Le projet permet de sécuriser l'accès à une porte grâce à une authentification par badge RFID et code PIN.

L'ensemble du système est composé d'un M5Stack CoreS3, d'un lecteur RFID, d'une API Flask hébergée sur un Raspberry Pi et d'une base de données MariaDB.

Fonctionnalités
Lecture des badges RFID via protocole Wiegand
Vérification des badges via une API Flask
Authentification par badge + code PIN
Commande d'une gâche électrique
Détection d'ouverture forcée de la porte
Journalisation des accès dans MariaDB
Interface utilisateur sur écran tactile M5Stack
Architecture
M5Stack CoreS3
Lecture du badge RFID
Affichage de l'interface utilisateur
Saisie du code PIN
Communication avec l'API
API Flask
Vérification des badges
Vérification des utilisateurs
Enregistrement des accès
MariaDB
Stockage des utilisateurs
Stockage des badges
Stockage des journaux d'accès
Technologies utilisées
C++
Arduino Framework
M5Stack CoreS3
Flask
Python
MariaDB
RFID Wiegand
WiFi
Installation
API Flask
pip install -r requirements.txt
python3 api_badge.py
Base de données

Importer le fichier :

station_blanche.sql
M5Stack

Configurer :

wifi_config.h

Puis téléverser le programme sur le M5Stack CoreS3.

Auteur

Mathéo GODARD

Projet réalisé dans le cadre du BTS CIEL.
