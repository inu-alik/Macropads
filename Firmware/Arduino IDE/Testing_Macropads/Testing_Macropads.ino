#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// --- 1. Definisi Pin Komponen (Sesuai Jalur PCB Anda) ---
const int rowPins[3] = {16, 17, 18}; // Baris 0, 1, 2
const int colPins[3] = {19, 27, 14}; // Kolom 0, 1, 2

const int encAPin = 25; // Sinyal A Encoder (Rotasi)
const int encBPin = 26; // Sinyal B Encoder (Rotasi)

const int slider1Pin = 34; // RV1 (Volume)
const int slider2Pin = 35; // RV2 (Brightness)

// --- 2. Variabel Status & Debouncing ---
int lastButtonState[3][3] = {{HIGH,HIGH,HIGH}, {HIGH,HIGH,HIGH}, {HIGH,HIGH,HIGH}};
unsigned long lastMatrixDebounce[3][3] = {{0,0,0}, {0,0,0}, {0,0,0}};
const unsigned long matrixDebounceDelay = 40;

int lastStateA;
unsigned long lastEncDebounceTime = 0;
const unsigned long encDebounceDelay = 5;

int lastSlider1 = 0;
int lastSlider2 = 0;
unsigned long lastSliderTime = 0;

String currentSong = "Tidak Ada Musik";

// --- 3. Fungsi-Fungsi Tampilan Layar OLED ---
void tampilkanMenu(String baris1, String baris2) {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  
  // Header Statis
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("=== MACROPADS ===");
  
  // Baris 1: Informasi Komponen/Aksi
  display.setTextSize(1);
  display.setCursor(0, 16);
  display.println(baris1);
  
  // Baris 2: Nilai / Status Detail
  display.setTextSize(1);
  display.setCursor(0, 36);
  display.println(baris2);
  
  display.display();
}

void kembalikanKeTampilanMusik() {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("=== MACROPADS ===");
  
  display.setCursor(0, 18);
  display.println("🎵 NOW PLAYING:");
  
  // Menampilkan judul musik (Otomatis turun baris jika terlalu panjang)
  display.setCursor(0, 32);
  display.println(currentSong);
  
  display.display();
}

// --- 4. Setup Awal ---
void setup() {
  Serial.begin(115200);
  Serial.setTimeout(10); // Respons baca data serial dipercepat agar matriks tidak lag

  // Inisialisasi Layar OLED (Alamat I2C: 0x3C)
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("OLED SSD1306 tidak terdeteksi!"));
  }
  
  display.clearDisplay();
  tampilkanMenu("SISTEM READY", "Menunggu Python...");

  // Konfigurasi Pin Matriks Tombol
  for(int i = 0; i < 3; i++) {
    pinMode(rowPins[i], OUTPUT);
    digitalWrite(rowPins[i], HIGH);
    pinMode(colPins[i], INPUT_PULLUP);
  }
  
  // Konfigurasi Pin Rotary Encoder
  pinMode(encAPin, INPUT_PULLUP);
  pinMode(encBPin, INPUT_PULLUP);
  lastStateA = digitalRead(encAPin);
}

// --- 5. Loop Utama ---
void loop() {
  
  // =========================================================
  // FASE 1: MENERIMA DATA JUDUL MUSIK DARI PYTHON (DUPIEKS)
  // =========================================================
  if (Serial.available() > 0) {
    String dataMasuk = Serial.readStringUntil('\n');
    dataMasuk.trim();
    
    // Perbaikan: Menggunakan camelCase 'startsWith' resmi Arduino C++
    if (dataMasuk.startsWith("MSC:")) {
      currentSong = dataMasuk.substring(4);
      kembalikanKeTampilanMusik(); 
    }
  }

  // =========================================================
  // FASE 2: MEMBACA PUTARAN ROTARY ENCODER
  // =========================================================
  int currentStateA = digitalRead(encAPin);
  if (currentStateA != lastStateA) {
    if ((millis() - lastEncDebounceTime) > encDebounceDelay) {
      if (currentStateA == LOW) {
        if (digitalRead(encBPin) == HIGH) {
          Serial.println("ENC:CW");
          tampilkanMenu("VOL/TRACK CHANGE", "Putar Kanan (CW)");
        } else {
          Serial.println("ENC:CCW");
          tampilkanMenu("VOL/TRACK CHANGE", "Putar Kiri (CCW)");
        }
      }
      lastEncDebounceTime = millis();
    }
  }
  lastStateA = currentStateA;

  // =========================================================
  // FASE 3: MEMBACA MATRIKS TOMBOL & KLIK ENCODER
  // =========================================================
  for(int r = 0; r < 3; r++) {
    digitalWrite(rowPins[r], LOW);
    
    for(int c = 0; c < 3; c++) {
      int currentState = digitalRead(colPins[c]);
      
      if (currentState != lastButtonState[r][c]) {
        if ((millis() - lastMatrixDebounce[r][c]) > matrixDebounceDelay) {
          if (currentState == LOW) {
            // HACK JALUR PCB: Klik Encoder terdeteksi di Baris 1, Kolom 2
            if (r == 1 && c == 2) {
              Serial.println("BTN:ENC");
              tampilkanMenu("MEDIA ACTION", "KLIK ENCODER");
            } else {
              Serial.print("BTN:"); Serial.print(r); Serial.print(","); Serial.println(c);
              tampilkanMenu("TOMBOL AKTIF", "Baris:" + String(r) + " Kolom:" + String(c));
            }
          }
          lastButtonState[r][c] = currentState;
          lastMatrixDebounce[r][c] = millis();
        }
      }
    }
    digitalWrite(rowPins[r], HIGH);
  }

  // =========================================================
  // FASE 4: MEMBACA DUAL MONO SLIDER (INVERTED FIX)
  // =========================================================
  if (millis() - lastSliderTime > 100) {
    // Membalik nilai analog secara software (4095 - Nilai Asli)
    int s1 = 4095 - analogRead(slider1Pin);
    int s2 = 4095 - analogRead(slider2Pin);
    
    // Kirim data hanya jika ada pergeseran signifikan (>150) untuk mencegah lag
    if (abs(s1 - lastSlider1) > 150) {
      Serial.print("SLD:1,"); Serial.println(s1);
      tampilkanMenu("CONTROL: VOLUME", "Nilai: " + String(int((s1/4095.0)*100)) + "%");
      lastSlider1 = s1;
    }
    if (abs(s2 - lastSlider2) > 150) {
      Serial.print("SLD:2,"); Serial.println(s2);
      tampilkanMenu("CONTROL: BRIGHTNESS", "Nilai: " + String(int((s2/4095.0)*100)) + "%");
      lastSlider2 = s2;
    }
    lastSliderTime = millis();
  }
}