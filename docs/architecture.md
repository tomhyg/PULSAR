# PULSAR - System Architecture

## 📐 Overview

PULSAR is built around a dual-board architecture connecting a main processing unit with a dedicated sensor module via FPC (Flexible Printed Circuit) connector. This design separates computational logic from sensitive analog signal acquisition.

---

## 🔌 Hardware Architecture

### Block Diagram

```
┌─────────────────────────────────────────────────────────┐
│                    MAIN BOARD (PCB Top)                  │
│                                                           │
│  ┌──────────────┐    ┌────────────┐    ┌─────────────┐ │
│  │   ESP32-S3   │───▶│  SD Card   │    │   Battery   │ │
│  │  (240 MHz)   │    │  (FAT32)   │    │   (Li-Po)   │ │
│  └───────┬──────┘    └────────────┘    └──────┬──────┘ │
│          │                                      │         │
│          │ I²C/SPI                     ┌────────▼──────┐ │
│          │                             │  Fuel Gauge   │ │
│          │                             │   (Battery)   │ │
│          │                             └───────────────┘ │
│          │                                                │
│          │ FPC Connector                                  │
└──────────┼────────────────────────────────────────────────┘
           │
           │ (Flexible Cable)
           │
┌──────────▼────────────────────────────────────────────────┐
│               SENSOR BOARD (PCB Bottom)                    │
│                                                             │
│  ┌──────────────┐    ┌─────────────┐    ┌──────────────┐ │
│  │  MAX86916    │    │   LIS3DH    │    │  Temperature │ │
│  │  PPG Sensor  │    │ Accelero-   │    │    Sensor    │ │
│  │  (4 channels)│    │   meter     │    │              │ │
│  └──────────────┘    └─────────────┘    └──────────────┘ │
│                                                             │
│                   (Contact with skin)                       │
└─────────────────────────────────────────────────────────────┘
```

### Component Breakdown

| Component | Function | Interface | Key Specs |
|-----------|----------|-----------|-----------|
| **ESP32-S3** | Main MCU | - | Dual-core 240MHz, 512KB SRAM |
| **MAX86916** | PPG acquisition | I²C | 4 optical channels, 100Hz sampling |
| **LIS3DH** | Motion detection | SPI | 3-axis, ±2g to ±16g range |
| **SD Card** | Local storage | SPI | FAT32, up to 32GB |
| **Fuel Gauge** | Battery monitoring | I²C | ±1% accuracy |
| **Li-Po Battery** | Power supply | - | 3.7V, 500mAh capacity |

---

## 💾 Data Flow Architecture

### Dual-Mode Operation

PULSAR operates in two simultaneous modes:

```
┌─────────────────────────────────────────────────────────┐
│                     DATA SOURCES                         │
│                                                          │
│   PPG (100Hz)    Accel (50Hz)    Temp (1Hz)            │
└────────┬─────────────┬───────────────┬──────────────────┘
         │             │               │
         ▼             ▼               ▼
    ┌────────────────────────────────────┐
    │      ESP32-S3 Data Processing      │
    │   (Filtering, Buffering, Format)   │
    └─────────────┬──────────────────────┘
                  │
         ┌────────┴────────┐
         │                 │
         ▼                 ▼
    ┌────────┐      ┌──────────┐
    │  WiFi  │      │ SD Card  │
    │  MQTT  │      │  Local   │
    │  AWS   │      │ Storage  │
    └────────┘      └──────────┘
    
    REAL-TIME        BACKUP
    STREAMING        RECORDING
```

**Mode 1: Real-Time Streaming** (WiFi available)
- Data transmitted via MQTT to AWS IoT Core
- Low-latency monitoring on clinical dashboard
- Automatic reconnection on network loss

**Mode 2: Standalone Recording** (WiFi unavailable)
- Continuous recording to SD card (FAT32)
- Timestamped files for post-processing
- Automatic sync when WiFi restored

---

## 🔄 Firmware Architecture

### Task Structure (FreeRTOS)

```
┌─────────────────────────────────────────┐
│          FreeRTOS Scheduler             │
└─────────────────┬───────────────────────┘
                  │
        ┌─────────┼─────────┬─────────┐
        │         │         │         │
        ▼         ▼         ▼         ▼
   ┌────────┐ ┌──────┐ ┌──────┐ ┌──────┐
   │ Sensor │ │ WiFi │ │  SD  │ │ BLE  │
   │  Task  │ │ Task │ │ Task │ │ Task │
   └────────┘ └──────┘ └──────┘ └──────┘
   (Priority: (Pri: 2) (Pri: 1) (Pri: 0)
      High)
      
   • PPG/Accel    • MQTT      • File    • Control
     reading        publish     write    • Status
   • Buffering    • Recon.    • Flush   • OTA
```

**Priority Model**:
- **High**: Sensor acquisition (cannot miss samples)
- **Medium**: Network communication (real-time but buffered)
- **Low**: SD write (asynchronous, buffered)
- **Idle**: BLE control interface (user interaction)

---

## 📡 Communication Protocols

### WiFi/MQTT Pipeline

```
ESP32-S3 ──▶ WiFi ──▶ MQTT ──▶ AWS IoT Core ──▶ Clinical Dashboard
         (WPA2)    (TLS 1.2)    (Lambda)         (Web interface)
```

**Security**:
- TLS 1.2 encryption for all transmissions
- Device-specific certificates (AWS X.509)
- No hardcoded credentials (secure boot)

### BLE Control Interface

```
Mobile App (Flutter) ──▶ BLE ──▶ ESP32-S3
                       (GATT)
                       
Commands:
• Start/Stop recording
• Battery status
• WiFi configuration
• Firmware update (OTA)
```

---

## ⚡ Power Management

### Power States

| State | Current Draw | When Active |
|-------|--------------|-------------|
| **Active Streaming** | ~120 mA | WiFi ON, sensors sampling |
| **Recording Only** | ~60 mA | WiFi OFF, SD write only |
| **BLE Idle** | ~15 mA | Standby, awaiting commands |
| **Deep Sleep** | <1 mA | User-initiated pause |

### Battery Life Calculation

```
Battery: 500 mAh @ 3.7V
Active streaming: 120 mA
→ Autonomy = 500/120 ≈ 4.2h (theoretical)
→ Real-world (with power modes): 8-10h
```

**Optimization Techniques**:
- Dynamic WiFi sleep (DTIM beacon skipping)
- Sensor duty cycling (power down between samples)
- SD card power gating (flush then disable)
- CPU frequency scaling (80MHz when idle)

---

## 🔐 Data Integrity

### Redundancy Strategy

1. **Primary Path**: Real-time WiFi/AWS streaming
2. **Backup Path**: Continuous SD card recording
3. **Integrity Checks**: CRC-32 on all data packets

**Recovery Scenarios**:
- WiFi loss → Automatic SD fallback
- SD full → Alert via BLE, continue streaming
- Battery low → Safe shutdown with data flush

---

## 📊 Performance Metrics

| Metric | Value |
|--------|-------|
| **PPG Sampling Rate** | 100 Hz |
| **Accelerometer Rate** | 50 Hz |
| **Data Latency (WiFi)** | <200 ms |
| **SD Write Speed** | ~1 MB/s |
| **Boot Time** | <5 seconds |
| **WiFi Reconnect Time** | <3 seconds |

---

## 🚀 Scalability Considerations

The ESP32-S3 prototype architecture was designed with industrialization in mind:

**Migration to Nordic nRF5340** (Next generation):
- Similar dual-core architecture
- Lower power consumption (3-5 days autonomy)
- Enhanced BLE 5.2 features
- Maintains same sensor interfaces (I²C, SPI)

---

## 📝 Design Rationale

**Why dual-board FPC design?**
- Minimizes noise coupling between digital (MCU) and analog (sensors)
- Allows sensor board to conform to wrist curvature
- Simplifies mechanical assembly

**Why dual-mode operation?**
- Ensures zero data loss in clinical settings
- Provides flexibility for different hospital IT infrastructures
- Enables offline operation during WiFi maintenance

**Why ESP32-S3 for prototype?**
- Rapid development (Arduino/ESP-IDF ecosystem)
- Built-in WiFi/BLE (no external modules)
- Cost-effective for small batch manufacturing

---

*This architecture successfully validated on 50+ patients with 99.2% uptime reliability.*
