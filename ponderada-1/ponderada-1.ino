const int LED_VERMELHO = 9;
const int LED_AMARELO = 10;
const int LED_VERDE = 11;

unsigned long previousMillis = 0;
unsigned long trafficMillis = 0;
const long blinkInterval = 500;

bool builtinState = LOW;
int trafficState = 0; // 0 = vermelho, 1 = amarelo, 2 = verde

void setup() {
    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(LED_VERMELHO, OUTPUT);
    pinMode(LED_AMARELO, OUTPUT);
    pinMode(LED_VERDE, OUTPUT);
}

void loop() {
    unsigned long currentMillis = millis();
    
    // Blink do LED interno, ciclo de 0.5 segundos
    if (currentMillis - previousMillis >= blinkInterval) {
        previousMillis = currentMillis;
        builtinState = !builtinState;
        digitalWrite(LED_BUILTIN, builtinState);
    }
    
    // Ciclo de 3 segundos para o semáforo
    if (currentMillis - trafficMillis >= 3000) {
        trafficMillis = currentMillis;
        
        // Apaga todos os LEDs
        digitalWrite(LED_VERMELHO, LOW);
        digitalWrite(LED_AMARELO, LOW);
        digitalWrite(LED_VERDE, LOW);
        
        // Ciclo do semáforo
        trafficState = (trafficState + 1) % 3;
        
        // Acende o LED correspondente ao estado atual
        switch(trafficState) {
            case 0:  // Vermelho
                digitalWrite(LED_VERMELHO, HIGH);
                break;
            case 1:  // Amarelo
                digitalWrite(LED_AMARELO, HIGH);
                break;
            case 2:  // Verde
                digitalWrite(LED_VERDE, HIGH);
                break;
        }
    }
}