/*
 * Controle de Semáforo com Display LCD
 * 
 * Descrição: Sistema de controle de semáforo com três fases
 * - Vermelho: 6 segundos
 * - Verde: 4 segundos  
 * - Amarelo: 2 segundos
 * 
 * Componentes:
 * - LEDs: Vermelho (pino 27), Amarelo (pino 26), Verde (pino 25)
 * - Display LCD I2C: SDA (pino 21), SCL (pino 22)
 * 
 * Autor: Davi Duarte
 */

#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define PIN_LED_VERMELHO 27
#define PIN_LED_AMARELO  26
#define PIN_LED_VERDE    25

#define TEMPO_VERMELHO 6000  // 6 segundos
#define TEMPO_VERDE    4000  // 4 segundos
#define TEMPO_AMARELO  2000  // 2 segundos

enum FaseSemaforo {
  FASE_VERMELHO = 0,
  FASE_VERDE    = 1,
  FASE_AMARELO  = 2
};

struct ControleSemaforo {
  uint8_t* pinVermelho; // Ponteiro para o pino do LED vermelho
  uint8_t* pinAmarelo; // Ponteiro para o pino do LED amarelo
  uint8_t* pinVerde; // Ponteiro para o pino do LED verde
  FaseSemaforo faseAtual; // Fase atual do semáforo
  unsigned long* tempoInicio; // Ponteiro para controle de tempo
};

LiquidCrystal_I2C lcd(0x27, 16, 2);

// Variáveis que serão acessadas por ponteiros
uint8_t pinoVermelho = PIN_LED_VERMELHO;
uint8_t pinoAmarelo = PIN_LED_AMARELO;
uint8_t pinoVerde = PIN_LED_VERDE;
unsigned long tempoInicioFase = 0;

ControleSemaforo* controle;

void inicializarSemaforo(ControleSemaforo** ctrl);
void atualizarSemaforo(ControleSemaforo* ctrl);
void mudarFase(ControleSemaforo* ctrl, FaseSemaforo novaFase);
void atualizarLCD(FaseSemaforo fase, int tempoRestante);
int calcularTempoRestante(ControleSemaforo* ctrl);
unsigned long obterDuracaoFase(FaseSemaforo fase);

void setup() {
  Serial.begin(115200);
  
  // Configurar pinos dos LEDs como saída
  pinMode(PIN_LED_VERMELHO, OUTPUT);
  pinMode(PIN_LED_AMARELO, OUTPUT);
  pinMode(PIN_LED_VERDE, OUTPUT);
  
  // Garantir que todos os LEDs iniciem apagados
  digitalWrite(PIN_LED_VERMELHO, LOW);
  digitalWrite(PIN_LED_AMARELO, LOW);
  digitalWrite(PIN_LED_VERDE, LOW);
  
  // Iniciar display LCD
  lcd.init();
  lcd.backlight();
  lcd.clear();
  
  // Mensagem fofinha no LCD
  lcd.setCursor(0, 0);
  lcd.print("  SEMAFORO  ");
  lcd.setCursor(0, 1);
  lcd.print(" INTELIGENTE ");
  delay(2000);
  lcd.clear();
  
  // Iniciar controle do semaforo
  inicializarSemaforo(&controle);
  
  // Iniciar no vermelho
  mudarFase(controle, FASE_VERMELHO);
  
  Serial.println("Sistema pronto!");
}

void loop() {
  // Atualizar estado do semáforo
  atualizarSemaforo(controle);
  
  // Atualizar display LCD sempre
  int tempoRestante = calcularTempoRestante(controle);
  atualizarLCD(controle->faseAtual, tempoRestante);
  
  // Delay de 100ms pra não explodir nosso arduino de processamento
  delay(100);
}

void inicializarSemaforo(ControleSemaforo** ctrl) {
  // Alocar memória para a estrutura usando ponteiro duplo (tive q pesquisar mt complexo)
  *ctrl = (ControleSemaforo*)malloc(sizeof(ControleSemaforo));
  
  // Atribuir os ponteiros pros pinos e controle de tempo
  (*ctrl)->pinVermelho = &pinoVermelho;
  (*ctrl)->pinAmarelo = &pinoAmarelo;
  (*ctrl)->pinVerde = &pinoVerde;
  (*ctrl)->tempoInicio = &tempoInicioFase;
  (*ctrl)->faseAtual = FASE_VERMELHO;
  
  Serial.println("Estrutura de controle inicializada com ponteiros");
}

void atualizarSemaforo(ControleSemaforo* ctrl) {
  unsigned long tempoDecorrido = millis() - *(ctrl->tempoInicio);
  unsigned long duracaoFaseAtual = obterDuracaoFase(ctrl->faseAtual);
  
  // Ver se ja pode mudar de fase
  if (tempoDecorrido >= duracaoFaseAtual) {
    // Lógica de negócio pra alterar pra prox fase
    FaseSemaforo proximaFase;
    
    switch (ctrl->faseAtual) {
      case FASE_VERMELHO:
        proximaFase = FASE_VERDE;
        break;
      case FASE_VERDE:
        proximaFase = FASE_AMARELO;
        break;
      case FASE_AMARELO:
        proximaFase = FASE_VERMELHO;
        break;
    }
    
    // Agora sim mudar para a próxima fase :)
    mudarFase(ctrl, proximaFase);
  }
}

void mudarFase(ControleSemaforo* ctrl, FaseSemaforo novaFase) {
  // Apagar todos os LEDs usando ponteiros para os pinos
  digitalWrite(*(ctrl->pinVermelho), LOW);
  digitalWrite(*(ctrl->pinAmarelo), LOW);
  digitalWrite(*(ctrl->pinVerde), LOW);
  
  // Acender o LED da nova fase
  switch (novaFase) {
    case FASE_VERMELHO:
      digitalWrite(*(ctrl->pinVermelho), HIGH);
      Serial.println(">>> FASE: VERMELHO (6s)");
      break;
      
    case FASE_VERDE:
      digitalWrite(*(ctrl->pinVerde), HIGH);
      Serial.println(">>> FASE: VERDE (4s)");
      break;
      
    case FASE_AMARELO:
      digitalWrite(*(ctrl->pinAmarelo), HIGH);
      Serial.println(">>> FASE: AMARELO (2s)");
      break;
  }
  
  // Atualizar fase e tempo usando ponteiro
  ctrl->faseAtual = novaFase;
  *(ctrl->tempoInicio) = millis();
}

void atualizarLCD(FaseSemaforo fase, int tempoRestante) {
  // Primeira linha: nome da fase
  lcd.setCursor(0, 0);
  lcd.print("                "); // Limpar linha
  lcd.setCursor(0, 0);
  
  switch (fase) {
    case FASE_VERMELHO:
      lcd.print("  VERMELHO  ");
      break;
    case FASE_VERDE:
      lcd.print("   VERDE    ");
      break;
    case FASE_AMARELO:
      lcd.print("  AMARELO   ");
      break;
  }
  
  // Segunda linha: tempo restante
  lcd.setCursor(0, 1);
  lcd.print("                "); // Limpar linha
  lcd.setCursor(0, 1);
  lcd.print("Tempo: ");
  lcd.print(tempoRestante);
  lcd.print("s  ");
}

/**
 * Calcula o tempo restante da fase atual em segundos
 */
int calcularTempoRestante(ControleSemaforo* ctrl) {
  unsigned long tempoDecorrido = millis() - *(ctrl->tempoInicio);
  unsigned long duracaoFase = obterDuracaoFase(ctrl->faseAtual);
  unsigned long tempoRestanteMs = duracaoFase - tempoDecorrido;
  
  return (tempoRestanteMs + 999) / 1000;
}

unsigned long obterDuracaoFase(FaseSemaforo fase) {
  switch (fase) {
    case FASE_VERMELHO:
      return TEMPO_VERMELHO;
    case FASE_VERDE:
      return TEMPO_VERDE;
    case FASE_AMARELO:
      return TEMPO_AMARELO;
    default:
      return TEMPO_VERMELHO;
  }
}