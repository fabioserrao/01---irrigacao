// ================= CONFIGURAÇÃO ARDUINO UNO =================
const uint8_t PIN_SENSOR = A0;

// Valores típicos no Arduino Uno (10 bits: 0 a 1023)
// Ajuste após testar sua sonda no ar (seco) e na água (úmido)
const int ADC_SECO  = 1015; 
const int ADC_UMIDO = 310;  

const uint8_t NUM_AMOSTRAS       = 10;
const unsigned long INTERVALO_MS  = 1000; // Atualiza a cada 1 segundo

unsigned long ultimoTempo = 0;

int lerSensorFiltrado() {
  long soma = 0;
  int minimo = 1024;
  int maximo = -1;

  for (uint8_t i = 0; i < NUM_AMOSTRAS; i++) {
    int leitura = analogRead(PIN_SENSOR);
    soma += leitura;
    if (leitura < minimo) minimo = leitura;
    if (leitura > maximo) maximo = leitura;
    delay(2);
  }

  // Descarta os 2 extremos (menor e maior) para estabilizar a leitura
  soma -= (minimo + maximo);
  return soma / (NUM_AMOSTRAS - 2);
}

int calcularUmidade(int valorBruto) {
  int pct = map(valorBruto, ADC_SECO, ADC_UMIDO, 0, 100);
  return constrain(pct, 0, 100);
}

void setup() {
  Serial.begin(9600); // Padrão mais comum no Monitor Serial do Arduino IDE
  Serial.println(F("Leitura HW-103 - Arduino Uno"));
  Serial.println(F("Bruto\tUmidade\tEstado"));
}

void loop() {
  unsigned long agora = millis();

  if (agora - ultimoTempo >= INTERVALO_MS) {
    ultimoTempo = agora;

    int bruto = lerSensorFiltrado();
    int umidade = calcularUmidade(bruto);

    Serial.print(bruto);
    Serial.print(F("\t"));
    Serial.print(umidade);
    Serial.print(F("%\t"));

    if (bruto <= 10) {
      Serial.println(F("ERRO: Curto ou fio solto"));
    } else if (umidade < 30) {
      Serial.println(F("Solo Seco"));
    } else if (umidade < 70) {
      Serial.println(F("Umidade Ideal"));
    } else {
      Serial.println(F("Solo Encharcado"));
    }
  }
}
