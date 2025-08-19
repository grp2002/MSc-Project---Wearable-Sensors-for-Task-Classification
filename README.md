# MSc Project – Wearable Sensors for Task Classification

## 📚 Table of Contents
- [Project Overview](#-project-overview)
- [Objectives](#-objectives)
- [Original Plan vs Final Implementation](#-original-plan-vs-final-implementation)
- [Hardware Components](#-hardware-components)
- [Software Pipeline](#-software-pipeline)
- [Testing & Results](#-testing--results)
- [Repository Structure](#-repository-structure)

---

## 🧠 Project Overview

This project develops a compact, wearable sensor system capable of **real-time motion intent detection** for integration with a **knee exoskeleton**. The system uses **multimodal sensing**—inertial measurement units (IMUs), surface electromyography (sEMG), and flexible resistive strain sensors (FRS)—to classify **gait phases** and transition movements (e.g., walking, sit-to-stand, stair ascent/descent).  

Developed as part of the **MSc ENG5059 (2024–2025)** at the **University of Glasgow**, supervised by **Dr. Lucas Ferrari Gerez**.  

---

## 🎯 Objectives

- **Sensor Integration**  
  - IMUs for joint kinematics  
  - Surface EMG for muscle activation  
  - Flexible resistive strain sensors for knee flexion tracking  

- **Real-Time Processing**  
  - Filtering and feature extraction pipelines  
  - Synchronized data acquisition via Teensy 4.1  

- **Motion Classification**  
  - Supervised ML models (Random Forest)  
  - Evaluation of classification accuracy, latency, and robustness  

- **System Evaluation**  
  - Controlled user trials  
  - Assessment of **wearability, responsiveness, and intent prediction efficacy**  

---

## 🔄 Original Plan vs Final Implementation

The design evolved significantly during prototyping.  

### 🔹 Original Circuit Plan
<img width="665" height="364" alt="image" src="https://github.com/user-attachments/assets/d084921b-578a-46bf-83e1-9fc8fd085e78" />

- Used a **single IMU (ICM-20948)** connected via I²C with pull-up resistors.  
- EMG sensing relied on a **Grove EMG detector**.  
- Knee flexion measured with a **resistive flex sensor** in a simple voltage divider.  
- Simpler design, but **limited sensing coverage** and higher susceptibility to EMG noise.  
---

### 🔹 Final Implemented Circuit
<img width="557" height="447" alt="image" src="https://github.com/user-attachments/assets/e8c979e4-d115-4961-9ffe-fadf528431b8" />

- **2× IMUs (thigh + shank)** for relative orientation  
- **MyoWare 2.0 EMG** (medial gastrocnemius)  
- Flex sensor repositioned for consistent knee tracking  
- Teensy used for **synchronized data logging**, classification offloaded to PC  
- Improved **grounding and wiring stability**  

---

### 🔍 Key Changes
| Aspect               | Original Plan                                | Final Implementation                        |
|----------------------|-----------------------------------------------|---------------------------------------------|
| IMU Setup            | 1× ICM-20948 (generic placement)             | 2× ICM-20948 (thigh + shank) for better kinematics |
| EMG Sensor           | Grove EMG (rectus femoris)                   | MyoWare 2.0 (medial gastrocnemius)          |
| Flex Sensor          | Basic voltage divider                        | Preloaded, aligned vertically over patella   |
| Teensy Role          | Logging + planned on-device classification   | Logging + synchronized acquisition (classification offloaded to PC) |
| Stability            | Simple wiring, limited grounding             | Improved wiring, added grounding patch + Velcro stabilization |

---

## 💻 Software Tools
### 1. Firmware (Teensy 4.1, Arduino IDE + Teensyduino)
- Dual I²C buses for IMUs  
- ADC pins for EMG (A8) and FRS (A14)  
- SD logging + USB serial streaming (200 Hz)  
- Serial commands:  
  - **‘s’** → start logging  
  - **‘e’** → end logging
 
<img width="931" height="899" alt="image" src="https://github.com/user-attachments/assets/d4edda07-a599-4a9f-8072-4d5a52f0b21c" />
<img width="1628" height="414" alt="image" src="https://github.com/user-attachments/assets/1cafd3f9-2cb7-4910-a9dc-18506a35ca9a" />

---

### 2. Data Processing (Python)
- **Windowing:** 200 samples (1s @ 200 Hz), step 50 (75% overlap)  
- **Features:** mean, std, min, max, RMS  
- **Channels:** Roll1, Pitch1, Yaw1, Roll2, Pitch2, Yaw2, FRS, EMG  
- → **40-D feature vector per window**  

---

### 3. Machine Learning (Offline)
- Classifier: **Random Forest (n_estimators=100)**  
- Train/test split + cross-validation  
- Achieved **~98% classification accuracy**  
- Model + label encoder exported (`rf_model.pkl`, `label_encoder.pkl`) 
 
    <img width="1639" height="856" alt="image" src="https://github.com/user-attachments/assets/a74e2637-6c6f-42f9-ac23-fafee9d1e38e" />


---

### 4. Real-Time Classification (Host PC)
- Teensy streams sensor data via USB serial (115200 baud)  
- Python script buffers **200 samples**, extracts features, runs ML  
- Predictions smoothed with **majority vote over last 5 windows**  
- Output: Live activity label displayed/logged  
---

---

## 🎥 Testing & Results
 
- Initial set of Classifications:Standing, Heel standing, Toe standing, Walking, Sit-to-stand
  - https://github.com/grp2002/MSc-Project---Wearable-Sensors-for-Task-Classification/blob/main/videos/First_Classifications_Test.mp4
  - Heel standing and toe standing were included to evaluate EMG sensitivity over the medial gastrocnemius.

- Revised Classification Set: Standing, Walking, Sitting, Sit-to-stand
  - https://github.com/grp2002/MSc-Project---Wearable-Sensors-for-Task-Classification/blob/main/videos/Second_Classifications_Test.mp4
  - Heel standing and toe standing were removed after results showed minimal contribution to knee kinematic variation.
 
-Final Classification Set: Standing, Walking, Upstairs, Downstairs, Sitting, Sit-to-stand
  - Stair-related activities were added to capture additional lower-limb motion patterns.
- Final classifier achieved:  
  - **98% accuracy (cross-validation)**  
  - Stable real-time prediction with minimal latency
<img width="618" height="438" alt="image" src="https://github.com/user-attachments/assets/89e48836-5b99-4c96-9c5a-818626d063a1" />

<img width="548" height="361" alt="image" src="https://github.com/user-attachments/assets/a3a850e8-15d2-4c9f-9b6e-122f02ca11ac" />



 
---
## 📉 GANTT chart
![image](https://github.com/user-attachments/assets/a639579e-bbe8-46da-a7e4-b72322364ded)

---

