#include <WiFi.h>
#include "esp_event.h"
#include "mqtt_client.h"
#include "UbidotsEsp32Mqtt.h"

// =====================================================================
// CONFIG WiFi
// =====================================================================
const char* ssid     = "Inteli.Iot";
const char* password = "";

// =====================================================================
// UBIDOTS
// =====================================================================
const char *UBI_TOKEN   = "BBUS-";
const char *DEVICE_LABEL = "esp32_davi";
const char *CLIENT_ID    = "davi_cross";

// Variáveis
const char *VAR_NIGHT     = "night_mode";
const char *VAR_FORCE     = "force_green";
const char *VAR_LDR1      = "ldr_1";
const char *VAR_ACTIVE    = "active_novo";

Ubidots ubidots(UBI_TOKEN, CLIENT_ID);

// =====================================================================
// AWS CONFIG
// =====================================================================
const char* TOPIC_AWS = "ponderada";
const char* CLIENT_ID_AWS = "esp_test";

// Seus certificados PEM (mesmos que enviou)
static const char AWS_CERT_CA[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
-----END CERTIFICATE-----
)EOF";

static const char AWS_CERT_CRT[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
-----END CERTIFICATE-----
)EOF";

static const char AWS_CERT_PRIVATE[] PROGMEM = R"EOF(
-----BEGIN RSA PRIVATE KEY-----
-----END RSA PRIVATE KEY-----
)EOF";

esp_mqtt_client_handle_t clientAWS;

// =====================================================================
// PINAGEM – 4 SEMÁFOROS
// =====================================================================
struct Light {
  int r, y, g;
};

Light S[4] = {
  {2, 15, 0},   // S1
  {18, 5, 19},  // S2
  {27, 26, 25}, // S3
  {23, 13, 22}  // S4
};

#define LDR1 33
#define BTN_NIGHT 4

// =====================================================================
// VARIÁVEIS DE CONTROLE
// =====================================================================
bool nightMode = false;
bool forceMode = false;  // Indica se há comando manual ativo
int activeGreen = 1;     // Qual semáforo está verde (1-4)

unsigned long lastChange = 0;
unsigned long greenTime = 5000;    // 5 segundos no verde
unsigned long yellowTime = 2000;   // 2 segundos no amarelo

enum TrafficState { GREEN, YELLOW };
TrafficState currentState = GREEN;

// =====================================================================
// FUNÇÕES DE CONTROLE
// =====================================================================
void setAllOff() {
  for (int i=0; i<4; i++) {
    digitalWrite(S[i].r, LOW);
    digitalWrite(S[i].y, LOW);
    digitalWrite(S[i].g, LOW);
  }
}

void setTrafficLight(int id, TrafficState state) {
  int idx = id - 1;
  digitalWrite(S[idx].r, state == GREEN ? LOW : HIGH);  // Vermelho quando não está verde
  digitalWrite(S[idx].y, state == YELLOW ? HIGH : LOW);
  digitalWrite(S[idx].g, state == GREEN ? HIGH : LOW);
}

void setGreenOnly(int id) {
  if (activeGreen == id && currentState == GREEN) return;

  // TODOS os semáforos em VERMELHO
  for (int i=0; i<4; i++) {
    digitalWrite(S[i].r, HIGH);
    digitalWrite(S[i].y, LOW);
    digitalWrite(S[i].g, LOW);
  }
  
  // Apenas o semáforo selecionado fica VERDE
  int idx = id - 1;
  digitalWrite(S[idx].r, LOW);
  digitalWrite(S[idx].g, HIGH);
  
  activeGreen = id;
  currentState = GREEN;
  lastChange = millis();
  
  Serial.printf("Semáforo S%d -> VERDE | Demais -> VERMELHO\n", id);
}

void blinkYellowAll() {
  static unsigned long lastBlink = 0;
  static bool state = false;
  
  if (millis() - lastBlink > 500) {
    state = !state;
    for (int i=0; i<4; i++) {
      digitalWrite(S[i].r, LOW);
      digitalWrite(S[i].g, LOW);
      digitalWrite(S[i].y, state ? HIGH : LOW);
    }
    lastBlink = millis();
  }
}

// =====================================================================
// LÓGICA AUTOMÁTICA DO SEMÁFORO DE CRUZAMENTO
// =====================================================================
void updateTrafficLights() {
  unsigned long now = millis();
  unsigned long elapsed = now - lastChange;

  switch(currentState) {
    case GREEN:
      // Após o tempo de verde, passa para AMARELO
      if (elapsed >= greenTime) {
        // Mantém todos vermelhos, exceto o atual que vai para amarelo
        for (int i=0; i<4; i++) {
          if (i == activeGreen - 1) {
            digitalWrite(S[i].r, LOW);
            digitalWrite(S[i].y, HIGH);
            digitalWrite(S[i].g, LOW);
          } else {
            digitalWrite(S[i].r, HIGH);
            digitalWrite(S[i].y, LOW);
            digitalWrite(S[i].g, LOW);
          }
        }
        
        currentState = YELLOW;
        lastChange = now;
        Serial.printf("S%d -> AMARELO | Demais -> VERMELHO\n", activeGreen);
      }
      break;

    case YELLOW:
      // Após o amarelo, passa para o próximo semáforo
      if (elapsed >= yellowTime) {
        // Avança para o próximo semáforo
        activeGreen++;
        if (activeGreen > 4) activeGreen = 1;
        
        // Coloca o próximo em VERDE e todos outros em VERMELHO
        setGreenOnly(activeGreen);
      }
      break;
  }
}

// =====================================================================
// UBIDOTS – CALLBACK DE COMANDO
// =====================================================================
void callback(char *topic, byte *payload, unsigned int length){
  String msg = "";
  for (int i = 0; i < length; i++) msg += (char)payload[i];

  Serial.print("UBIDOTS: ");
  Serial.print(topic);
  Serial.print(" = ");
  Serial.println(msg);

  String newVarNight = "night_mode/lv";
  String newVarForce = "force_green/lv";

  if (String(topic).endsWith(newVarNight)) {
    nightMode = msg.toInt() == 1;
    forceMode = false;  // Cancela modo forçado se houver
    Serial.printf("Modo Noturno: %s\n", nightMode ? "ATIVADO" : "DESATIVADO");
  }

  if (String(topic).endsWith(newVarForce)) {
    int p = msg.toInt();
    if (p >= 1 && p <= 4) {
      nightMode = false;  // Desativa modo noturno
      forceMode = true;   // Ativa modo forçado
      setGreenOnly(p);
      Serial.printf("Comando Manual: S%d forçado em VERDE\n", p);
    } else if (p == 0) {
      // Valor 0 desativa o modo forçado e volta ao automático
      forceMode = false;
      Serial.println("Modo Forçado DESATIVADO - Voltando ao automático");
    }
  }
}

// =====================================================================
// AWS – EVENTO MQTT
// =====================================================================
static esp_err_t mqtt_event_handler_cb(esp_mqtt_event_handle_t event) {
  
  switch (event->event_id) {
    case MQTT_EVENT_CONNECTED:
      Serial.println("✅ AWS conectado!");
      esp_mqtt_client_subscribe(clientAWS, TOPIC_AWS, 1);
      break;

    case MQTT_EVENT_DISCONNECTED:
      Serial.println("❌ AWS desconectado");
      break;

    case MQTT_EVENT_DATA: {
      String payload = "";
      for (int i=0; i<event->data_len; i++) payload += (char)event->data[i];

      Serial.print("CMD AWS: ");
      Serial.println(payload);

      if (payload.indexOf("night_mode") >= 0) {
        nightMode = payload.indexOf("1") >= 0;
        forceMode = false;
        Serial.printf("AWS: Modo Noturno %s\n", nightMode ? "ON" : "OFF");
      }

      if (payload.indexOf("set_green") >= 0) {
        int g = payload.substring(payload.indexOf(":")+1).toInt();
        if (g >= 1 && g <= 4) {
          nightMode = false;
          forceMode = true;
          setGreenOnly(g);
          Serial.printf("AWS: S%d forçado\n", g);
        } else if (g == 0) {
          forceMode = false;
          Serial.println("AWS: Modo automático restaurado");
        }
      }
      break;
    }

    case MQTT_EVENT_ERROR:
      Serial.println("❌ Erro AWS MQTT");
      break;
  }

  return ESP_OK;
}

static void mqtt_event_handler(void *handler_args, esp_event_base_t base,
                               int32_t event_id, void *event_data) {
    mqtt_event_handler_cb((esp_mqtt_event_handle_t) event_data);
}

// =====================================================================
// SETUP AWS
// =====================================================================
void initAWS() {
    esp_mqtt_client_config_t mqtt_cfg = {};
    mqtt_cfg.broker.address.uri = "mqtts://xxxxxxx-ats.iot.us-east-2.amazonaws.com:8883";

    mqtt_cfg.broker.verification.certificate = AWS_CERT_CA;
    mqtt_cfg.credentials.client_id = CLIENT_ID_AWS;
    mqtt_cfg.credentials.authentication.certificate = AWS_CERT_CRT;
    mqtt_cfg.credentials.authentication.key = AWS_CERT_PRIVATE;

    mqtt_cfg.session.protocol_ver = MQTT_PROTOCOL_V_3_1_1;

    clientAWS = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(clientAWS, MQTT_EVENT_ANY, mqtt_event_handler, clientAWS);
    esp_mqtt_client_start(clientAWS);
}

// =====================================================================
// SETUP
// =====================================================================
void setup() {
  Serial.begin(115200);
  delay(1000);

  pinMode(BTN_NIGHT, INPUT_PULLUP);
  pinMode(LDR1, INPUT);

  Serial.println("\n=== Inicializando Semáforos ===");
  for (int i=0; i<4; i++) {
    Serial.printf("S%d -> R:%d Y:%d G:%d\n", i+1, S[i].r, S[i].y, S[i].g);
    pinMode(S[i].r, OUTPUT);
    pinMode(S[i].y, OUTPUT);
    pinMode(S[i].g, OUTPUT);
  }

  // TESTE: Acende todos os LEDs por 2 segundos
  Serial.println("\n=== TESTE DE LEDs ===");
  for (int i=0; i<4; i++) {
    digitalWrite(S[i].r, HIGH);
    digitalWrite(S[i].y, HIGH);
    digitalWrite(S[i].g, HIGH);
  }
  delay(2000);
  setAllOff();
  Serial.println("Teste concluído!\n");

  WiFi.begin(ssid, password);
  Serial.print("Conectando WiFi");
  while (WiFi.status() != WL_CONNECTED) { 
    delay(300); 
    Serial.print("."); 
  }
  Serial.println(" ✅ conectado!");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());

  configTime(0, 0, "pool.ntp.org", "time.nist.gov");
  Serial.print("Aguardando sync NTP");
  time_t now = 0;
  while (now < 8 * 3600 * 2) { // espera até ter um horário razoável
    Serial.print(".");
    delay(500);
    time(&now);
  }
  Serial.println("\nHora sincronizada com NTP");
  
  initAWS();

  ubidots.connectToWifi(ssid, password);
  ubidots.setCallback(callback);
  ubidots.setup();
  ubidots.reconnect();

  ubidots.subscribeLastValue(DEVICE_LABEL, VAR_NIGHT);
  ubidots.subscribeLastValue(DEVICE_LABEL, VAR_FORCE);

  setGreenOnly(1);
  Serial.println("\n=== Sistema Iniciado ===\n");
}

// =====================================================================
// LOOP
// =====================================================================
unsigned long lastSend = 0;
unsigned long lastBtnCheck = 0;
static int lastActiveSent = -1;

void loop() {
  ubidots.loop();
  yield();

  if (!ubidots.connected()) {
    Serial.println("Reconectando Ubidots...");
    ubidots.reconnect();
  }

  // BOTÃO — Alterna Modo Noturno (com debounce)
  if (millis() - lastBtnCheck > 200) {
    if (digitalRead(BTN_NIGHT) == LOW) {
      nightMode = !nightMode;
      forceMode = false;  // Cancela qualquer comando manual
      Serial.printf("🔘 Botão: Modo Noturno %s\n", nightMode ? "ATIVADO" : "DESATIVADO");
      lastBtnCheck = millis();
      delay(300);
    }
  }

  // ============================================
  // LEITURA DOS LDRs
  // ============================================
  int v1 = analogRead(LDR1);

  // Ajuste esse limiar conforme o seu hardware (0–4095).
  // Quanto MENOR o valor, mais escuro (depende de como está montado o divisor).
  const int LDR_THRESHOLD = 1500;

  bool dark1 = v1 > LDR_THRESHOLD;  // baixa luminosidade no LDR1

  // LÓGICA POR LDR TEM PRIORIDADE SOBRE AUTOMÁTICO
  // Mantém o semáforo correspondente no VERDE enquanto estiver escuro
  if (dark1) {
    // LDR1 controla S2
    delay(1000);
    setGreenOnly(2);
    // opcional: cancelar modos
    nightMode = false;
  } else {
    // Quando nenhum LDR está em baixa luminosidade, sai do modo "forçado por LDR"
    // (se quiser que comandos manuais da AWS/Ubidots continuem funcionando,
    //  não zere forceMode aqui; ou use um flag separado, ex: ldrOverrideMode)

    // ============================================
    // CONTROLE DOS SEMÁFOROS
    // ============================================
    if (nightMode) {
      blinkYellowAll();     // MODO NOTURNO
    } 
    else if (forceMode) {
      // MODO FORÇADO (comando manual via AWS/Ubidots): não altera nada
    } 
    else {
      updateTrafficLights(); // MODO AUTOMÁTICO
    }
  }

  // ENVIO PERIÓDICO UBIDOTS + AWS
  if (millis() - lastSend > 4500) {  // A cada ~4,5 segundos
    // Já temos v1 e v2 lidos acima; reaproveita:
    ubidots.add(VAR_LDR1, v1);
    ubidots.add(VAR_NIGHT, nightMode ? 1 : 0);
    
    if (activeGreen != lastActiveSent) {
      lastActiveSent = activeGreen;
      ubidots.add(VAR_ACTIVE, activeGreen);
    }

    ubidots.publish(DEVICE_LABEL);
    Serial.printf("📤 Ubidots: S%d ativo : NightMode: %d | LDR1=%d \n",
                  activeGreen, nightMode, v1);

    // AWS JSON
    String json = "{";
    json += "\"night\":" + String(nightMode ? 1 : 0) + ",";
    json += "\"force_mode\":" + String(forceMode ? 1 : 0) + ",";
    json += "\"active_green\":" + String(activeGreen) + ",";
    json += "\"state\":\"" + String(currentState == GREEN ? "GREEN" : "YELLOW") + "\",";
    json += "\"ldr1\":" + String(v1);
    json += "}";

    int msg_id = esp_mqtt_client_publish(clientAWS, TOPIC_AWS, json.c_str(), 0, 1, 0);
    Serial.printf("📤 AWS (msg_id=%d): %s\n", msg_id, json.c_str());

    lastSend = millis();
  }
}