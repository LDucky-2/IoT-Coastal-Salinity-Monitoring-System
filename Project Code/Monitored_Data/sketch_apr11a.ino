#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>
#include <TinyGPS++.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// Pins
#define ONE_WIRE_BUS 4      // Connect DS18B20 Yellow wire to Digital Pin 4
#define TdsSensorPin A1
#define VREF 5.0      

// Setup OneWire and Sensors
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

LiquidCrystal_I2C lcd(0x27, 16, 2); 
SoftwareSerial gpsSerial(2, 3); 
TinyGPSPlus gps;

void setup() {
  Serial.begin(115200); 
  gpsSerial.begin(9600);
  sensors.begin(); // Start DS18B20
  
  lcd.init();
  lcd.backlight();
  lcd.print("Coastal Monitor");
  delay(1000);
}

void loop() {
  // 1. Digital Temperature Reading
  sensors.requestTemperatures(); 
  float temperature = sensors.getTempCByIndex(0);

  // 2. TDS & Salinity Calculation
  float voltage = analogRead(TdsSensorPin) * VREF / 1024.0;
  float compensationCoefficient = 1.0 + 0.02 * (temperature - 25.0);
  float tdsValue = (133.42 * pow(voltage/compensationCoefficient, 3) - 255.86 * pow(voltage/compensationCoefficient, 2) + 857.39 * (voltage/compensationCoefficient)) * 0.5;
  float salinity = tdsValue / 1000.0;

  // 3. GPS Processing
  while (gpsSerial.available() > 0) gps.encode(gpsSerial.read());

  // 4. Update LCD
  lcd.setCursor(0, 0);
  lcd.print("T:"); lcd.print(temperature, 1);
  lcd.print("C S:"); lcd.print(salinity, 1); lcd.print("ppt ");
  
  lcd.setCursor(0, 1);
  if (gps.location.isValid()) {
    lcd.print("GPS: Fixed      ");
  } else {
    lcd.print("Searching GPS...");
  }

  // 5. Send CSV to Python (Temp, TDS, Salinity, Lat, Lng)
  Serial.print(temperature, 2); Serial.print(",");
  Serial.print(tdsValue, 0); Serial.print(",");
  Serial.print(salinity, 2); Serial.print(",");
  Serial.print(gps.location.lat(), 6); Serial.print(",");
  Serial.println(gps.location.lng(), 6);

  delay(1000);
}