//GND - GND
//VCC - VCC
//SDA - Pin A4
//SCL - Pin A5
 
#include <I2Cdev.h>
#include <MPU6050.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

#define SERVICE_UUID "00000000-0000-0000-0000-000000000001"
#define CHARACTERISTIC_UUID "00000000-0000-0000-0000-000000000002"

BLEServer *pServer;
BLECharacteristic *pCharacteristic;
bool deviceConnected = false;

const int menique = 34;
const int anular = 35;
const int corazon = 32;
const int indice = 25;
const int pulgar = 27;

const int mpuAddress = 0x68;  // Puede ser 0x68 o 0x69
MPU6050 mpu(mpuAddress);
 
int16_t ax, ay, az;
int16_t gx, gy, gz;
 
 
// Factores de conversion
const float accScale = 2.0 * 9.81 / 32768.0;
const float gyroScale = 250.0 / 32768.0;

class MyServerCallbacks : public BLEServerCallbacks {
public:
  void onConnect(BLEServer* pServer) {
    deviceConnected = true;
  }

  void onDisconnect(BLEServer* pServer) {
    deviceConnected = false;
  }
}; 

void setup() {
  Serial.begin(9600);
  pinMode(menique, INPUT);
  pinMode(anular, INPUT);
  pinMode(corazon, INPUT);
  pinMode(indice, INPUT);
  pinMode(pulgar, INPUT);
  Wire.begin();
  // Inicializar el MPU6050
  mpu.initialize();

  // Iniciar BLE
  BLEDevice::init("Tu Gesto es mi Voz"); // Call init() without checking for a return value
  Serial.println("BLE initialized successfully"); // Assume success if init() executes without errors

  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());
  BLEService *pService = pServer->createService(BLEUUID(SERVICE_UUID));
  pCharacteristic = pService->createCharacteristic(
    BLEUUID(CHARACTERISTIC_UUID),
    BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY
  );
  pService->start();

  BLEAdvertising *pAdvertising = pServer->getAdvertising();
  pAdvertising->start();
}

void loop() {
  int flexMenique = analogRead(menique);
  int flexAnular = analogRead(anular);
  int flexCorazon = analogRead(corazon);
  int flexIndice = analogRead(indice);
  int flexPulgar = analogRead(pulgar);

  int scaledMenique = map(flexMenique, 810, 380, 0, 300);
  int scaledAnular = map(flexAnular, 810, 485, 0, 300);
  int scaledCorazon = map(flexCorazon, 810, 450, 0, 300);
  int scaledIndice = map(flexIndice, 660, 250, 0, 300);
  int scaledPulgar = map(flexPulgar, 810, 450, 0, 300);

  // Construyo lecturas de los dedos
  String cad = String(scaledMenique)+";"+String(scaledAnular)+";"+String(scaledCorazon)+";"+String(scaledIndice)+";"+String(scaledPulgar);

  // Leer datos del acelerómetro
  mpu.getAcceleration(&ax, &ay, &az);
  mpu.getRotation(&gx, &gy, &gz);

  // Construyo lecturas del accelerometro
  String cad1 = String(ax * accScale)+";"+String(ay * accScale)+";"+String(az * accScale)+";"+String(gx * gyroScale)+";"+String(gy * gyroScale)+";"+String(gz * gyroScale);

  // Construir el paquete de datos
  String data = cad+";"+cad1;

  // Enviar los datos por Bluetooth
  pCharacteristic->setValue(data.c_str());
  pCharacteristic->notify();

  Serial.println(data);

  delay(2000);
}
