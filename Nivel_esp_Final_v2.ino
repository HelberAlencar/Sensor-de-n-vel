#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

// Pinos do ESP32
const int trigger = 12;
const int echo = 14;

// 6 LEDs
const int ledPins[6] = {15, 2, 4, 5, 18, 19};

// Variáveis de medida
long tiempo;
float distancia;
int porc = 0;

// -----------------------------------------
// CONFIGURAÇÃO DO RESERVATÓRIO
// -----------------------------------------
float H_total = 20;   // altura total em cm
float D_top   = 27;   // diâmetro superior
float D_bottom = 24;  // diâmetro inferior

// Distâncias reais medidas pelo usuário
float dist_nivel_max = 11;  // sensor → água quando cheio
float dist_fundo     = 29;  // sensor → fundo

float volumeLitros = 0;


// -----------------------------------------
// Função para calcular volume do tronco de cone
// -----------------------------------------
float calcularVolume(float distanciaMedida) {
  float h = H_total - (distanciaMedida - dist_nivel_max);  

  if (h < 0) h = 0;
  if (h > H_total) h = H_total;

  float R1 = (D_bottom / 2.0);
  float R2 = (D_top / 2.0);

  float V = (3.14159 * h / 3.0) * (R1*R1 + R1*R2 + R2*R2);

  return V / 1000.0;
}


void setup()
{
  Serial.begin(115200);
  Wire.begin();

  lcd.init();
  lcd.backlight();

  pinMode(echo, INPUT);
  pinMode(trigger, OUTPUT);

  for (int i = 0; i < 6; i++) {
    pinMode(ledPins[i], OUTPUT);
  }

  Serial.println("Sistema iniciado no ESP32");
}


void loop()
{
  lcd.clear();

  medir();
  porcentaje();

  volumeLitros = calcularVolume(distancia);

  atualizarLEDs();

  lcd.setCursor(0, 0);
  lcd.print(distancia, 1);
  lcd.print(" cm");

  lcd.setCursor(10, 0);
  lcd.print(porc);
  lcd.print(" %");

  lcd.setCursor(5, 1);
  lcd.print(volumeLitros, 1);
  lcd.print(" L");

  Serial.print("Distancia: ");
  Serial.print(distancia);
  Serial.print(" cm | Porcentagem: ");
  Serial.print(porc);
  Serial.print(" % | Volume: ");
  Serial.print(volumeLitros);
  Serial.println(" L");

  delay(200);
}


// ------------------------------
// Calcula porcentagem REAL
// ------------------------------
void porcentaje()
{
  float h = H_total - (distancia - dist_nivel_max);

  if (h < 0) h = 0;
  if (h > H_total) h = H_total;

  porc = (h / H_total) * 100.0;

  porc = constrain(porc, 0, 100);
}


// ------------------------------
// Mede distância com HC-SR04
// ------------------------------
void medir()
{
  digitalWrite(trigger, LOW);
  delayMicroseconds(2);

  digitalWrite(trigger, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigger, LOW);

  tiempo = pulseIn(echo, HIGH);

  distancia = (tiempo * 0.0343) / 2.0;

  if (distancia > 400 || distancia < 2) {
    distancia = dist_fundo + 5;  // evita 0 fantasma
  }

  delay(10);
}


// ------------------------------
// Atualiza LEDs com base na %
// ------------------------------
void atualizarLEDs()
{
  for (int i = 0; i < 6; i++) {
    digitalWrite(ledPins[i], LOW);
  }

  if (porc > 0)   digitalWrite(ledPins[0], HIGH);
  if (porc >= 20) digitalWrite(ledPins[1], HIGH);
  if (porc >= 40) digitalWrite(ledPins[2], HIGH);
  if (porc >= 60) digitalWrite(ledPins[3], HIGH);
  if (porc >= 75) digitalWrite(ledPins[4], HIGH);
  if (porc >= 90) digitalWrite(ledPins[5], HIGH);
}
