#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// Definição dos pinos
#define BUZZER_PIN 5
#define LED_R 33
#define LED_G 25
#define LED_B 26
#define TRIG_PIN 17   // Pino TRIG do sensor ultrassônico
#define ECHO_PIN 16  // Pino ECHO do sensor ultrassônico

// Configuração do LCD (endereço, colunas, linhas)
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Variáveis
long duration;
int distance;

void setup() {
  Serial.begin(115200);
  
  // Configuração dos pinos
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_R, OUTPUT);
  pinMode(LED_G, OUTPUT);
  pinMode(LED_B, OUTPUT);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  
  // Inicializa o LCD
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Sensor Prox.");
  lcd.setCursor(0, 1);
  lcd.print("Iniciando...");
  delay(2000);
  lcd.clear();
}

void loop() {
  // Lê a distância do sensor
  distance = readDistance();
  
  // Exibe no LCD
  lcd.setCursor(0, 0);
  lcd.print("Distancia:");
  lcd.setCursor(0, 1);
  lcd.print(distance);
  lcd.print(" cm   ");
  
  // Controla LED RGB e Buzzer baseado na distância
  if (distance < 5) {
    // Muito perto - LED vermelho e buzzer contínuo
    setColor(255, 0, 0);
    tone(BUZZER_PIN, 100, 50);
  } 
  else if (distance < 10) {
    // Perto - LED amarelo e buzzer intermitente rápido
    setColor(255, 255, 0);
    tone(BUZZER_PIN, 1000, 100);
    delay(100);
  } 
  else if (distance < 15) {
    // Médio - LED azul e buzzer intermitente lento
    setColor(0, 0, 255);
    tone(BUZZER_PIN, 1500, 200);
    delay(300);
  } 
  else {
    // Longe - LED verde e sem buzzer
    setColor(0, 255, 0);
    noTone(BUZZER_PIN);
  }
  
  delay(100);
}

int readDistance() {
  // Limpa o pino TRIG
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  
  // Envia pulso de 10us
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  
  // Lê o tempo de retorno do ECHO (timeout de 30ms)
  duration = pulseIn(ECHO_PIN, HIGH, 30000);
  
  // Calcula a distância em cm
  int dist = duration * 0.034 / 2;
  
  // Limita valores inválidos
  if (dist == 0 || dist > 600) {
    dist = 0.5; // Se não detectar ou estiver muito longe, considera muito perto
  }
  
  return dist;
}

void setColor(int red, int green, int blue) {
  analogWrite(LED_R, red);
  analogWrite(LED_G, green);
  analogWrite(LED_B, blue);
}