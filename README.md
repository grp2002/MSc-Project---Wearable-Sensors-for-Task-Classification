# MSc Project – Wearable Sensors for Task Classification

## 📚 Table of Contents
- [Project Overview](#-project-overview)
- [Objectives](#-objectives)
- [Hardware Components](#-hardware-components)
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
 
    
![image](https://github.com/user-attachments/assets/7f25eb37-445a-4450-a062-eca3d5324180)

---

## 🔧 Hardware Components

| Component                | Description                              |
|-------------------------|------------------------------------------|
| Teensy 4.1              | High-speed MCU for real-time processing  |
| IMU (ICM-20948)         | Captures joint angles and gait events    |
| Grove EMG Detector      | sEMG from quadriceps and hamstrings      |
| Flexible Resistive Sensors | Detect knee joint bending               |
| Elastic Knee Brace & Velcro | Stable sensor attachment              |

---
![image](https://github.com/user-attachments/assets/9265d820-38ee-4826-acce-d92a69dc13e1)
![image](https://github.com/user-attachments/assets/ba808ef0-afb2-44a8-bbe3-c594706213db)


## 💻 Software Tools

- **Firmware**: Arduino IDE (C++ for Teensy)
- **Signal Processing & ML**:
  - Python (Jupyter Notebook)
  - NumPy, SciPy for filtering
  - scikit-learn, PyTorch for classification

---
## 📉 GANTT chart
![image](https://github.com/user-attachments/assets/a639579e-bbe8-46da-a7e4-b72322364ded)

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

