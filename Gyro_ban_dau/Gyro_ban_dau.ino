#include <Wire.h> // I2C

#define ACCEL_XOUTH 0x3B
#define ACCEL_XOUTL 0x3C
#define ACCEL_YOUTH 0x3D
#define ACCEL_YOUTL 0x3E
#define ACCEL_ZOUTH 0x3F
#define ACCEL_ZOUTL 0x40

#define GYRO_XH     0x43
#define GYRO_XL     0x44

const int MPU = 0x68; // MPU6050 I2C address
//float AccX, AccY, AccZ;
float GyroX, GyroY, GyroZ;
float roll, pitch, yaw, froll, fpitch, fyaw = 0;

float elapsedTime, currentTime, previousTime;
float dt = 0.015;
bool turn_fillter = false;
void setup()
{
  Wire.begin();
  Wire.beginTransmission(MPU);
  Wire.write(0x6B);     // register set mode
  Wire.write(0x00);
  Wire.endTransmission();
  Serial.begin(9600);
}

void loop()
{
  if(turn_fillter == false)
  {
    filter_gyro();
    turn_fillter = true;
  }

  previousTime = millis();
  Wire.beginTransmission(MPU);
  Wire.write(GYRO_XH);       // start with firts gyroscope register
  Wire.endTransmission(false);
  Wire.requestFrom(MPU, 6, true);
  GyroX = (Wire.read() << 8 | Wire.read())/131.0;   // according datasheet, Gyro/131 = degree/second
  GyroY = (Wire.read() << 8 | Wire.read())/131.0;
  GyroZ = (Wire.read() << 8 | Wire.read())/131.0;
  roll  += (GyroX -froll)  *dt;
  pitch += (GyroY - fpitch) *dt;
  yaw   += (GyroZ - fyaw)  *dt;
  Serial.print("X : "); Serial.print(roll, 2);
  Serial.print(", Y : "); Serial.print(pitch, 2);
  Serial.print(", Z : "); Serial.println(yaw, 2);
  currentTime = millis();
  elapsedTime = currentTime - previousTime;
  dt = elapsedTime / 1000;
}

void filter_gyro()
{
  Serial.println("Start probe filter");
  delay(500);
  for(int x = 0; x < 10; x ++)
  {
    Serial.print("*");
    delay(100);
  }
  Serial.println("*");

  for(int x = 0; x < 1000; x++)
  {
    Wire.beginTransmission(MPU);
    Wire.write(0x43);        // start with firts gyroscope register
    Wire.endTransmission(false);
    Wire.requestFrom(MPU, 6, true);
    GyroX = (Wire.read() << 8 | Wire.read())/131.0;
    GyroY = (Wire.read() << 8 | Wire.read())/131.0;
    GyroZ = (Wire.read() << 8 | Wire.read())/131.0;
    froll  += GyroX;
    fpitch += GyroY;
    fyaw   += GyroZ;
  }
  
  froll = froll / 1000;
  fpitch = fpitch / 1000;
  fyaw = fyaw / 1000;
  Serial.println("Probe filter done!");
  Serial.print("X filter : "); Serial.print(froll, 2);
  Serial.print(", Y filter : "); Serial.print(fpitch, 2);
  Serial.print(", Z filter : "); Serial.println(fyaw, 2);
}