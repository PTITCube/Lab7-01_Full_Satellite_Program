/**
 * @file main.cpp
 * @brief Ví dụ tổng hợp toàn bộ các phân hệ của vệ tinh CubeSat (Sequential Loop).
 * 
 * Mô tả:
 * - Khởi tạo toàn bộ các module: EPS, Sensor, GPS, Camera, Storage, COM (LoRa).
 * - Sử dụng cdh (PTIT_CDH) chuẩn hóa phân hệ C&DH theo chuẩn PTITCube.
 * - Thu thập dữ liệu, đóng gói JSON, truyền qua LoRa và lưu MicroSD.
 */

#include <Arduino.h>
#include <ArduinoJson.h>
#include <math.h>
#include <PTITCube.h>

PTIT_CDH cdh;

// Khởi tạo các phân hệ
PTIT_EPS eps;
PTIT_Sensor sensor;
PTIT_GPS gps;
PTIT_Camera camera;
PTIT_Storage storage;
PTIT_COM com;

unsigned long lastTelemetryTime = 0;
const unsigned long TELEMETRY_INTERVAL = 5000; // 5 giây gửi telemetry 1 lần

void setup() {
    cdh.begin(115200);
    delay(1000);
    
    cdh.println("[BOOT] Lab 7-01: Satellite System Init");

    eps.init();
    sensor.init();
    gps.init();
    storage.init();
    camera.init(12, 14); // Chân MUX
    com.init();

    cdh.println("[BOOT] All subsystems ready");
}

void loop() {
    // 1. Cập nhật dữ liệu từ các cảm biến định kỳ
    eps.update();
    sensor.update();
    
    // Bật kênh GPS để lấy vị trí
    camera.selectGPS();
    gps.update();

    unsigned long currentMillis = millis();

    // 2. Gửi Telemetry sau mỗi khoảng thời gian định trước
    if (currentMillis - lastTelemetryTime >= TELEMETRY_INTERVAL) {
        lastTelemetryTime = currentMillis;

        // Tạo Document JSON (Chuẩn ArduinoJson v7)
        JsonDocument doc;

        // Dữ liệu Nguồn (EPS)
        JsonObject epsObj = doc["eps"].to<JsonObject>();
        epsObj["v"] = eps.getBatteryVoltage();
        epsObj["pct"] = eps.getBatteryPercentage();
        epsObj["rate"] = eps.getChargeRate();

        // Dữ liệu Môi trường (Sensor)
        JsonObject envObj = doc["env"].to<JsonObject>();
        envObj["temp"] = sensor.getTemperature();
        envObj["press"] = sensor.getPressure();

        // Dữ liệu Tư thế (IMU)
        JsonObject imuObj = doc["imu"].to<JsonObject>();
        float ax = sensor.getAccX();
        float ay = sensor.getAccY();
        float az = sensor.getAccZ();
        imuObj["roll"] = atan2(ay, az) * 180.0 / PI;
        imuObj["pitch"] = atan2(-ax, sqrt(ay * ay + az * az)) * 180.0 / PI;
        imuObj["yaw"] = atan2(sensor.getMagY(), sensor.getMagX()) * 180.0 / PI;

        // Dữ liệu Định vị (GPS)
        if (gps.hasFix()) {
            JsonObject gpsObj = doc["gps"].to<JsonObject>();
            gpsObj["lat"] = gps.getLatitude();
            gpsObj["lng"] = gps.getLongitude();
            gpsObj["alt"] = gps.getAltitude();
            gpsObj["sats"] = gps.getSatellites();
        }

        // Chuyển đổi JSON thành chuỗi
        String telemetryString;
        serializeJson(doc, telemetryString);

        cdh.printf("[TELEM] %s\n", telemetryString.c_str());

        // 3. Gửi dữ liệu qua LoRa
        com.sendMessage(telemetryString);

        // 4. Lưu log vào thẻ nhớ
        storage.logData(telemetryString.c_str());
    }
}
