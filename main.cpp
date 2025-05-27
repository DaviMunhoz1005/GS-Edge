#include <LiquidCrystal_I2C.h>
#include <Wire.h> 

/* DEFINIÇÃO DAS VARIÁVEIS */

// LEDs
#define DANGER_LED 9
#define ALERT_LED 11
#define OK_LED 13

// BUZZER
#define BUZZER 8
unsigned long previousBuzzTime = 0;

// Duração do buzzer em diferentes estados
const int buzzDuration = 500; 
const int buzzIntervalAlert = 3000;
const int buzzIntervalDanger = 500;
bool buzzerOn = false;
unsigned long buzzerStartTime = 0;
bool canBuzz = false;

// SENSOR DE DISTANCIA ULTRASSONICO HC-SR04
#define TRIGGER_SENSOR 3
#define ECHO_SENSOR 2
float distance = 0;
float previousDistance = -1;
// Distância padrão para estado OK ou ALERTA
const float DIST_OK = 150.0;
const float DIST_ALERT = 100.0;

// LCD
#define column 16 
#define line 2 
#define address 0x27 

LiquidCrystal_I2C lcd(address,column,line);
boolean changeDisplay;

// ENUM DO ESTADO DO SISTEMA
enum State {
    OK = 0,
    ALERT = 1,
    DANGER = 2
};

State state = ALERT;

// Definição do que cada pino do arduino vai fazer no sistema
void setup() {
    Serial.begin(9600);

    pinMode(DANGER_LED, OUTPUT);
    pinMode(ALERT_LED, OUTPUT);
    pinMode(OK_LED, OUTPUT);

    pinMode(TRIGGER_SENSOR, OUTPUT);
    pinMode(ECHO_SENSOR, INPUT);
    
    pinMode(BUZZER, OUTPUT);

    lcd.init(); 
    lcd.backlight(); 
    lcd.clear();
}

void loop() {
    takeDistanceSensor();

    // Checa se o estado atual modificou, se sim ele troca o display
    State previousState = state;
    checkState(); 

    // Checa se o display mudou os valores
    if (previousState != state || abs(distance - previousDistance) >= 0.1) {
        changeDisplay = true;
    }

    // Liga os LEDs e faz o display
    ledsOnOff();
    if (changeDisplay) {
      if (previousState != state) {
          switch (state) {
          case OK:
              displayState("Estado = OK");
              canBuzz = false;
              break;
          case ALERT:
              displayState("Estado = ALERTA");
              canBuzz = true;
              break;
          case DANGER:
              displayState("Estado = PERIGO");
              canBuzz = true;
              break;
          }
      }
      displayDistance(distance);
      changeDisplay = false;
    }
}

// Função que mede a distância usando o sensor ultrassônico HC-SR04
void takeDistanceSensor() {
    digitalWrite(TRIGGER_SENSOR, LOW);
    delayMicroseconds(5);        
    digitalWrite(TRIGGER_SENSOR, HIGH); // Lança o pulso ultrassônico
    delayMicroseconds(10);      
    digitalWrite(TRIGGER_SENSOR, LOW); 
    
    distance = pulseIn(ECHO_SENSOR, HIGH); // Recebe o retorno do pulso ultrassônico  
    distance = distance / 58; 

    // Printa no Serial Monitor o valor da Distância
    Serial.print("Distancia = ");
    Serial.print(distance);         
    Serial.println(" cm");
}

// Checa e atualiza o estado do sistema
void checkState() {
    if (distance > DIST_OK) {
        state = OK;
    } else if (distance <= DIST_OK && distance > DIST_ALERT) {
        state = ALERT;
    } else {
        state = DANGER;
    }
}

// Liga e desliga os LEDs de acordo com o estado atual do sistema
void ledsOnOff() {
    digitalWrite(OK_LED, state == OK);
    digitalWrite(ALERT_LED, state == ALERT);
    digitalWrite(DANGER_LED, state == DANGER);
}

// Display que manda para o LCD_I2C
void displayTwoLines(String line1, String line2) {
    lcd.setCursor(0, 0);
    lcd.print(line1);
    lcd.setCursor(0, 1);
    lcd.print(line2);
}

// Display que manda para o LCD_I2C na primeira linha
void displayState(String line1) {
    lcd.setCursor(0, 0);
    lcd.print("                ");  // Limpa a linha
    lcd.setCursor(0, 0);
    lcd.print(line1);
}

// Display que manda para o LCD_I2C na segunda linha
void displayDistance(float dist) {
    lcd.setCursor(0, 1);
    lcd.print("                ");  // Limpa a linha
    lcd.setCursor(0, 1);
    lcd.print("Dist = " + String(dist, 1) + "cm");
    delay(500);
}

// Lida com a forma que o BUZZER vai ser ativo de acordo com o estado do sistema
void handleBuzzer() {
    // Checa se pode ativar o BUZZER ou se o estado está em OK
    if (!canBuzz || state == OK) {
        noTone(BUZZER);
        return;
    }
    // Pega o intervalo correspondente do state do sistema
    int interval = (state == ALERT) ? buzzIntervalAlert : buzzIntervalDanger;

    // Liga o buzzer pelo intervalo com a função millis() para não parar o sistema
    if (!buzzerOn && millis() - previousBuzzTime >= interval) {
        tone(BUZZER, 1500);
        buzzerStartTime = millis();
        buzzerOn = true;
    }

    // Desliga o buzzer pelo intervalo com a função millis() para não parar o sistema
    if (buzzerOn && millis() - buzzerStartTime >= buzzDuration) {
        noTone(BUZZER);
        buzzerOn = false;
        previousBuzzTime = millis();
    }
}