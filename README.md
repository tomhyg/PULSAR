# 🏥 PULSAR - Montre Physiologique Médicale

> **Système de monitoring physiologique de précision médicale validé sur 50+ patients en milieu hospitalier**
> 
> Conçu et développé lors de mon stage de fin d'études chez **Medivietech** (Startup MedTech incubée à AGORANOV Paris)

[![Ingénierie](https://img.shields.io/badge/Ingénierie-Mécatronique-blue)](https://www.esme.fr)
[![Hardware](https://img.shields.io/badge/Hardware-ESP32--S3%20%7C%20Nordic%20nRF5340-green)](#)
[![Médical](https://img.shields.io/badge/Médical-Validation%20Clinique-red)](https://www.medivietech.com)
[![Licence](https://img.shields.io/badge/Licence-Portfolio%20Uniquement-yellow)](#licence)

---

## 📸 Présentation du Projet

<p align="center">
  <img src="images/prototypes/pulsar-final-renders.jpg" width="800" alt="PULSAR - Rendus 3D finaux (3 variantes de couleur)"/>
  <br/>
  <em>Rendus CAO professionnels des prototypes PULSAR (variantes Blanc, Bleu, Bordeaux)</em>
</p>

<p align="center">
  <img src="images/prototypes/pulsar-dual-watches.jpg" width="600" alt="Prototypes physiques PULSAR-009 et PULSAR-007"/>
  <br/>
  <em>Prototypes physiques PULSAR-009 et PULSAR-007 avec bracelets silicone médical</em>
</p>

---

## 🎯 Vue d'Ensemble

**PULSAR** est un dispositif médical portable conçu pour le **monitoring physiologique continu** en environnement hospitalier. Contrairement aux wearables grand public (Fitbit, Garmin, WHOOP), PULSAR atteint une **précision clinique** validée face à des équipements médicaux professionnels.

### Réalisations Clés

- ✅ **50+ patients** monitorés à la Clinique Hartmann (Neuilly-sur-Seine)
- ✅ **400+ heures** de données cliniques collectées
- ✅ **99,2% de disponibilité** durant la période de validation de 3 mois
- ✅ **15+ prototypes fonctionnels** produits
- ✅ **0% de perte de données** grâce à l'architecture FIFO optimisée
- ✅ **8-10h d'autonomie** en mode acquisition continue (prototypes ESP32-S3)
- ✅ **3-5 jours d'autonomie** visés sur la version industrielle Nordic nRF5340

---

## 🔬 Points Techniques Forts

### Capteur PPG Multi-Spectral
- **MAX86916** capteur optique 4 canaux (Rouge, Infrarouge, Vert, Bleu)
- **100 Hz par LED** fréquence d'échantillonnage
- **Résolution ADC 19-bit** (32 768 niveaux)
- **Architecture PCB déporté** via nappe flexible FPC

### Innovation Hardware
- **ESP32-S3** microcontrôleur dual-core (240 MHz) pour prototypage
- **Nordic nRF5340** dual-core ARM Cortex-M33 pour industrialisation
- **Architecture dual-mode** : WiFi + AWS **ou** enregistrement SD autonome
- **Fuel gauge** intégré pour monitoring précis de batterie
- **Accéléromètre 3 axes** pour détection d'artefacts de mouvement
- **Conception mécanique custom** (impression 3D, SolidWorks)

### Métriques de Performance

| Métrique | Spécification | Résultat |
|----------|--------------|----------|
| **Fréquence d'échantillonnage** | 100 Hz/canal | ✅ Stable |
| **Intégrité des données** | 0% de perte requis | ✅ 0% de perte mesuré |
| **Autonomie ESP32-S3** | 8h minimum | ✅ 8-10h validé |
| **Autonomie Nordic (cible)** | N/A | 🎯 3-5 jours |
| **Disponibilité clinique** | 95%+ requis | ✅ 99,2% atteint |
| **Vitesse d'upload** | N/A | 40 Ko/s via WiFi |

---

## 📐 Architecture Système

### Architecture Matérielle - Prototypes ESP32-S3

```
┌──────────────────────────────────────────────────────────────┐
│                    Architecture PULSAR (ESP32)                │
├──────────────────────────────────────────────────────────────┤
│                                                                │
│  ┌──────────────┐      ┌────────────────────────────┐        │
│  │  MAX86916    │◄────►│   ESP32-S3-WROOM-1-N      │        │
│  │  Capteur PPG │ FPC  │   Dual-Core 240MHz        │        │
│  │  (Déporté)   │ 6pin │   8MB PSRAM               │        │
│  └──────────────┘      └────────────────────────────┘        │
│         │                         │                           │
│         │                         ├─► [LIS3DHTR] Accéléro    │
│         │                         ├─► [MAX1704x] Fuel Gauge  │
│         │                         ├─► [Slot Carte SD]        │
│         │                         └─► [LED NeoPixel]         │
│         │                                                      │
│         └─► I2C @ 400kHz (via TCA9509 level shifter 3.3V→1.8V)│
│                                                                │
│  Alimentation : LiPo 3.7V 850mAh → 8-10h d'autonomie         │
└──────────────────────────────────────────────────────────────┘
```

### Architecture Matérielle - Version Industrielle Nordic nRF5340

```
┌──────────────────────────────────────────────────────────────┐
│              Architecture PULSAR (Nordic nRF5340)             │
├──────────────────────────────────────────────────────────────┤
│                                                                │
│  ┌──────────────┐      ┌────────────────────────────┐        │
│  │  MAX86916    │◄────►│   ISP2053-AX (nRF5340)    │        │
│  │  Capteur PPG │ I2C  │   Dual ARM Cortex-M33     │        │
│  │  4 canaux    │400kHz│   App Core @ 128MHz       │        │
│  └──────────────┘      │   Net Core @ 64MHz        │        │
│                        └────────────────────────────┘        │
│                                  │                            │
│                                  ├─► [LIS2DE12TR] Accéléro   │
│                                  ├─► [MAX30208] Température  │
│                                  ├─► [BQ25170] Chargeur      │
│                                  ├─► [MX25R6435F] NOR Flash  │
│                                  └─► [AN7002Q-P] WiFi 6      │
│                                                                │
│  Alimentation : LiPo 3.7V 550mAh → 3-5 jours d'autonomie     │
└──────────────────────────────────────────────────────────────┘
```

**[📖 Documentation Architecture Détaillée](docs/architecture.md)**

---

## 🛠️ Défis Techniques Résolus

### 1. **Résolution du Débordement FIFO** ❌→✅
**Problème** : L'implémentation originale rejetait 29 échantillons sur 30  
**Solution** : Refonte complète de la gestion FIFO → **0% de perte de données**

### 2. **Conflits Bus SPI** ❌→✅
**Problème** : Carte SD + Accéléromètre partageant le bus SPI causaient des plantages  
**Solution** : Basculement en mode SPI exclusif avec temporisation appropriée

### 3. **Ergonomie PCB** ❌→✅
**Problème** : PCB carré inconfortable pour port au poignet  
**Solution** : Capteur déporté circulaire (Ø10mm) via nappe FPC flexible

### 4. **Transition R&D → Production** 🔄
**Défi actuel** : Migration ESP32-S3 → Nordic nRF5340 pour industrialisation  
**Objectif** : Multiplier l'autonomie par 8 tout en conservant les performances

**[🔧 Défis Techniques Détaillés](docs/challenges.md)**

---

## 🏥 Validation Clinique

### Déploiement Hospitalier
- **Lieu** : Clinique Hartmann, Neuilly-sur-Seine (92)
- **Superviseur** : Dr. Lee (Service de Réanimation)
- **Durée** : 3 mois de tests continus
- **Patients** : 50+ (démographies diverses)
- **Sessions** : 24 enregistrements de 5 minutes par patient

### Résultats de Validation

| Paramètre | Objectif | Résultat |
|-----------|----------|----------|
| **Disponibilité** | 95% minimum | 99,2% atteint ✅ |
| **Qualité des données** | Précision clinique | Validée ✅ |
| **Confort utilisateur** | Confortable | Retours positifs ✅ |
| **Sécurité** | Zéro incident | Zéro incident ✅ |

**[📊 Rapport de Validation Complet](docs/validation.md)**

---

## 🔄 Évolution du Projet

### Phase 1 : Analyse du Prototype Hérité (Avril 2024)
- Diagnostic du problème critique de débordement FIFO
- Identification des conflits bus SPI
- Évaluation des limitations ergonomiques

### Phase 2 : Refonte Architecture (Mai-Juin 2024)
- Implémentation du design capteur déporté
- Résolution du pipeline d'acquisition de données
- Ajout de la capacité dual-mode (WiFi/SD)

### Phase 3 : Validation Clinique (Juillet-Septembre 2024)
- Déploiement de 15 prototypes à la Clinique Hartmann
- Collection de 400+ heures de données patients
- Atteinte de 99,2% de disponibilité système

### Phase 4 : Préparation Industrialisation (Septembre 2024)
- Collaboration avec EMBRILL (Inde) pour migration Nordic nRF5340
- Transfert de connaissances pour design production
- Objectif : 3-5 jours d'autonomie batterie (amélioration ×8)

---

## 📂 Structure du Repository

```
PULSAR/
├── docs/                      # Documentation technique
│   ├── architecture.md        # Architecture système détaillée
│   ├── challenges.md          # Problèmes techniques résolus
│   └── validation.md          # Résultats validation clinique
├── hardware/                  # Spécifications matérielles
│   ├── README.md             # Vue d'ensemble hardware
│   └── component-list.md     # Liste complète des composants (BOM)
└── images/                    # Documentation visuelle
    ├── prototypes/           # Photos dispositifs physiques
    ├── hardware/             # Photos PCB et composants
    ├── cad/                  # Rendus SolidWorks
    └── architecture/         # Diagrammes techniques
```

---

## 🎓 Compétences Démontrées

### **Ingénierie Hardware**
- Conception PCB (EasyEDA Pro)
- Intégration de capteurs (protocoles I2C, SPI)
- Optimisation de gestion d'énergie
- Conception CAO mécanique (Fusion 360, SolidWorks)

### **Software Embarqué**
- Développement firmware ESP32-S3 (Arduino/ESP-IDF)
- Acquisition de données temps réel (concepts RTOS)
- Optimisation mémoire (gestion PSRAM)
- Implémentation de protocoles (I2C, SPI, carte SD)

### **Architecture Système**
- Communication dual-mode (WiFi/SD)
- Conception de pipeline de données (chunking JSON, encodage Base64)
- Intégration cloud (AWS S3)
- Optimisation autonomie batterie

### **Développement Dispositif Médical**
- Méthodologie de validation clinique
- Assurance qualité données de précision médicale
- Protocoles de déploiement hospitalier
- Considérations réglementaires (parcours marquage CE)

### **Collaboration Internationale**
- Transfert de connaissances techniques (EMBRILL, Inde)
- Documentation professionnelle multilingue
- Gestion de projet R&D en environnement startup
- Communication avec équipes médicales

---

## 🔬 Technologies Utilisées

### **Prototypage R&D (ESP32-S3)**
- **Microcontrôleur** : ESP32-S3-WROOM-1-N (Dual-core Xtensa @ 240MHz, 8MB PSRAM)
- **Capteur PPG** : MAX86916EFD+ (4 canaux, 19-bit ADC)
- **Accéléromètre** : LIS3DHTR (3 axes @ 100Hz)
- **Fuel Gauge** : MAX1704x (I2C)
- **Stockage** : MicroSD card (FAT32, LittleFS)
- **Connectivité** : WiFi 802.11 b/g/n, Bluetooth 5.0 LE

### **Industrialisation (Nordic nRF5340)**
- **Microcontrôleur** : ISP2053-AX nRF5340 (Dual ARM Cortex-M33)
- **Capteur PPG** : MAX86916 (conservation écosystème capteurs)
- **IMU** : LIS2DE12TR (3 axes, low-power)
- **Température** : MAX30208 (±0.1°C précision clinique)
- **Flash** : MX25R6435F (8MB NOR Flash QSPI)
- **WiFi** : AN7002Q-P (nRF7002, WiFi 6 + BLE 5.3)
- **RTOS** : Zephyr Project

### **Logiciels & Outils**
- **IDE** : VS Code, Arduino IDE, nRF Connect SDK
- **CAO Mécanique** : SolidWorks, Fusion 360
- **CAO Électronique** : EasyEDA Pro
- **Langages** : C/C++ (embarqué), Python (outils extraction)
- **Cloud** : AWS S3, AWS Lambda
- **Versioning** : Git, GitHub

---

## 📚 Projets Connexes

Ce projet s'inscrit dans une mission R&D plus large chez Medivietech :

- **PULSAR** (ce repo) : Montre médicale portable
- **BABYCAM** : Caméra bébé intelligente avec IA embarquée (ESP32-P4)
- **Migration Nordic** : Productisation industrielle nRF5340

---

## 🤝 Contexte Professionnel

**Entreprise** : Medivietech (Startup MedTech)  
**Localisation** : AGORANOV Paris (Incubateur Deep-Tech de référence en France)  
**Rôle** : Stagiaire Ingénieur Hardware/Software (6 mois)  
**Équipe** : CEO (Neil Benhamou), CTO (Thomas Baret - IA/Data), Ingénieur QARA  
**Collaboration** : EMBRILL (Inde) pour l'industrialisation  
**Période** : Avril - Octobre 2024

---

## 📄 Licence

**Usage Portfolio & Référence Uniquement**

Ce repository présente mon travail d'ingénierie réalisé durant mon stage chez Medivietech. La propriété intellectuelle appartient à Medivietech.

- ✅ **Autorisé** : Consultation, référence pour évaluation recrutement/freelance
- ❌ **Non autorisé** : Utilisation commerciale, reproduction de code, produits dérivés

Pour toute demande commerciale concernant la technologie Medivietech, merci de contacter : [contact@medivietech.com](mailto:contact@medivietech.com)

---

## 👤 À Propos

**Tom HUYGHE**  
*Ingénieur Mécatronique | Spécialiste Systèmes Embarqués | Innovateur MedTech*

🎓 **ESME SUDRIA** - Diplôme d'Ingénieur (Mécatronique & Systèmes Embarqués)  
🏢 **Freelance** : Disponible pour projets hardware/software embarqués  
💼 **Ouvert à** : Opportunités CDI dans la MedTech, IoT, Wearables

### 📬 Contact

- 📧 Email : [Votre Email]
- 💼 LinkedIn : [Votre LinkedIn]
- 🌐 Portfolio : [Votre Site Web]
- 🏢 Micro-Entreprise (SIRET : 99486587100010)
- 💰 TJM Freelance : 350€/jour

### 💡 Expertise

- Firmware embarqué (ESP32, Nordic, STM32)
- Conception & prototypage PCB
- Traitement du signal (PPG, IMU)
- Développement de dispositifs médicaux
- Architectures IoT (WiFi, BLE, Cloud)
- Outillage Python & automatisation

---

## ⭐ Remerciements

Remerciements particuliers à :
- **Neil Benhamou** (CEO, Medivietech) - Pour la confiance et l'autonomie technique accordées
- **Thomas Baret** (CTO, Medivietech) - Pour la collaboration IA/Data
- **Dr. Lee** (Clinique Hartmann) - Pour le support validation clinique
- **AGORANOV** - Pour l'environnement incubateur inspirant
- **Équipe EMBRILL** - Pour le partenariat industrialisation

---

<p align="center">
  <strong>⚡ Conçu avec passion pour l'innovation MedTech ⚡</strong>
  <br/>
  <em>Transformer des compétences d'ingénierie en dispositifs médicaux améliorant la vie</em>
</p>

---

## 📸 Galerie Supplémentaire

<p align="center">
  <img src="images/prototypes/pulsar-open-housing.jpg" width="400" alt="Vue interne du boîtier PULSAR"/>
  <img src="images/prototypes/pulsar-sensor-extrusion.jpg" width="400" alt="Détail capteur PPG déporté"/>
  <br/>
  <em>Gauche : Intégration électronique interne | Droite : Détail nappe FPC et capteur déporté</em>
</p>

---

**Dernière mise à jour** : Janvier 2025  
**Version du projet** : Prototypes ESP32-S3 validés cliniquement | Migration Nordic nRF5340 en cours
