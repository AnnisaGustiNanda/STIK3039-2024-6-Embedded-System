#include <LiquidCrystal_I2C.h>

// Konfigurasi sensor aliran air
volatile int pulsa_sensor = 0; // Variabel untuk menghitung pulsa dari sensor
unsigned long waktuAktual;     // Waktu saat ini
unsigned long waktuLoop = 0;   // Waktu loop sebelumnya
double volumeAir = 0;          // Volume air total dalam liter
unsigned int literPerjam;      // Debit air dalam liter per jam
const unsigned char pinFlowsensor = 2; // Pin sensor aliran air
const int ledPin = 13;                 // Pin untuk LED indikator

// Konfigurasi sensor ultrasonik
int trigPin = 9;    // TRIG pin
int echoPin = 8;    // ECHO pin
float duration_us, distance_cm;

// Konfigurasi LCD
int lcdColumns = 16;
int lcdRows = 2;
LiquidCrystal_I2C lcd(0x27, lcdColumns, lcdRows);

// Data tren harian (dalam cm)
float dataHarian[7] = {10.5, 9.9, 9.5, 9.2, 7.0, 6.9, 0.0}; // Hari ke-7 diisi saat runtime

void cacahPulsa() {
  pulsa_sensor++; // Increment jumlah pulsa setiap interrupt
}

void setup() {
  // Konfigurasi sensor aliran air
  pinMode(pinFlowsensor, INPUT);  // Sensor sebagai input
  pinMode(ledPin, OUTPUT);        // LED sebagai output
  digitalWrite(pinFlowsensor, HIGH); // Pull-up resistor untuk pin sensor
  attachInterrupt(digitalPinToInterrupt(pinFlowsensor), cacahPulsa, RISING); // Interrupt pada sinyal naik

  // Konfigurasi sensor ultrasonik
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  // Inisialisasi komunikasi serial
  Serial.begin(9600);

  // Inisialisasi LCD
  lcd.init();
  lcd.backlight();

  // Menampilkan pesan awal di LCD
  lcd.setCursor(0, 0);
  lcd.print("Inisialisasi...");
  delay(2000); // Tunggu 2 detik
  lcd.clear();
}

void loop() {
  // Mengukur durasi pulsa dari sensor ultrasonik
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  duration_us = pulseIn(echoPin, HIGH);
  distance_cm = 0.017 * duration_us;

  // Update data untuk hari ke-7
  dataHarian[6] = distance_cm;

  // Hitung tren berdasarkan data harian
  String trenAir = hitungTren(dataHarian);

  // Proses setiap 1 detik untuk sensor aliran air
  waktuAktual = millis();
  if (waktuAktual - waktuLoop >= 1000) {
    waktuLoop = waktuAktual;

    // Hitung debit air dalam liter per jam
    literPerjam = (pulsa_sensor * 60) / 7.5;

    // Hitung volume air total dalam liter
    double literPerDetik = literPerjam / 3600.0; // Konversi ke liter per detik
    volumeAir += literPerDetik;                 // Tambahkan ke total volume air

    // Reset pulsa untuk penghitungan berikutnya
    pulsa_sensor = 0;

    // Tampilkan volume air di Serial Monitor
    Serial.print("Volume air total: ");
    Serial.print(volumeAir, 2);
    Serial.println(" liter");
  }

  // Tampilkan tren di Serial Monitor
  Serial.print("Tren Air: ");
  Serial.println(trenAir);

  // Menampilkan data pada LCD
  lcd.setCursor(0, 0);
  lcd.print("Volume:");
  lcd.print(volumeAir, 2);
  lcd.print(" L   "); // Tambahkan spasi untuk membersihkan sisa karakter

  lcd.setCursor(0, 1);
  lcd.print("Tren: ");
  lcd.print(trenAir);
  lcd.print("     "); // Tambahkan spasi untuk membersihkan sisa karakter

  // Tunggu 500 ms sebelum pengukuran berikutnya
  delay(500);
}

// Fungsi untuk menghitung tren
String hitungTren(float data[]) {
  float selisih = data[6] - data[5]; // Bandingkan hari ke-7 dengan hari ke-6
  if (selisih < 0) {
    return "Naik"; // Air naik (jarak berkurang)
  } else if (selisih > 0) {
    return "Turun"; // Air turun (jarak bertambah)
  }
  return "Stabil"; // Tidak ada perubahan signifikan (opsional jika ingin tetap dimasukkan)
}