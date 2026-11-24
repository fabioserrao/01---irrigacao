#include <Arduino.h>

// ================= CONFIGURAÇÃO DE HARDWARE =================
// Para Arduino Uno/Nano: A0 e pino digital 7
// Para ESP32: use um pino ADC1 (ex.: GPIO 34) e um GPIO de saída (ex.: GPIO 25)
constexpr uint8_t PIN_SENSOR_ANALOG = A1;
constexpr uint8_t PIN_SENSOR_POWER  = 7;

// ================= CALIBRAÇÃO (AJUSTE NA BANCADA) =================
// Leia o valor bruto com a sonda totalmente seca ao ar e submersa em água
constexpr int ADC_SECO    = 1023; // Ex.: ~1023 no Arduino (10 bits) ou ~4095 no ESP32 (12 bits)
constexpr int ADC_UMIDO   = 320;  // Ex.: ~300-400 no Arduino ou ~1200-1600 no ESP32

// ================= PARÂMETROS DE LEITURA =================
constexpr uint8_t NUM_AMOSTRAS          = 10;
constexpr unsigned long INTERVALO_MS     = 5000; // Leitura a cada 5 segundos
constexpr unsigned long TEMPO_ESTAB_MS   = 20;   // Tempo para estabilizar tensão após ligar

unsigned long ultimoTempoLeitura = 0;

int lerSensorFiltrado() {
  // 1. Energiza o módulo apenas para a medição
  digitalWrite(PIN_SENSOR_POWER, HIGH);
  delay(TEMPO_ESTAB_MS);

  long soma = 0;
  int minimo = 32767;
  int maximo = -1;

  for (uint8_t i = 0; i < NUM_AMOSTRAS; i++) {
    int leitura = analogRead(PIN_SENSOR_ANALOG);
    soma += leitura;
    if (leitura < minimo) minimo = leitura;
    if (leitura > maximo) maximo = leitura;
    delay(2);
  }

  // 2. Desliga o módulo imediatamente para evitar eletrólise/oxidação
  digitalWrite(PIN_SENSOR_POWER, LOW);

  // Descarta os extremos (mínimo e máximo) para reduzir ruído
  if (NUM_AMOSTRAS > 2) {
    soma -= (minimo + maximo);
    return soma / (NUM_AMOSTRAS - 2);
  }
  return soma / NUM_AMOSTRAS;
}

float calcularUmidadePercentual(int valorBruto) {
  // Mapeia inversamente (quanto menor a resistência/tensão, maior a umidade)
  long percentual = map(valorBruto, ADC_SECO, ADC_UMIDO, 0, 100);
  return constrain(percentual, 0, 100);
}

void setup() {
  Serial.begin(115200);
  pinMode(PIN_SENSOR_POWER, OUTPUT);
  digitalWrite(PIN_SENSOR_POWER, LOW);

  // Descomente a linha abaixo se estiver usando ESP32 para garantir resolução de 12 bits
  // analogReadResolution(12);

  Serial.println(F("=== INICIANDO MONITORAMENTO PROFISSIONAL HW-103 ==="));
  Serial.println(F("Tempo(ms)\tADC_Bruto\tUmidade(%)\tStatus"));
}

void loop() {
  unsigned long agora = millis();

  if (agora - ultimoTempoLeitura >= INTERVALO_MS) {
    ultimoTempoLeitura = agora;

    int adcBruto = lerSensorFiltrado();
    float umidade = calcularUmidadePercentual(adcBruto);

    // Diagnóstico simples de falha de conexão (fio solto ou curto)
    const char* status = "OK";
    if (adcBruto >= ADC_SECO + 50 || adcBruto <= 5) {
      status = "ALERTA_SONDA";
    }

    Serial.print(agora);
    Serial.print(F("\t\t"));
    Serial.print(adcBruto);
    Serial.print(F("\t\t"));
    Serial.print(umidade, 1);
    Serial.print(F("%\t\t"));
    Serial.println(status);
  }
}
