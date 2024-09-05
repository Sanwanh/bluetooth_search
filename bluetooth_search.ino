#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
#include <ArduinoJson.h>
#include <esp_bt_device.h>
#include <esp_gap_ble_api.h>
#include "esp_system.h"


// #define MANUFACTURER_NAME "team-nothing-bluetooth-scanner"
// #define PRODUCT_ID 0x8787

int scanTime = 5; // 掃描時間（秒）
BLEScan* pBLEScan;
class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks {
    void onResult(BLEAdvertisedDevice advertisedDevice) {
        // 這個函數會在每次發現新裝置時被調用
    }
};

void setup() {
  Serial.begin(115200);
  BLEDevice::init("");
  pBLEScan = BLEDevice::getScan(); // 創建新的掃描
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true); // 主動掃描消耗更多電力，但更快
  pBLEScan->setInterval(100);
  pBLEScan->setWindow(99);  // 較小的間隔會更快地掃描，但會消耗更多電力
}
void loop() {
  pBLEScan->start(scanTime, false);

  // 获取自己的MAC地址
  const uint8_t* ownMac = esp_bt_dev_get_address();
  char ownMacStr[18];
  snprintf(ownMacStr, sizeof(ownMacStr), "%02X:%02X:%02X:%02X:%02X:%02X",
           ownMac[0], ownMac[1], ownMac[2], ownMac[3], ownMac[4], ownMac[5]);

  // 创建初始 JSON 文档
  DynamicJsonDocument doc(4096);

  // 设置MAC地址
  doc["MAC"] = ownMacStr;

  // 创建nearby_devices数组
  JsonArray nearby_devices = doc.createNestedArray("nearby_devices");

  int deviceCount = pBLEScan->getResults()->getCount();
  for (int i = 0; i < deviceCount; i++) {
    BLEAdvertisedDevice device = pBLEScan->getResults()->getDevice(i);

    JsonObject deviceObj = nearby_devices.createNestedObject();
    deviceObj["rssi"] = device.getRSSI();
    deviceObj["MAC"] = device.getAddress().toString();
  }

  String jsonOutput;
  serializeJson(doc, jsonOutput);

  // 创建包含 "data" 和 "length" 的 JSON 文档并发送
  DynamicJsonDocument firstDoc(512);
  firstDoc["data"] = "bluetooth";
  firstDoc["length"] = jsonOutput.length();  // 计算总字符串长度

  String firstJsonOutput;
  serializeJson(firstDoc, firstJsonOutput);
  Serial.println(firstJsonOutput);

  // 短暂延迟后发送 MAC 地址和附近设备信息
  delay(100);

  // 创建最终包含MAC地址和附近设备信息的JSON文档并发送
  String finalJsonOutput;
  serializeJson(doc, finalJsonOutput);  // 直接使用之前创建的文档
  Serial.println(finalJsonOutput);

  pBLEScan->clearResults();   // 删除扫描结果以释放内存
}
