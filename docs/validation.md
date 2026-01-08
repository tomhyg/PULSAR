# PULSAR - Clinical Validation Results

This document summarizes the clinical validation protocol and results for the PULSAR physiological monitoring smartwatch.

---

## 🏥 Validation Overview

**Location**: Clinique Hartmann, Paris, France  
**Duration**: 6 months (Prototype development + Clinical trials)  
**Participants**: 50+ patients  
**Total Recording Hours**: 400+ hours  
**Supervising Physician**: Medical team at Clinique Hartmann  
**Regulatory Context**: Pre-certification validation for CE medical device marking

---

## 🎯 Validation Objectives

### Primary Objectives
1. **Functional Reliability**: Verify device operates continuously without crashes
2. **Data Integrity**: Confirm zero data loss during physiological monitoring
3. **Clinical Usability**: Assess comfort, wearability, and user acceptance
4. **Power Autonomy**: Validate 8+ hour battery life in real clinical use

### Secondary Objectives
5. **Signal Quality**: Evaluate PPG and accelerometer data quality
6. **Connectivity Robustness**: Test WiFi/Cloud streaming stability
7. **Clinical Workflow Integration**: Assess compatibility with hospital procedures

---

## 👥 Patient Demographics

| Parameter | Value |
|-----------|-------|
| **Total participants** | 52 patients |
| **Age range** | 18-75 years |
| **Gender distribution** | 60% female, 40% male |
| **Clinical conditions** | Post-operative monitoring, cardiac rehabilitation |
| **Monitoring duration** | 4-12 hours per patient |

---

## 📋 Validation Protocol

### Phase 1: Laboratory Testing (Internal)
**Before clinical trials, internal validation:**
- ✅ Component testing (sensors, PCB, firmware)
- ✅ Environmental testing (temperature, humidity)
- ✅ Accelerated life testing (battery cycles)
- ✅ EMC/EMI compliance preliminary checks

### Phase 2: Clinical Pilot (5 patients)
**Initial real-world testing:**
- Small cohort (5 patients)
- Continuous monitoring (6-8 hours)
- Direct medical staff supervision
- Identify usability issues
- **Result**: Minor firmware adjustments, mechanical design validated

### Phase 3: Extended Clinical Validation (47 patients)
**Full-scale validation:**
- 47 patients across multiple wards
- Standard clinical monitoring protocol
- Data comparison with reference devices
- User experience surveys (patients + medical staff)

---

## 📊 Technical Performance Results

### Data Acquisition Reliability

| Metric | Target | Achieved | Status |
|--------|--------|----------|--------|
| **Uptime (device operational)** | ≥95% | 99.2% | ✅ |
| **Data loss rate** | <1% | 0.08% | ✅ |
| **Sample accuracy** | ±2% | ±1.5% | ✅ |
| **Timestamp precision** | ±10ms | ±5ms | ✅ |

### Connectivity Performance

| Metric | Result |
|--------|--------|
| **WiFi connection success rate** | 98.5% (402/408 sessions) |
| **Average reconnection time** | 2.8 seconds |
| **Cloud data transmission success** | 99.7% |
| **BLE pairing success** | 100% (52/52) |

### Power & Autonomy

| Metric | Target | Achieved | Status |
|--------|--------|----------|--------|
| **Battery life (active)** | ≥8 hours | 8-10 hours | ✅ |
| **Average current draw** | <150mA | 120mA | ✅ |
| **Battery consistency** | ±10% | ±5% | ✅ |

**Notes**:
- Autonomy tested across full 8-hour shifts
- Variation due to WiFi signal strength and patient activity level
- 0 premature battery depletion incidents

---

## 💡 Signal Quality Assessment

### PPG Signal Quality

**Evaluation Method**: Medical staff reviewed PPG traces for artifacts, noise, baseline drift

| Quality Level | Percentage of Recordings |
|---------------|--------------------------|
| **Excellent** (clinical-grade) | 78% |
| **Good** (usable, minor artifacts) | 18% |
| **Poor** (excessive noise) | 4% |

**Poor quality causes**:
- Patient motion during procedure
- Incorrect wristband tension
- Very dark skin tones (known PPG limitation)

### Accelerometer Data Quality

| Metric | Result |
|--------|--------|
| **Motion detection accuracy** | 96% (validated against video recordings) |
| **False positive rate** | <5% |
| **False negative rate** | <2% |

---

## 👨‍⚕️ Clinical Usability Assessment

### Medical Staff Feedback (n=12 nurses/doctors)

| Aspect | Rating (1-5) | Comments |
|--------|--------------|----------|
| **Ease of installation** | 4.2 | "Quick to set up on patients" |
| **Data visualization** | 4.5 | "Real-time dashboard is helpful" |
| **Device size/weight** | 4.0 | "Acceptable for short-term use" |
| **Reliability** | 4.8 | "No device failures during shifts" |
| **Overall satisfaction** | 4.4 | - |

**Common positive feedback**:
- "Much better than wired systems"
- "Allows patient mobility during recovery"
- "Real-time alerts are valuable"

**Areas for improvement**:
- Longer battery life desired (3-5 days for extended monitoring)
- Smaller form factor for overnight comfort
- More robust wristband attachment

### Patient Feedback (n=52)

| Aspect | Rating (1-5) |
|--------|--------------|
| **Comfort** | 3.8 |
| **Perceived intrusiveness** | 2.1 (lower is better) |
| **Willingness to wear again** | 4.6 |

**Patient comments**:
- 😊 "Forgot I was wearing it most of the time"
- 😊 "More comfortable than traditional monitors"
- 😐 "A bit heavy after several hours"
- 😐 "Wristband left slight marks after 8h"

---

## ⚠️ Adverse Events & Incidents

### Safety Record

| Category | Count | Severity |
|----------|-------|----------|
| **Device malfunctions** | 0 | N/A |
| **Data security breaches** | 0 | N/A |
| **Patient injuries** | 0 | N/A |
| **Skin irritation** | 2 | Minor |

**Skin irritation cases**:
- Patient #23: Mild redness after 10h wear (pre-existing sensitive skin)
- Patient #41: Slight pressure mark from tight wristband (user error)
- **Actions taken**: Wristband tension training, hypoallergenic material recommendation

---

## 📈 Statistical Summary

### Session Success Metrics

```
Total Sessions:           52
Successful Completions:   51 (98.1%)
Partial Failures:         1 (1.9%)
Complete Failures:        0 (0%)
```

**Partial failure case** (Patient #17):
- Device powered off after 6h due to user accidentally pressing button
- Data successfully recovered from SD card (no loss)
- Classified as "user error," not device failure

### Data Volume Collected

```
Total PPG samples:        144 million (400h × 100Hz × 4 channels)
Total Accel samples:      72 million (400h × 50Hz × 3 axes)
Total data transmitted:   ~2.8 GB (cloud storage)
Total SD recordings:      ~3.1 GB (local backup)
```

---

## 🎯 Validation Conclusions

### Primary Objectives - Status

| Objective | Status | Evidence |
|-----------|--------|----------|
| **Functional Reliability** | ✅ PASS | 99.2% uptime, 0 crashes |
| **Data Integrity** | ✅ PASS | <0.1% data loss |
| **Clinical Usability** | ✅ PASS | 4.4/5 staff satisfaction |
| **Power Autonomy** | ✅ PASS | 8-10h validated |

### Key Findings

**Strengths**:
- ✅ Robust data acquisition (medical-grade reliability)
- ✅ Excellent connectivity (WiFi/BLE stability)
- ✅ User-friendly interface (minimal training required)
- ✅ Safe for patient use (0 serious incidents)

**Areas Validated for Industrialization**:
- Architecture validated for production scaling
- Component selection appropriate (MAX86916, ESP32-S3)
- Firmware stability confirmed over 400+ hours
- Clinical workflow integration successful

**Recommendations for Next Generation** (Nordic nRF5340):
- Increase battery capacity (target: 3-5 days autonomy)
- Reduce form factor by 20-30%
- Improve wristband ergonomics
- Add multi-patient management dashboard

---

## 📄 Regulatory Impact

### CE Medical Device Pathway

**Pre-certification validation** (this study):
- ✅ Technical file evidence (performance data)
- ✅ Clinical evaluation report (safety/efficacy)
- ✅ Risk management documentation (ISO 14971)

**Next steps for certification**:
- Transition to Nordic nRF5340 platform
- Extended usability testing (IEC 62366)
- Biocompatibility testing (ISO 10993)
- Electromagnetic compatibility (IEC 60601-1-2)

---

## 🏆 Clinical Impact

**Demonstrated Value**:
- Enables **continuous physiological monitoring** without mobility restriction
- Provides **real-time data** to clinical staff for faster intervention
- Improves **patient comfort** vs. traditional wired systems
- Generates **high-quality data** suitable for medical decision-making

**Clinical Partners Testimonial**:
> *"PULSAR has proven to be a reliable and user-friendly device for post-operative monitoring. The real-time data access and patient mobility are significant advantages over traditional systems."*  
> — Medical Staff, Clinique Hartmann

---

## 📚 Publications & Dissemination

**Internal Report**: Full validation report delivered to Medivietech (100+ pages)  
**Thesis**: Results integrated into engineering thesis defense (ESME SUDRIA, 2024)  
**Conference**: Presented at internal MedTech showcase (AGORANOV, Paris)

---

## 🔬 Future Work

Based on validation results, next steps include:

1. **Platform Migration**: Transition to Nordic nRF5340 for extended autonomy
2. **Long-term Studies**: Test 3-5 day continuous monitoring
3. **Algorithm Development**: Advanced PPG processing for clinical parameters
4. **Multi-site Validation**: Expand to additional hospitals
5. **Regulatory Submission**: Complete CE marking process

---

*Validation conducted from January to June 2024 at Clinique Hartmann, Paris.*  
*Supervised by medical professionals and approved by hospital ethics committee.*
