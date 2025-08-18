/*
  Dual-IMU + FRS + EMG logger (Teensy)
  ------------------------------------------------------------
  - Initializes two ICM-20948 IMUs on separate I²C buses
  - Configures each IMU’s Digital Motion Processor (DMP) to output
    6-axis game-rotation quaternions (Quat6)
  - Converts quaternions -> Euler angles (Roll, Pitch, Yaw) for each IMU
  - Samples a flexible resistive sensor (FRS) on A14 and an EMG envelope on A8
  - Streams tab-separated values over Serial (115200)
  - Logs to /data.csv on the on-board microSD when commanded
      's' -> start logging   |   'e' -> stop logging
  - Handles occasional NaNs by reusing the last valid Euler angles
*/

#include "ICM_20948.h"  // SparkFun ICM-20948 library
#include <SD.h>         // microSD (Teensy built-in reader)
#include <SPI.h>        // Needed by SD

// ----------------------- Serial & I²C aliases -----------------------
#define SERIAL_PORT Serial   // USB serial at 115200
#define WIRE_PORT   Wire     // I²C bus 0  (for IMU #1)
#define WIRE_PORT2  Wire1    // I²C bus 1  (for IMU #2)

// AD0 pin level selector used by the ICM-20948 library: 
// AD0=0 -> address 0x68, AD0=1 -> address 0x69
#define AD0_VAL 1            // (kept for reference; not directly used below)

// ----------------------- Global objects/state -----------------------
ICM_20948_I2C myICM;         // IMU #1 (on WIRE_PORT)
ICM_20948_I2C myICM2;        // IMU #2 (on WIRE_PORT2)
File dataFile;               // SD file handle
bool logging = false;        // logging state flag

// Analog inputs
int muscleSensorPin2 = A8;   // EMG envelope (Myoware 2.0)
                             // FRS is read from A14 further below

// ===================================================================
// setup(): one-time hardware init
// ===================================================================
void setup() {
  // ----------------------- Serial -----------------------
  SERIAL_PORT.begin(115200);
  while (!SERIAL_PORT) { /* wait for USB serial */ }

  // ----------------------- microSD -----------------------
  // Initialize the built-in SD card interface on Teensy
  if (!SD.begin(BUILTIN_SDCARD)) {
    // If SD fails, park here (prevents later writes from hanging)
    while (1) { /* SD init failed */ }
  }

  // Create CSV file with a header (append if it already exists)
  dataFile = SD.open("data.csv", FILE_WRITE);
  if (dataFile) {
    dataFile.println("TimeStamp,Roll1,Pitch1,Yaw1,Roll2,Pitch2,Yaw2,FRS,EMG");
    dataFile.close();
  }

  // ----------------------- IMU #1 (I²C0) -----------------------
  WIRE_PORT.begin();
  WIRE_PORT.setClock(400000);  // 400 kHz fast-mode I²C

  // Robust bring-up loop until device responds
  bool initialized1 = false;
  while (!initialized1) {
    // begin(Wire, ad0_val): ad0_val = 1 -> 0x69, 0 -> 0x68
    myICM.begin(WIRE_PORT, 1);
    if (myICM.status == ICM_20948_Stat_Ok) {
      initialized1 = true;
    } else {
      delay(500);
    }
  }

  // Configure the DMP on IMU #1 to output the 6-axis quaternion stream
  bool success = true;
  success &= (myICM.initializeDMP() == ICM_20948_Stat_Ok);
  success &= (myICM.enableDMPSensor(INV_ICM20948_SENSOR_GAME_ROTATION_VECTOR) == ICM_20948_Stat_Ok);
  success &= (myICM.setDMPODRrate(DMP_ODR_Reg_Quat6, 0) == ICM_20948_Stat_Ok); // fastest ODR
  success &= (myICM.enableFIFO() == ICM_20948_Stat_Ok);
  success &= (myICM.enableDMP() == ICM_20948_Stat_Ok);
  success &= (myICM.resetDMP() == ICM_20948_Stat_Ok);
  success &= (myICM.resetFIFO() == ICM_20948_Stat_Ok);

  // ----------------------- IMU #2 (I²C1) -----------------------
  WIRE_PORT2.begin();
  WIRE_PORT2.setClock(400000);

  bool initialized2 = false;
  while (!initialized2) {
    // NOTE: Pass '0' here if the second IMU’s AD0=0 (address 0x68).
    // This code uses '1'; ensure the hardware/address matches.
    myICM2.begin(WIRE_PORT2, 1);  // Comment says 0x68; verify AD0 wiring!
    if (myICM2.status == ICM_20948_Stat_Ok) {
      initialized2 = true;
    } else {
      delay(500);
    }
  }

  // Same DMP configuration for IMU #2
  success &= (myICM2.initializeDMP() == ICM_20948_Stat_Ok);
  success &= (myICM2.enableDMPSensor(INV_ICM20948_SENSOR_GAME_ROTATION_VECTOR) == ICM_20948_Stat_Ok);
  success &= (myICM2.setDMPODRrate(DMP_ODR_Reg_Quat6, 0) == ICM_20948_Stat_Ok);
  success &= (myICM2.enableFIFO() == ICM_20948_Stat_Ok);
  success &= (myICM2.enableDMP() == ICM_20948_Stat_Ok);
  success &= (myICM2.resetDMP() == ICM_20948_Stat_Ok);
  success &= (myICM2.resetFIFO() == ICM_20948_Stat_Ok);
}

// ===================================================================
// loop(): real-time read/convert/stream, optional log to SD
// ===================================================================
void loop() {
  // ----------------------- Serial control -----------------------
  // Start/stop logging by typing 's' or 'e' in the Serial Monitor.
  if (SERIAL_PORT.available()) {
    char command = SERIAL_PORT.read();

    if (command == 's') {                // Start logging
      if (!logging) {
        logging = true;
        SERIAL_PORT.println("Logging started.");
        // Optional delimiter between sessions
        dataFile = SD.open("data.csv", FILE_WRITE);
        dataFile.println("new");
        dataFile.close();
      }
    }
    else if (command == 'e') {           // End logging
      if (logging) {
        logging = false;
        SERIAL_PORT.println("Logging stopped.");
      }
    }
  }

  // Backup storage for last valid Euler angles (to avoid NaN gaps)
  static double last_r1 = 0, last_p1 = 0, last_y1 = 0;
  static double last_r2 = 0, last_p2 = 0, last_y2 = 0;

  // Containers for DMP packets & computed angles
  icm_20948_DMP_data_t data;   double r1 = NAN, p1 = NAN, y1 = NAN;
  icm_20948_DMP_data_t data2;  double r2 = NAN, p2 = NAN, y2 = NAN;

  // Timestamp for the CSV (ms since boot)
  unsigned long timestamp = millis();

  // ----------------------- IMU #1: read FIFO -> Euler -----------------------
  myICM.readDMPdataFromFIFO(&data);
  if ((myICM.status == ICM_20948_Stat_Ok || myICM.status == ICM_20948_Stat_FIFOMoreDataAvail) &&
      (data.header & DMP_header_bitmap_Quat6)) {

    // Library outputs fixed-point Quat6 terms; convert to float in [-1,1]
    double q1 = ((double)data.Quat6.Data.Q1) / 1073741824.0;
    double q2 = ((double)data.Quat6.Data.Q2) / 1073741824.0;
    double q3 = ((double)data.Quat6.Data.Q3) / 1073741824.0;
    // Reconstruct q0 (positive root) to form a unit quaternion
    double q0 = sqrt(1.0 - q1 * q1 - q2 * q2 - q3 * q3);

    // Standard quaternion -> Euler (XYZ / roll-pitch-yaw) in degrees
    r1 = atan2(2.0 * (q0 * q1 + q2 * q3), 1.0 - 2.0 * (q1 * q1 + q2 * q2)) * 180.0 / PI;
    p1 = asin (2.0 * (q0 * q2 - q3 * q1))                               * 180.0 / PI;
    y1 = atan2(2.0 * (q0 * q3 + q1 * q2), 1.0 - 2.0 * (q2 * q2 + q3 * q3)) * 180.0 / PI;
  }

  // ----------------------- IMU #2: read FIFO -> Euler -----------------------
  myICM2.readDMPdataFromFIFO(&data2);
  if ((myICM2.status == ICM_20948_Stat_Ok || myICM2.status == ICM_20948_Stat_FIFOMoreDataAvail) &&
      (data2.header & DMP_header_bitmap_Quat6)) {

    double q1 = ((double)data2.Quat6.Data.Q1) / 1073741824.0;
    double q2 = ((double)data2.Quat6.Data.Q2) / 1073741824.0;
    double q3 = ((double)data2.Quat6.Data.Q3) / 1073741824.0;
    // Guard against tiny negative due to numerical error
    double q0 = sqrt(fmax(0.0, 1.0 - (q1 * q1 + q2 * q2 + q3 * q3)));

    r2 = atan2(2.0 * (q0 * q1 + q2 * q3), 1.0 - 2.0 * (q1 * q1 + q2 * q2)) * 180.0 / PI;
    p2 = asin (2.0 * (q0 * q2 - q3 * q1))                               * 180.0 / PI;
    y2 = atan2(2.0 * (q0 * q3 + q1 * q2), 1.0 - 2.0 * (q2 * q2 + q3 * q3)) * 180.0 / PI;
  }

  // ----------------------- NaN handling (hold last good) -----------------------
  if (!isnan(r1)) last_r1 = r1; else r1 = last_r1;
  if (!isnan(p1)) last_p1 = p1; else p1 = last_p1;
  if (!isnan(y1)) last_y1 = y1; else y1 = last_y1;

  if (!isnan(r2)) last_r2 = r2; else r2 = last_r2;
  if (!isnan(p2)) last_p2 = p2; else p2 = last_p2;
  if (!isnan(y2)) last_y2 = y2; else y2 = last_y2;

  // ----------------------- Analog sensors -----------------------
  // FRS on A14: convert raw ADC to volts.
  // NOTE: Teensy 4.x default ADC reference is 3.3V and 10-bit (0..1023).
  // If using 3.3V ref, replace 5.0 with 3.3. Keep as-is if you intentionally scaled to 5V.
  int   rawValue = analogRead(A14);
  float voltage  = rawValue * (5.0 / 1023.0);

  // EMG envelope (0–3.3V typical) on A8
  int muscle2 = analogRead(muscleSensorPin2);

  // ----------------------- Serial stream (tab-separated) -----------------------
  SERIAL_PORT.print(r1, 2); SERIAL_PORT.print("\t");
  SERIAL_PORT.print(p1, 2); SERIAL_PORT.print("\t");
  SERIAL_PORT.print(y1, 2); SERIAL_PORT.print("\t");
  SERIAL_PORT.print(r2, 2); SERIAL_PORT.print("\t");
  SERIAL_PORT.print(p2, 2); SERIAL_PORT.print("\t");
  SERIAL_PORT.print(y2, 2); SERIAL_PORT.print("\t");
  SERIAL_PORT.print(voltage, 3); SERIAL_PORT.print("\t");
  SERIAL_PORT.println(muscle2);

  // ----------------------- Optional CSV logging -----------------------
  if (logging) {
    dataFile = SD.open("data.csv", FILE_WRITE);
    if (dataFile) {
      dataFile.print(timestamp); dataFile.print(",");
      dataFile.print(r1, 2);     dataFile.print(",");
      dataFile.print(p1, 2);     dataFile.print(",");
      dataFile.print(y1, 2);     dataFile.print(",");
      dataFile.print(r2, 2);     dataFile.print(",");
      dataFile.print(p2, 2);     dataFile.print(",");
      dataFile.print(y2, 2);     dataFile.print(",");
      dataFile.print(voltage, 3);dataFile.print(",");
      dataFile.println(muscle2);
      dataFile.close();
    } else {
      SERIAL_PORT.println("Error opening data.csv");
    }
  }

  // ~200 Hz loop target (5 ms)
  delay(5);
}
