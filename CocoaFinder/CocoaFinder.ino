#include <M5StickC.h>
#include <BLEDevice.h>

int scanTime = 5; //In seconds
BLEScan* pBLEScan;

const char* uuid = "0000fd6f-0000-1000-8000-00805f9b34fb";
int deviceNum = 0;
int rssis[50];

int desc(const void *a, const void *b) {
  return *(int *)a - *(int *)b;
}

class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice advertisedDevice) {
    if(advertisedDevice.haveServiceUUID()){
      if(strncmp(advertisedDevice.getServiceUUID().toString().c_str(),uuid, 36) == 0){
        int rssi = advertisedDevice.getRSSI();
        Serial.print("  >> ADDR: ");
        Serial.print(advertisedDevice.getAddress().toString().c_str());
        Serial.print(" , ");
        Serial.print("RSSI: ");
        Serial.print(rssi);
        if (deviceNum < sizeof rssis) {
          rssis[deviceNum++] = rssi;
        }
        Serial.println("");
      }
    }
  }
};

void setup() {
  M5.begin(true, false, true);
  pinMode(GPIO_NUM_10, OUTPUT);
  M5.Lcd.setRotation( 1 );
  M5.Lcd.fillScreen(BLACK);
//    Serial.begin(115200);
  Serial.println("Scanning...");
  BLEDevice::init("");
  pBLEScan = BLEDevice::getScan(); //create new scan
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true); //active scan uses more power, but get results faster
  pBLEScan->setInterval(100);
  pBLEScan->setWindow(99); // less or equal setInterval value

  M5.Lcd.setTextSize(1);
}

void led() {
  int wait = 1000;
  digitalWrite(GPIO_NUM_10, LOW);
  delay(50);
  digitalWrite(GPIO_NUM_10, HIGH);
  delay(50);

  for (int i = 0; i < deviceNum; i++) {
    digitalWrite(GPIO_NUM_10, LOW);
    delay(50);
    digitalWrite(GPIO_NUM_10, HIGH);
    delay(50);
    wait -= 100;
  }
  digitalWrite(GPIO_NUM_10, HIGH);
  delay(wait);
}

void loop(){
  M5.update();

  Serial.println("Loop Start.");
  deviceNum = 0;
  BLEScanResults foundDevices = pBLEScan->start(scanTime, false); 

  // ソートする
  qsort(rssis, sizeof(int) * deviceNum, sizeof(int), desc);
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setTextColor(WHITE);

  M5.Lcd.setTextFont(4);
  M5.Lcd.setCursor(0, 0);
  M5.Lcd.print("Found : ");
  M5.Lcd.println(deviceNum);

  M5.Lcd.setTextFont(2);
  int n = 20;
  for (int i = 0; i < deviceNum; ++i, n += 10) { 
    M5.Lcd.setCursor(0, n);
    M5.Lcd.printf("%4d", rssis[i]);
    uint16_t color = RED;
    if (rssis[i] < -120) {
      rssis[i] = -120;
      color = RED;
    } else if(rssis[i] < -50) {
      color = YELLOW;      
    } else if(rssis[i] < 0) {
      color = GREEN;      
    } else {
      rssis[i] = 0;      
    }
    M5.Lcd.fillRect(40, n + 3, rssis[i] + 130, 8, color);
  }

  pBLEScan->clearResults(); // delete results fromBLEScan buffer to release memory
  Serial.println("Loop End.");
  led();
}
