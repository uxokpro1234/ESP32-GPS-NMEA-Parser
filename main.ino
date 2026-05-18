#include <HardwareSerial.h>
#include <BluetoothSerial.h>

BluetoothSerial SerialBT;
HardwareSerial GPSSerial(1);

#define RXD2 16
#define TXD2 17

struct GPSInfo {
    double latitude = 0;
    double longitude = 0;
    double altitude = 0;
    double speedKmh = 0;
    double speedKnots = 0;
    int fixStatus = 0;
    int satellitesUsed = 0;
};

struct Satellite {
    int prn = 0;
    int elevation = 0;
    int azimuth = 0;
    int snr = 0;
};

GPSInfo gps;
Satellite satellites[32];  // max 32 satellites
int satCount = 0;

// --- NMEA helpers ---
double nmeaToDecimal(const String &val, char dir) {
    if (val.length() < 4) return 0.0;
    int dot = val.indexOf('.');
    int degLength = (dot > 2) ? dot - 2 : 2;
    double deg = val.substring(0, degLength).toDouble();
    double min = val.substring(degLength).toDouble();
    double dec = deg + min / 60.0;
    if (dir == 'S' || dir == 'W') dec = -dec;
    return dec;
}

double knotsToKmh(double knots) { return knots * 1.852; }

// Split NMEA line into fields
void splitNMEA(const String &line, String fields[], int maxFields) {
    int start = 0;
    int field = 0;
    while (field < maxFields - 1) {
        int comma = line.indexOf(',', start);
        if (comma == -1) break;
        fields[field++] = line.substring(start, comma);
        start = comma + 1;
    }
    fields[field] = line.substring(start);
}

// --- Parse functions ---
void parseGGA(const String &line) {
    String fields[15];
    splitNMEA(line, fields, 15);

    gps.fixStatus = fields[6].toInt();
    gps.satellitesUsed = fields[7].toInt();
    gps.latitude = nmeaToDecimal(fields[2], fields[3].charAt(0));
    gps.longitude = nmeaToDecimal(fields[4], fields[5].charAt(0));
    gps.altitude = fields[9].toDouble();
}

void parseRMC(const String &line) {
    String fields[12];
    splitNMEA(line, fields, 12);

    double speedKnots = fields[7].toDouble();
    gps.speedKnots = speedKnots;
    gps.speedKmh = knotsToKmh(speedKnots);
}

void parseGSV(const String &line) {
    String fields[20];
    splitNMEA(line, fields, 20);

    int totalSentences = fields[1].toInt();
    int sentenceNumber = fields[2].toInt();

    for (int i = 0; i < 4; i++) {
        int base = 4 + i * 4;
        if (base + 3 >= 20 || fields[base].length() == 0) break;

        satellites[satCount].prn = fields[base].toInt();
        satellites[satCount].elevation = fields[base + 1].toInt();
        satellites[satCount].azimuth = fields[base + 2].toInt();
        satellites[satCount].snr = fields[base + 3].toInt();
        satCount++;
        if (satCount >= 32) break;
    }

    if (sentenceNumber == totalSentences) {
        Serial.println("--- GPS Info ---");
        Serial.println("Latitude: " + String(gps.latitude, 6));
        Serial.println("Longitude: " + String(gps.longitude, 6));
        Serial.println("Altitude: " + String(gps.altitude, 2) + " m");
        Serial.println("Speed: " + String(gps.speedKmh, 2) + " km/h (" + String(gps.speedKnots, 2) + " kts)");
        Serial.println("Fix Status: " + String(gps.fixStatus));
        Serial.println("Satellites used: " + String(gps.satellitesUsed));

        Serial.println("Satellites in view:");
        for (int i = 0; i < satCount; i++) {
            Serial.printf("PRN:%d Elev:%d Az:%d SNR:%d\n",
                          satellites[i].prn,
                          satellites[i].elevation,
                          satellites[i].azimuth,
                          satellites[i].snr);
        }
        Serial.println("---");

        // Bluetooth output
        SerialBT.println("--- GPS Info ---");
        SerialBT.println("Latitude: " + String(gps.latitude, 6));
        SerialBT.println("Longitude: " + String(gps.longitude, 6));
        SerialBT.println("Altitude: " + String(gps.altitude, 2) + " m");
        SerialBT.println("Speed: " + String(gps.speedKmh, 2) + " km/h (" + String(gps.speedKnots, 2) + " kts)");
        SerialBT.println("Fix Status: " + String(gps.fixStatus));
        SerialBT.println("Satellites used: " + String(gps.satellitesUsed));
        for (int i = 0; i < satCount; i++) {
            SerialBT.printf("PRN:%d Elev:%d Az:%d SNR:%d\n",
                            satellites[i].prn,
                            satellites[i].elevation,
                            satellites[i].azimuth,
                            satellites[i].snr);
        }
        SerialBT.println("---");

        satCount = 0; // Reset for next epoch
    }
}

// --- Setup & Loop ---
void setup() {
    Serial.begin(115200);
    SerialBT.begin("ESP32_GPS");
    GPSSerial.begin(38400, SERIAL_8N1, RXD2, TXD2);
    Serial.println("=== GPS Parser Ready ===");
}

void loop() {
    static String line;

    while (GPSSerial.available()) {
        char c = GPSSerial.read();
        if (c == '\n' || c == '\r') {
            line.trim();
            if (line.length() > 0) {
                if (line.startsWith("$GNGGA") || line.startsWith("$GPGGA")) parseGGA(line);
                else if (line.startsWith("$GNRMC") || line.startsWith("$GPRMC")) parseRMC(line);
                else if (line.indexOf("GSV") != -1) parseGSV(line);
            }
            line = "";
        } else line += c;
    }
}
