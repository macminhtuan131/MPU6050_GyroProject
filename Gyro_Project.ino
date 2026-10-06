#include <Wire.h> // I2C

#define ACCEL_XOUTH 0x3B
#define GYRO_XH     0x43

const int MPU = 0x68; // MPU6050 I2C address
float GyroX, GyroY, GyroZ;
float roll = 0, pitch = 0, yaw = 0;

float froll = 0, fpitch = 0, fyaw = 0;
float accRollError = 0, accPitchError = 0;

float elapsedTime, currentTime, previousTime;
float dt = 0.015; // Khởi tạo dt mặc định cho vòng lặp đầu tiên
bool turn_fillter = false;

// --- BIẾN LƯU GIÁ TRỊ LỚN NHẤT ---
float maxX = 0.0;
float maxY = 0.0;
float maxZ = 0.0;

void setup() {
  Wire.begin();
  Wire.beginTransmission(MPU);
  Wire.write(0x6B);     
  Wire.write(0x00);
  Wire.endTransmission();
  Serial.begin(9600);
  Serial.println("He thong san sang! Go bat ky phim nao roi nhan Enter de xem thong so.");
}

float displayRoll = 0.0;
float displayPitch = 0.0;
float displayYaw = 0.0;

void loop() {
  if(turn_fillter == false) {
    filter_gyro();
    turn_fillter = true;
    previousTime = millis(); // Chốt mốc thời gian ngay sau khi calibration xong
  }

  // CHUẨN HOÁ TÍNH DELTA TIME (dt)
  currentTime = millis();
  dt = (currentTime - previousTime) / 1000.0;
  previousTime = currentTime; 

  // 1. ĐỌC DỮ LIỆU GIA TỐC KẾ (ACCELEROMETER)
  Wire.beginTransmission(MPU);
  Wire.write(ACCEL_XOUTH); 
  Wire.endTransmission(false);
  Wire.requestFrom(MPU, 6, true);
  float AccX = (Wire.read() << 8 | Wire.read()) / 16384.0;
  float AccY = (Wire.read() << 8 | Wire.read()) / 16384.0;
  float AccZ = (Wire.read() << 8 | Wire.read()) / 16384.0;

  float accRoll = ((atan2(AccY, AccZ) * 180.0) / PI) - accRollError;
  float accPitch = ((atan2(-AccX, sqrt(AccY * AccY + AccZ * AccZ)) * 180.0) / PI) - accPitchError;

  // 2. ĐỌC DỮ LIỆU CON QUAY HỒI CHUYỂN (GYROSCOPE)
  Wire.beginTransmission(MPU);
  Wire.write(GYRO_XH);       
  Wire.endTransmission(false);
  Wire.requestFrom(MPU, 6, true);
  GyroX = (Wire.read() << 8 | Wire.read()) / 131.0;   
  GyroY = (Wire.read() << 8 | Wire.read()) / 131.0;
  GyroZ = (Wire.read() << 8 | Wire.read()) / 131.0;
  
  // 3. BỘ LỌC BÙ VÀ CỘNG DỒN (Tích phân với dt chuẩn)
  roll  = 0.96 * (roll + (GyroX - froll) * dt) + 0.04 * accRoll;
  pitch = 0.96 * (pitch + (GyroY - fpitch) * dt) + 0.04 * accPitch;

  float actualGyroZ = GyroZ - fyaw;
  if (abs(actualGyroZ) > 0.5) { 
    yaw += actualGyroZ * dt;
  }

  // 4. BỘ LỌC HIỂN THỊ
  if (abs(roll - displayRoll) > 0.05)   displayRoll = roll;
  if (abs(pitch - displayPitch) > 0.05) displayPitch = pitch;
  if (abs(yaw - displayYaw) > 0.05)     displayYaw = yaw;

  // --- CẬP NHẬT GIÁ TRỊ LỚN NHẤT LIÊN TỤC TRONG NỀN ---
  if (abs(displayRoll) > abs(maxX)) maxX = displayRoll;
  if (abs(displayPitch) > abs(maxY)) maxY = displayPitch;
  if (abs(displayYaw) > abs(maxZ)) maxZ = displayYaw;

  // 5. IN RA MÀN HÌNH (Chỉ thực hiện khi có ký tự gửi từ Serial)
 if (Serial.available() > 0) {
    // Đọc toàn bộ dữ liệu người dùng vừa gõ để làm trống bộ đệm
    // Việc này tránh trường hợp gõ "abc" nó in ra 3 lần
    String input = Serial.readStringUntil('\n'); 

    // In ra màn hình
    Serial.print("X: "); Serial.print(displayRoll, 2);
    Serial.print(", Y: "); Serial.print(displayPitch, 2);
    Serial.print(", Z: "); Serial.print(displayYaw, 2);
    
    // In vách ngăn để dễ nhìn
    Serial.print("   ||   MAX -> X: "); Serial.print(maxX, 2);
    Serial.print(", Y: "); Serial.print(maxY, 2);
    Serial.print(", Z: "); Serial.println(maxZ, 2);
  }
}

void filter_gyro() {
  Serial.println("Getting Gyro Infos");
  delay(500);
  for(int x = 0; x < 10; x ++) {
    Serial.print("*");
    delay(100);
  }
  Serial.println("*");

  for(int x = 0; x < 1000; x++) {
    Wire.beginTransmission(MPU);
    Wire.write(0x43);        
    Wire.endTransmission(false);
    Wire.requestFrom(MPU, 6, true);
    froll  += (Wire.read() << 8 | Wire.read())/131.0;
    fpitch += (Wire.read() << 8 | Wire.read())/131.0;
    fyaw   += (Wire.read() << 8 | Wire.read())/131.0;

    Wire.beginTransmission(MPU);
    Wire.write(0x3B);        
    Wire.endTransmission(false);
    Wire.requestFrom(MPU, 6, true);
    float aX = (Wire.read() << 8 | Wire.read()) / 16384.0;
    float aY = (Wire.read() << 8 | Wire.read()) / 16384.0;
    float aZ = (Wire.read() << 8 | Wire.read()) / 16384.0;
    accRollError += (atan2(aY, aZ) * 180.0) / PI;
    accPitchError += (atan2(-aX, sqrt(aY * aY + aZ * aZ)) * 180.0) / PI;
  }
  
  froll /= 1000.0;
  fpitch /= 1000.0;
  fyaw /= 1000.0;
  accRollError /= 1000.0;
  accPitchError /= 1000.0;

  roll = 0;
  pitch = 0;
  yaw = 0;
  
  Serial.println("Taken Gyros Info");
}