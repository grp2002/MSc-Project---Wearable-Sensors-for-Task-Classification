# MSc Project – Wearable Sensors for Task Classification

## 📚 Table of Contents
- [Project Overview](#-project-overview)
- [Objectives](#-objectives)
- [Circuit Evolution](#-circuit-evolution)
- [Software Tools](#-software-tools)
- [GANTT Chart](#-gantt-chart)


## 🧠 Project Overview
This project aims to design and develop a compact, wearable sensor system capable of real-time motion intent detection for integration with a knee exoskeleton. The system leverages multimodal sensing—using IMUs, surface EMG, and flexible strain sensors—to classify gait phases and transition movements such as walking, sit-to-stand, and stand-to-sit, enabling predictive control for assistive robotics.

Developed as part of the MSc ENG5059 at the University of Glasgow (2024–2025), under the supervision of Dr. Lucas Ferrari Gerez.

---

## 🎯 Objectives

- **Sensor Integration**: Develop a wearable suite combining:
  - Inertial Measurement Units (IMUs) for joint kinematics
  - Surface electromyography (sEMG) for muscle activation
  - Flexible resistive strain sensors for knee flexion tracking

- **Real-Time Processing**:
  - Implement filtering and feature extraction pipelines
  - Enable synchronized data acquisition via Teensy 4.1 microcontroller

- **Motion Classification**:
  - Train supervised learning models to identify gait phases and transitions
  - Evaluate classification performance, latency, and fusion accuracy

- **System Evaluation**:
  - Conduct controlled trials
  - Assess wearability, responsiveness, and classification efficacy
---

## ⚡ Circuit Evolution: Original vs Final

The hardware circuit also evolved significantly during the project.  
Below is a comparison between the **original planned circuit** and the **final implemented version**.

### 🔹 Original Circuit Plan
<img width="665" height="364" alt="image" src="https://github.com/user-attachments/assets/d084921b-578a-46bf-83e1-9fc8fd085e78" />

- Used a **single IMU (ICM-20948)** connected via I²C with pull-up resistors.  
- EMG sensing relied on a **Grove EMG detector**.  
- Knee flexion measured with a **resistive flex sensor** in a simple voltage divider.  
- Simpler design, but **limited sensing coverage** and higher susceptibility to EMG noise.  

---

### 🔹 Final Implemented Circuit
<img width="557" height="447" alt="image" src="https://github.com/user-attachments/assets/e8c979e4-d115-4961-9ffe-fadf528431b8" />

- Added **two IMUs (thigh + shank)** for improved relative orientation tracking.  
- Switched to **MyoWare 2.0 EMG** (calf placement) for more stable neuromuscular signals.  
- Flex sensor repositioned and calibrated for knee bending measurement.  
- Improved **power routing and grounding** (including a decoupling capacitor and grounding patch).  
- Teensy 4.1 pin mapping fully clarified (A8 → EMG, A14 → FRS, I²C buses → IMUs).  

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
- **Firmware Development**  
  - Arduino IDE with Teensyduino (C++) for the Teensy 4.1  
  - Handles: sensor initialization (IMUs, EMG, FRS), SD card logging, dual I²C configuration, sampling synchronization  
<img width="1628" height="414" alt="image" src="https://github.com/user-attachments/assets/1cafd3f9-2cb7-4910-a9dc-18506a35ca9a" />

- **Data Processing & Machine Learning**  
  - Python (Anaconda environment) with:  
    - NumPy, SciPy, Pandas for filtering, signal conditioning, and feature extraction  
    - scikit-learn for training and validating the Random Forest classifier  
  - Feature extraction performed with 200 ms sliding windows and standard time-domain descriptors  

- **Real-Time Classification Workflow**  
  - Python script communicating with the Teensy over USB serial  
  - Rolling buffer + feature extraction in real time  
  - Random Forest inference with majority-vote smoothing for stable predictions
 
    <img width="1639" height="856" alt="image" src="https://github.com/user-attachments/assets/a74e2637-6c6f-42f9-ac23-fafee9d1e38e" />


- **Analysis & Visualization**  
  - Jupyter Notebooks for post-processing and evaluation  
  - Matplotlib for plotting results (confusion matrices, feature importance rankings, classification trends)
<img width="618" height="438" alt="image" src="https://github.com/user-attachments/assets/89e48836-5b99-4c96-9c5a-818626d063a1" />
<img width="645" height="398" alt="image" src="https://github.com/user-attachments/assets/991b0082-fa2a-4bbb-ac89-9d0a787b688f" />
<img width="548" height="361" alt="image" src="https://github.com/user-attachments/assets/a3a850e8-15d2-4c9f-9b6e-122f02ca11ac" />


<img width="931" height="899" alt="image" src="https://github.com/user-attachments/assets/d4edda07-a599-4a9f-8072-4d5a52f0b21c" />
---

## 🔄 Original Plan vs Final Implementation

This section highlights the original design intentions versus how the project evolved during development and testing.

### ✅ Original Plan

- **Sensor Configuration**:
  - Use **two EMG channels** (rectus femoris and medial gastrocnemius)
  - Track **heel standing and toe standing** as additional activities

- **System Architecture**:
  - Fully embedded **on-device classification** using the Teensy 4.1
  - Include real-time feedback for closed-loop control simulation

- **Data Strategy**:
  - Focus on offline analysis with pre-recorded trials only
  - Train multiple classifiers (e.g., k-NN, SVM, Random Forest)

---

### 🛠 Final Implementation

- **Simplified EMG Setup**:
  - EMG on rectus femoris was removed due to high noise and redundancy
  - Final system used **a single EMG channel** (medial gastrocnemius) for intent detection

- **Refined Activity Set**:
  - **Removed toe/heel standing** due to minimal kinematic differentiation
  - Final activity set:
    - Standing, Walking, Sitting, Sit-to-Stand, Upstairs, Downstairs

- **System Processing Flow**:
  - Offloaded real-time classification to a **host PC via serial** (Python-based pipeline)
  - Teensy focused on synchronized data acquisition and logging

- **Model Optimization**:
  - Settled on a **Random Forest Classifier** due to its low latency and high robustness
  - Achieved **98% classification accuracy** across all test conditions

- **User-Centric Adjustments**:
  - Modified knee brace with **Velcro and internal routing** for comfort and consistent sensor positioning
  - Added **grounding patch for EMG** signal stability during dynamic movement
 
---
## 📉 GANTT chart
![image](https://github.com/user-attachments/assets/a639579e-bbe8-46da-a7e4-b72322364ded)

---

