# PULSAR - Montre de Monitoring Physiologique 🏥

**Dispositif médical connecté pour la surveillance des paramètres physiologiques en milieu hospitalier**

![PULSAR Device](docs/images/pulsar-device.jpg)

> **Note**: Le code source est propriété de [Medivietech](https://www.medivietech.com) et reste confidentiel. Ce repository présente l'architecture technique, la méthodologie et les résultats du projet.

---

## 🎯 Contexte du Projet

- **Entreprise**: Medivietech (Startup MedTech incubée à AGORANOV Paris)
- **Période**: Avril 2025 - Octobre 2025 (6 mois)
- **Rôle**: Hardware/Software Engineer (Stage de fin d'études)
- **Équipe**: Neil Benhamou (CEO), Thomas Baret (CTO)
- **Validation**: Tests cliniques à la Clinique Hartmann avec 50+ patients

## 📝 Problématique

Les dispositifs de monitoring hospitaliers actuels présentent plusieurs limitations :
- Systèmes filaires encombrants limitant la mobilité du patient
- Coût élevé des solutions professionnelles existantes
- Manque de flexibilité dans la collecte et l'analyse des données
- Interfaces peu intuitives pour le personnel médical

**Mission** : Développer une montre connectée médicale autonome pour le monitoring continu des paramètres physiologiques, avec validation clinique en conditions réelles.

## 🔧 Solution Technique

### Architecture Générale

```
┌─────────────────────────────────────────────────────────────┐
│                    PULSAR Watch                              │
│  ┌──────────────┐      ┌─────────────┐    ┌──────────────┐ │
│  │  MAX86916    │─────▶│   ESP32-S3  │───▶│  SD Card     │ │
│  │ (PPG Sensor) │      │  (Main MCU) │    │  Storage     │ │
│  └──────────────┘      └─────────────┘    └──────────────┘ │
│         │                     │                    │         │
│         │                     ▼                    │         │
│         │            ┌─────────────────┐           │         │
│         └───────────▶│  FreeRTOS FIFO  │◀──────────┘         │
│                      └─────────────────┘                     │
│                             │                                │
│                             ▼                                │
│                    ┌─────────────────┐                       │
│                    │   WiFi Module   │                       │
│                    └─────────────────┘                       │
└────────────────────────────┬──────────────────────────────┘
                             │
                             ▼
                    ┌─────────────────┐
                    │    AWS IoT      │
                    │    Core MQTT    │
                    └─────────────────┘
                             │
                             ▼
                    ┌─────────────────┐
                    │  Flutter App    │
                    │  (Visualization)│
                    └─────────────────┘
```

### Stack Technique

#### Hardware
- **Microcontrôleur**: ESP32-S3 (Dual-core Xtensa LX7, WiFi/BLE)
- **Capteur PPG**: MAX86916 (SpO2, fréquence cardiaque)
- **Accéléromètre**: Détection de mouvement et d'activité
- **Stockage**: Carte SD pour enregistrement local
- **Interface**: Écran OLED pour feedback utilisateur
- **Connectivité**: WiFi (2.4GHz) + BLE 5.0

#### Firmware Embarqué
- **OS**: FreeRTOS (gestion temps réel multi-tâches)
- **Langage**: C/C++ (Arduino framework)
- **Architecture**: FIFO double buffer pour acquisition continue
- **Protocole**: MQTT over TLS 1.2 pour communication sécurisée
- **Gestion d'énergie**: Sleep modes pour autonomie optimisée

#### Cloud & Backend
- **Infrastructure**: AWS IoT Core
- **Protocole**: MQTT avec QoS 1
- **Sécurité**: Certificats X.509 pour authentification
- **Stockage**: Dual architecture (Cloud + SD card locale)

#### Application Mobile
- **Framework**: Flutter/Dart (cross-platform iOS/Android)
- **Fonctionnalités**: 
  - Visualisation temps réel des données physiologiques
  - Graphiques interactifs
  - Alertes et notifications
  - Synchronisation cloud

## 🚀 Réalisations Techniques

### 1. Résolution de Problèmes Critiques

#### Problème : Perte de données FIFO
**Symptôme** : Pertes aléatoires de données entre le capteur MAX86916 et l'ESP32, compromettant la fiabilité des mesures.

**Solution implémentée** :
- Architecture FIFO double buffer avec gestion asynchrone
- Synchronisation optimisée entre tâches FreeRTOS
- Mécanisme de récupération automatique en cas de buffer overflow
- Logging détaillé pour monitoring de la qualité d'acquisition

**Résultat** : 99.8% de fiabilité d'acquisition sur sessions de 8+ heures

#### Problème : Conflit bus SPI
**Symptôme** : Conflit entre l'accéléromètre et la carte SD sur le bus SPI, causant des corruptions de données.

**Solution implémentée** :
- Refonte complète de la gestion du bus SPI avec arbitrage
- Implémentation de mutex FreeRTOS pour accès concurrent
- Optimisation des transactions SPI (burst mode)
- Séparation des canaux DMA

**Résultat** : Stabilité parfaite avec taux d'erreur < 0.01%

### 2. Architecture Hybride Cloud + Local

**Dual storage strategy** :
- **Mode connecté** : Streaming temps réel vers AWS IoT (MQTT)
- **Mode déconnecté** : Enregistrement local sur carte SD (jusqu'à 72h d'autonomie)
- **Synchronisation automatique** : Upload des données locales à la reconnexion
- **Fallback intelligent** : Basculement automatique en cas de perte réseau

**Avantages** :
- ✅ Aucune perte de données en cas de coupure réseau
- ✅ Flexibilité d'utilisation en environnement hospitalier contraint
- ✅ Conformité RGPD avec stockage local optionnel

### 3. Validation Clinique

**Protocole de test** :
- **Lieu** : Clinique Hartmann (établissement certifié)
- **Échantillon** : 50+ patients sur 3 mois
- **Mesures** : Comparaison avec équipement médical de référence
- **Métriques** : Précision, fiabilité, confort, autonomie

**Résultats** :
- ✅ Précision cardiaque : ±2 BPM vs équipement de référence
- ✅ SpO2 : ±1% de précision
- ✅ Autonomie : 18-24h en usage continu
- ✅ Taux de satisfaction patients : 92%
- ✅ Validation du protocole pour certification médicale

## 📊 Indicateurs de Performance

| Métrique | Valeur |
|----------|--------|
| **Fiabilité d'acquisition** | 99.8% |
| **Précision cardiaque** | ±2 BPM |
| **Précision SpO2** | ±1% |
| **Autonomie batterie** | 18-24h |
| **Latence cloud** | <500ms |
| **Patients testés** | 50+ |
| **Heures de données cliniques** | 1000+ |

## 🎓 Compétences Développées

### Techniques
- Développement firmware temps réel (FreeRTOS, multi-threading)
- Intégration de capteurs physiologiques (I2C, SPI, PPG)
- Architecture IoT médicale (MQTT, AWS IoT Core, certificats X.509)
- Debugging hardware/software complexe (oscilloscope, analyseur logique)
- PCB design et prototypage électronique
- Développement mobile cross-platform (Flutter)
- Gestion de l'énergie embarquée
- Tests et validation en environnement clinique

### Méthodologiques
- Gestion de projet en environnement startup
- Collaboration avec équipes médicales
- Résolution de problèmes critiques sous contraintes
- Documentation technique pour certification médicale
- Tests en conditions réelles (environnement hospitalier)

## 📁 Documentation Disponible

- ✅ [Architecture technique détaillée](docs/architecture.md)
- ✅ [Rapport de stage complet (74 pages)](docs/rapport_stage.pdf)
- ✅ [Présentation de soutenance](docs/presentation.pdf)
- ✅ [Protocole de validation clinique](docs/validation_clinique.pdf)
- ✅ [Photos et démonstrations](docs/images/)

## 🏆 Impact & Résultats

- **Validation clinique réussie** sur 50+ patients
- **Prototype fonctionnel** prêt pour phase d'industrialisation
- **Contribution significative** au pipeline produit de Medivietech
- **Base technique solide** pour certification médicale (CE Medical Device)
- **Expérience utilisateur validée** par personnel médical et patients

## 🔒 Confidentialité

Le code source, les algorithmes propriétaires et les données cliniques sont la propriété de **Medivietech SAS** et ne sont pas publiés dans ce repository. 

Ce portfolio technique présente uniquement :
- L'architecture générale du système
- Les défis techniques rencontrés et solutions apportées
- Les résultats et métriques de validation
- Les compétences développées

Pour toute question technique sur ce projet, je suis disponible pour échanger en entretien.

---

## 📫 Contact

**Tom Huyghe** - Ingénieur Systèmes Embarqués  
📧 tom.huyghe@orange.fr  
💼 [LinkedIn](https://www.linkedin.com/in/tom-huyghe)  
🌐 [GitHub Portfolio](https://github.com/tomhyg)

---

*Développé chez Medivietech - Startup MedTech incubée à AGORANOV Paris*  
*Stage ingénieur de fin d'études - ESME SUDRIA | Avril - Octobre 2025*
