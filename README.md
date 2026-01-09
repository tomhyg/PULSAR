# PULSAR — Medical Wearable for Clinical Monitoring

> **Hardware/Software engineering project | 6 months R&D at Medivietech (MedTech Startup)**  
> **Clinical validation: 50+ patients | 400+ hours of data | 99.2% system uptime**

[![Engineering](https://img.shields.io/badge/Engineering-Mechatronics-blue)](#)
[![Hardware](https://img.shields.io/badge/Hardware-ESP32%20%7C%20Nordic%20nRF-green)](#)
[![Medical](https://img.shields.io/badge/Status-Clinically%20Validated-red)](#)

---

## 📖 Documentation

- **[📐 System Architecture](docs/architecture.md)** — Hardware/software architecture overview
- **[🔧 Technical Challenges](docs/challenges.md)** — Engineering problems solved
- **[🏥 Clinical Validation](docs/validation.md)** — Hospital deployment results (50+ patients)
- **[🔌 Hardware Guide](hardware/README.md)** — Component specifications & PCB design

---

## 📸 Project Showcase

<table>
<tr>
<td width="50%">
<img src="images/prototypes/pulsar-final-renders.jpg" alt="PULSAR 3D Renders"/>
<p align="center"><i>Professional CAD renders of PULSAR prototypes</i></p>
</td>
<td width="50%">
<img src="images/prototypes/pulsar-dual-watches.jpg" alt="Physical Prototypes"/>
<p align="center"><i>Functional prototypes PULSAR-007 & PULSAR-009</i></p>
</td>
</tr>
<tr>
<td width="50%">
<img src="images/prototypes/pulsar-sensor-extrusion.jpg" alt="Deported Sensor Detail"/>
<p align="center"><i>Deported circular sensor via FPC ribbon cable</i></p>
</td>
<td width="50%">
<img src="images/prototypes/pulsar-open-housing.jpg" alt="Internal Electronics"/>
<p align="center"><i>Internal integration: LiPo battery + PCB</i></p>
</td>
</tr>
</table>

---

## 🎯 What is PULSAR?

PULSAR is a **medical-grade wearable device** for continuous physiological monitoring in hospital environments. Unlike consumer wearables (Fitbit, WHOOP), PULSAR achieves **clinical-level precision** validated against professional medical equipment.

**My role:** Complete hardware/software development from legacy prototype debugging to clinical deployment.

### Key Achievements

| Metric | Result |
|--------|--------|
| **Clinical Validation** | 50+ patients @ Clinique Hartmann (Neuilly-sur-Seine) |
| **Data Collected** | 400+ hours of medical-grade recordings |
| **System Reliability** | 99.2% uptime during 3-month deployment |
| **Data Integrity** | 0% data loss (FIFO optimization) |
| **Prototypes Built** | 15+ functional units |
| **Battery Life** | 8-10h (ESP32 prototypes) → 3-5 days target (Nordic production) |

---

## 🛠️ Technical Highlights

### Hardware Engineering
- ✅ **Multi-channel optical sensor integration** (PPG 4-channel, 100 Hz sampling)
- ✅ **Custom PCB design** with deported circular sensor (ergonomic wearability)
- ✅ **Power optimization** (fuel gauge monitoring, LiPo charging circuit)
- ✅ **Mechanical CAD design** for 3D-printed enclosures (15+ iterations)
- ✅ **Sensor fusion** (accelerometer + PPG for motion artifact rejection)

<img src="images/hardware/max86916-sensor.jpg" alt="Deported Sensor PCB" width="400"/>

*Circular deported optical sensor (Ø10mm) connected via flexible FPC ribbon cable*

### Embedded Software Development
- ✅ **Real-time firmware** for dual-core ESP32-S3 (240 MHz)
- ✅ **Zero-loss data pipeline** (resolved critical FIFO overflow bug)
- ✅ **Dual-mode architecture** (WiFi + AWS cloud / Standalone SD card recording)
- ✅ **Protocol implementation** (I2C @ 400kHz, SPI, FAT32 file system)
- ✅ **Memory optimization** (PSRAM management for continuous acquisition)

### Signal Processing
- ✅ **PPG waveform analysis** (multi-spectral: Red, IR, Green, Blue channels)
- ✅ **Data encoding** (JSON chunking + Base64 for cloud upload)
- ✅ **Clinical-grade accuracy** validated against hospital equipment

<table>
<tr>
<td width="50%">
<img src="images/architecture/ppg-principle.png" alt="PPG Sensor Principle"/>
<p align="center"><i>Photoplethysmography (PPG) measurement principle</i></p>
</td>
<td width="50%">
<img src="images/architecture/ppg-waveform.png" alt="PPG Waveform"/>
<p align="center"><i>Typical PPG signal with cardiac features</i></p>
</td>
</tr>
</table>

---

## 🏥 Clinical Deployment

**Location:** Clinique Hartmann (Neuilly-sur-Seine, France)  
**Supervising Physician:** Dr. Lee (Intensive Care Unit)  
**Duration:** 3 months continuous testing  
**Patient Demographics:** 50+ patients (diverse age groups, pathologies)

### Validation Results
- ✅ **Medical precision** validated against professional oximeters
- ✅ **99.2% system uptime** (only 2 units had minor issues over 90 days)
- ✅ **User comfort** confirmed by medical staff and patients
- ✅ **Zero safety incidents** throughout deployment period

---

## 💡 Problem-Solving Experience

### Challenge 1: Critical FIFO Overflow ❌ → ✅
**Inherited Problem:** Original firmware discarded 29 out of 30 samples  
**Root Cause:** Incorrect FIFO read logic + buffer overflow  
**Solution:** Complete redesign of data acquisition pipeline  
**Result:** **0% data loss** on all 15 deployed units

### Challenge 2: SPI Bus Conflicts ❌ → ✅
**Problem:** SD card + Accelerometer sharing SPI bus caused system crashes  
**Solution:** Exclusive SPI access with proper timing guards  
**Result:** Stable 8-10h continuous recording sessions

### Challenge 3: Wearable Ergonomics ❌ → ✅
**Problem:** Square PCB uncomfortable for wrist wear  
**Solution:** Circular deported sensor (Ø10mm) via flexible FPC cable  
**Result:** Positive comfort feedback from 50+ patients

---

## 🔧 Technologies Used

**Microcontrollers:**  
- ESP32-S3 (R&D prototyping: 240 MHz dual-core, 8MB PSRAM)
- Nordic nRF5340 (Production target: ARM Cortex-M33, ultra-low power)

**Sensors & Components:**  
- Multi-spectral optical sensor (PPG 4-channel @ 100 Hz)
- 3-axis accelerometer (motion detection)
- Fuel gauge IC (battery monitoring)
- MicroSD card (FAT32 storage)

**Connectivity:**  
- WiFi 802.11n (AWS S3 upload pipeline)
- Bluetooth 5.0 LE (future mobile app support)

**Development Tools:**  
- **Firmware:** C/C++, Arduino/ESP-IDF, VS Code, Git
- **PCB Design:** Altium , EasyEDA Pro 
- **Mechanical CAD:** SolidWorks, Fusion 360
- **Cloud:** AWS S3, AWS Lambda
- **Analysis:** Python (data extraction, FFT analysis)

<img src="images/architecture/ppg-spectral-analysis.png" alt="FFT Analysis" width="700"/>

*Spectral analysis of 4-channel PPG data (cardiac frequency band highlighted)*

---

## 📂 Repository Structure

```
PULSAR/
├── README.md                      # Portfolio presentation (this file)
├── LICENSE.md                     # Confidentiality notice
├── docs/                          # Technical documentation
│   ├── architecture.md            # System architecture overview
│   ├── challenges.md              # Technical challenges solved
│   └── validation.md              # Clinical validation results
├── hardware/                      # Hardware specifications
│   ├── README.md                  # Hardware guide
│   └── component-list.md          # Bill of Materials
└── images/                        # Visual documentation
    ├── prototypes/                # Physical device photos
    ├── hardware/                  # PCB and component photos
    ├── cad/                       # CAD renders
    └── architecture/              # Technical diagrams
```

**Note:** Hardware schematics, firmware source code, and detailed system architectures remain confidential per Medivietech company policy. Documentation in `/docs/` provides high-level technical context without revealing proprietary implementations.

---

## 🎓 Skills Demonstrated

### Technical Competencies
- **Embedded Systems:** Firmware development, RTOS concepts, memory optimization
- **Hardware Design:** PCB design, sensor integration, power management
- **Signal Processing:** PPG analysis, FFT, artifact rejection algorithms
- **CAD Engineering:** Mechanical design for wearable devices
- **IoT Architecture:** WiFi connectivity, cloud integration (AWS), data pipelines
- **Medical Devices:** Clinical validation methodology, hospital deployment

### Professional Skills
- **R&D Project Management:** Led project from debugging to clinical validation
- **Cross-functional Collaboration:** Worked with medical staff, regulatory teams
- **Technical Documentation:** Created multilingual documentation for international partners
- **Problem-Solving:** Diagnosed and resolved critical system bugs under time pressure
- **Startup Environment:** Operated autonomously with minimal supervision

---

## 📄 License & Confidentiality

This repository showcases my engineering work during my internship at **Medivietech** (AGORANOV Paris).  
Intellectual property belongs to Medivietech.

**✅ Authorized:** Portfolio review for recruitment/freelance evaluation  
**❌ Not Authorized:** Commercial use, code reproduction, derivative products  

For commercial inquiries: [contact@medivietech.com](mailto:contact@medivietech.com)

---

## 👤 About Me

**Tom HUYGHE**  
*Mechatronics Engineer | Embedded Systems Specialist | MedTech Innovator*

🎓 **ESME SUDRIA** — Engineering Degree (Mechatronics & Embedded Systems)  
🏢 **Currently:** Freelance embedded consultant + Open to full-time opportunities  
💼 **Interests:** MedTech, IoT, Hardware, Wearables, Signal Processing  

### 📬 Contact

📧 **Email:** [tom.huyghe@orange.com]  
💼 **LinkedIn:** [linkedin.com/in/tom-huyghe]  
🌐 **Portfolio:** [tomhuyghe.dev]
💰 **Freelance Rate:** 350€/day  

### 💡 Core Expertise
- Embedded firmware (ESP32, Nordic nRF, STM32)
- PCB design & rapid prototyping
- Physiological signal processing (PPG, ECG, IMU)
- Medical device development & validation
- IoT cloud architectures
- Python tooling & data analysis

---

## 🏢 Professional Context

**Company:** [Medivietech](https://www.medivietech.com) (MedTech Startup)  
**Incubator:** AGORANOV Paris (Leading French Deep-Tech Incubator)  
**Role:** Hardware/Software Engineering Intern  
**Duration:** 6 months (April - October 2024)  
**Team:** CEO (Neil Benhamou), CTO (Thomas Baret - AI/Data),
**International Partnership:** EMBRILL (India) for industrialization

---

## 🙏 Acknowledgments

Special thanks to:
- **Neil Benhamou** (CEO, Medivietech) — For entrusting me with technical autonomy
- **Thomas Baret** (CTO, Medivietech) — For AI/Data collaboration
- **Dr. Lee** (Clinique Hartmann) — For clinical validation support
- **AGORANOV** — For the inspiring startup environment
- **EMBRILL Team** (India) — For industrialization partnership

---

**⚡ Engineered with passion for MedTech innovation ⚡**

*Transforming engineering skills into medical devices that improve lives*

---

**Last Updated:** January 2025  
**Project Status:** ✅ Clinical validation completed | ✅ Production migration ongoing (Nordic nRF5340)
