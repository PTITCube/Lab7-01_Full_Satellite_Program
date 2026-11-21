/**
 * LAB 7 - VÍ DỤ 01: TÍCH HỢP HỆ THỐNG TOÀN DIỆN (SEQUENTIAL LOOP)
 */
#include <Arduino.h>
#include <ArduinoJson.h>
#include <math.h>
#include <PTITCube.h>

PTIT_CDH cdh;

PTIT_EPS eps;
PTIT_Sensor sensor;
PTIT_GPS gps;
PTIT_Camera camera;
PTIT_Storage storage;
PTIT_COM com;

unsigned long lastTelemetryTime = 0;
const unsigned long TELEMETRY_INTERVAL = 5000;

void setup() {
    cdh.begin();

    eps.init();
    sensor.init();
    gps.init(12, 14);
    storage.init();
    camera.init(12, 14);
    com.init();

}

void loop() {
    eps.update();
    sensor.update();
    
    camera.selectGPS();
    gps.update();

    unsigned long currentMillis = millis();

    if (currentMillis - lastTelemetryTime >= TELEMETRY_INTERVAL) {
        lastTelemetryTime = currentMillis;

        JsonDocument doc;

        JsonObject epsObj = doc["eps"].to<JsonObject>();
        epsObj["v"]    = eps.getBatteryVoltage();
        epsObj["pct"]  = eps.getBatteryPercentage();
        epsObj["rate"] = eps.getChargeRate();

        JsonObject envObj = doc["env"].to<JsonObject>();
        envObj["temp"]  = sensor.getTemperature();
        envObj["press"] = sensor.getPressure();

        JsonObject imuObj = doc["imu"].to<JsonObject>();
        imuObj["roll"]  = sensor.getRoll();
        imuObj["pitch"] = sensor.getPitch();
        imuObj["head"]  = sensor.getHeading();

        if (gps.hasFix()) {
            JsonObject gpsObj = doc["gps"].to<JsonObject>();
            gpsObj["lat"]  = gps.getLatitude();
            gpsObj["lng"]  = gps.getLongitude();
            gpsObj["alt"]  = gps.getAltitude();
            gpsObj["sats"] = gps.getSatellites();
        }

        String telemetryString;
        serializeJson(doc, telemetryString);

        cdh.printf("TELEM: %s\n", telemetryString.c_str());

        com.sendMessage(telemetryString);
        storage.logData(telemetryString.c_str());
    }
}
