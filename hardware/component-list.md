# PULSAR - Hardware Component List

Bill of Materials (BOM) for the PULSAR prototype smartwatch. This list includes key components used in the dual-board design.

---

## 🧠 Main Processing Unit

| Component | Part Number | Manufacturer | Quantity | Function | Interface |
|-----------|-------------|--------------|----------|----------|-----------|
| **Microcontroller** | ESP32-S3-WROOM-1-N16R8 | Espressif | 1 | Main MCU, WiFi/BLE | - |
| **Crystal** | 40MHz | Generic | 1 | MCU clock reference | - |

**ESP32-S3 Specifications**:
- Dual-core Xtensa LX7 @ 240MHz
- 512KB SRAM, 16MB Flash, 8MB PSRAM
- Integrated WiFi 802.11 b/g/n
- Integrated Bluetooth 5.0 (BLE)

---

## 📡 Sensors

| Component | Part Number | Manufacturer | Quantity | Function | Interface |
|-----------|-------------|--------------|----------|----------|-----------|
| **PPG Sensor** | MAX86916 | Maxim Integrated (Analog Devices) | 1 | 4-channel PPG, SpO2 | I²C |
| **Accelerometer** | LIS3DH | STMicroelectronics | 1 | 3-axis motion detection | SPI |
| **Temperature** | Integrated in MAX86916 | - | - | Skin temperature | I²C |

**MAX86916 Details**:
- 4 LED channels (Green, Red, IR wavelengths)
- 19-bit ADC resolution
- Internal FIFO (32 samples)
- Sample rate: up to 400Hz

**LIS3DH Details**:
- 3-axis accelerometer
- ±2g/±4g/±8g/±16g selectable range
- 10-bit ADC
- Low-power modes (down to 2μA)

---

## 💾 Storage & Memory

| Component | Part Number | Manufacturer | Quantity | Function | Interface |
|-----------|-------------|--------------|----------|----------|-----------|
| **microSD Slot** | Standard microSD socket | Generic | 1 | Local data storage | SPI |
| **microSD Card** | 8GB Class 10 | SanDisk | 1 | FAT32 formatted | - |

**Storage Specifications**:
- Format: FAT32
- Max capacity tested: 32GB
- Write speed: ~1 MB/s (Class 10)

---

## 🔋 Power Management

| Component | Part Number | Manufacturer | Quantity | Function | Interface |
|-----------|-------------|--------------|----------|----------|-----------|
| **Battery** | Li-Po 3.7V 500mAh | Generic | 1 | Power supply | - |
| **Fuel Gauge** | MAX17048 | Maxim Integrated | 1 | Battery monitoring | I²C |
| **Charger IC** | MCP73831 | Microchip | 1 | Li-Po charging | - |
| **LDO Regulator** | AMS1117-3.3 | AMS | 1 | 3.3V power rail | - |

**Battery Details**:
- Type: Lithium Polymer (Li-Po)
- Voltage: 3.7V nominal (4.2V max, 3.0V min)
- Capacity: 500mAh
- Protection: Built-in PCM (overcharge, over-discharge)

**Fuel Gauge Features**:
- State of Charge (SOC) accuracy: ±1%
- Voltage measurement resolution: 78.125μV
- I²C interface for battery status

---

## 🔌 Connectivity Components

| Component | Part Number | Manufacturer | Quantity | Function |
|-----------|-------------|--------------|----------|----------|
| **WiFi Antenna** | PCB trace antenna | Integrated | 1 | 2.4GHz WiFi |
| **BLE Antenna** | PCB trace antenna | Integrated | 1 | Bluetooth 5.0 |
| **FPC Connector** | 0.5mm pitch, 12-pin | Generic | 2 | Board interconnect |

---

## 🔧 Passive Components

### Capacitors
| Value | Voltage | Quantity | Locations |
|-------|---------|----------|-----------|
| 100nF | 50V | 8 | Decoupling (MCU, sensors) |
| 10μF | 16V | 4 | Power supply filtering |
| 1μF | 16V | 6 | Sensor power rails |
| 22pF | 50V | 2 | Crystal load caps |

### Resistors
| Value | Power | Quantity | Function |
|-------|-------|----------|----------|
| 10kΩ | 1/4W | 6 | Pull-up/down (I²C, SPI) |
| 4.7kΩ | 1/4W | 2 | I²C pull-ups |
| 100Ω | 1/4W | 4 | Current limiting (LEDs) |
| 2kΩ | 1/4W | 2 | Charging current setting |

### Inductors
| Value | Current | Quantity | Function |
|-------|---------|----------|----------|
| 10μH | 1A | 1 | Power supply filtering |

---

## 🖥️ User Interface

| Component | Type | Quantity | Function |
|-----------|------|----------|----------|
| **Status LED** | RGB LED (0805) | 1 | Device status indicator |
| **Power Button** | Tactile switch | 1 | Power on/off, reset |

---

## 🔩 Mechanical Components

| Component | Material | Quantity | Function |
|-----------|----------|----------|----------|
| **Watch Housing** | ABS Plastic | 1 | Enclosure (3D printed prototype) |
| **Wristband** | Silicone | 1 | Strap for wrist attachment |
| **Battery Cover** | ABS Plastic | 1 | Battery compartment cover |
| **FPC Cable** | Flexible PCB | 1 | Main board ↔ Sensor board |

**Mechanical Design**:
- Housing designed in SolidWorks
- Prototypes manufactured via 3D printing (FDM/SLA)
- Future production: Injection molding

---

## 📏 PCB Specifications

### Main Board (PCB Top)
- **Layers**: 4-layer PCB
- **Dimensions**: 45mm × 35mm
- **Thickness**: 1.6mm
- **Components**: ESP32-S3, SD slot, battery, power management

### Sensor Board (PCB Bottom)
- **Layers**: 2-layer PCB
- **Dimensions**: 30mm × 20mm
- **Thickness**: 0.8mm (flex-compatible)
- **Components**: MAX86916, LIS3DH, temperature sensor

**Manufacturing**:
- PCB fabrication: JLCPCB
- Assembly: Manual for prototypes (15 units)
- Future: Automated SMT assembly for production

---

## 💰 Estimated Costs (Prototype Quantities)

| Category | Cost per Unit (€) | Notes |
|----------|-------------------|-------|
| **ESP32-S3 Module** | 8.50 | Single unit price |
| **MAX86916 Sensor** | 12.00 | Medical-grade sensor |
| **LIS3DH Accel** | 2.50 | Low-cost MEMS |
| **Battery** | 3.00 | 500mAh Li-Po |
| **PCBs (2 boards)** | 15.00 | Small batch (15 units) |
| **Passives** | 5.00 | Caps, resistors, etc. |
| **Mechanical** | 8.00 | 3D printed housing |
| **Assembly labor** | 20.00 | Manual assembly |
| **TOTAL (prototype)** | **~75€** | Per device |

**Production Estimates** (1000+ units):
- Component costs: ~40€/unit
- PCB + Assembly: ~15€/unit
- Mechanical: ~10€/unit
- **Total production**: ~65€/unit (estimate)

---

## 🔧 Tools & Equipment Used

### Design Tools
- **PCB Design**: Altium Designer
- **Mechanical Design**: SolidWorks
- **Firmware IDE**: ESP-IDF (VSCode + PlatformIO)

### Fabrication Equipment
- **3D Printer**: Ultimaker S5 (FDM), Formlabs Form 3 (SLA)
- **Soldering**: Hot air station, reflow oven
- **Testing**: Oscilloscope, logic analyzer, multimeter

---

## 📦 Suppliers

| Component Type | Supplier | Location |
|----------------|----------|----------|
| **ICs (ESP32, sensors)** | Mouser, DigiKey | International |
| **PCB Fabrication** | JLCPCB | China |
| **Battery** | AliExpress | China |
| **Mechanical parts** | 3D printing in-house | Paris |
| **Passives** | Farnell | France |

---

## 🔄 Design Iterations

### Version History
- **v1.0** (3 units): Initial proof-of-concept, ESP32-WROOM-32 (failed WiFi stability)
- **v2.0** (5 units): Switched to ESP32-S3, added MAX86916 PPG
- **v3.0** (7 units): Dual-board design with FPC, improved power management
- **v3.1** (15 units): Final prototype validated clinically

---

## 🚀 Future Component Changes (Nordic nRF5340 Version)

For production/industrialization, key component changes planned:

| Component | Current (ESP32) | Future (Nordic) |
|-----------|-----------------|-----------------|
| **MCU** | ESP32-S3 | Nordic nRF5340 |
| **Battery** | 500mAh | 1000mAh (extended autonomy) |
| **Housing** | 3D printed ABS | Injection-molded PC/ABS |
| **PCB layers** | 4-layer | 6-layer (better EMC) |

**Rationale for Nordic nRF5340**:
- Lower power consumption (3-5 day autonomy)
- Enhanced BLE 5.2 features
- Medical device pedigree (used in certified devices)
- Better RF performance

---

*Component list valid for PULSAR v3.1 prototype (15 units manufactured).*
