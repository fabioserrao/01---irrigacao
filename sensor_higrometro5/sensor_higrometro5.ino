// ================= CONFIGURAÇÃO DE PINOS (ARDUINO UNO) =================
const uint8_t PIN_SENSOR = A0;
const uint8_t PIN_RELE   = 7;

// ================= LÓGICA DO MÓDULO RELÉ =================
// A maioria dos módulos relé comerciais é ATIVO EM NÍVEL BAIXO (LOW liga o relé).
// Se o seu relé ligar com HIGH, inverta os valores abaixo.
const uint8_t RELE_LIGADO    = LOW;
const uint8_t RELE_DESLIGADO = HIGH;

// ================= CALIBRAÇÃO DO SENSOR =================
const int ADC_SECO  = 1015;
const int ADC_UMIDO = 310;

// ================= PARÂMETROS DE IRRIGAÇÃO (HISTERESE) =================
const int UMIDADE_LIGAR    = 30; // Abre a solenoide se cair abaixo de 30%
const int UMIDADE_DESLIGAR = 55; // Fecha a solenoide ao atingir 55%

// ================= SEGURANÇA E TEMPORIZAÇÃO =================
const uint8_t NUM_AMOSTRAS             = 10;
const unsigned long INTERVALO_LEITURA  = 1000;   // Lê o solo a cada 1 segundo
const unsigned long TEMPO_MAX_REGA_MS  = 6000;  // Máximo 60s ligada seguida (proteção)

unsigned long ultimoTempoLeitura = 0;
unsigned long inicioRegaMs       = 0;
bool solenoideAberta             = false;
bool bloqueioSeguranca           = false;

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

  soma -= (minimo + maximo);
  return soma / (NUM_AMOSTRAS - 2);
}

int calcularUmidade(int valorBruto) {
  int pct = map(valorBruto, ADC_SECO, ADC_UMIDO, 0, 100);
  return constrain(pct, 0, 100);
}

void acionarSolenoide(bool ligar) {
  if (ligar && !solenoideAberta) {
    digitalWrite(PIN_RELE, RELE_LIGADO);
    solenoideAberta = true;
    inicioRegaMs = millis();
  } else if (!ligar && solenoideAberta) {
    digitalWrite(PIN_RELE, RELE_DESLIGADO);
    solenoideAberta = false;
  }
}

void setup() {
  // Garante que o relé comece desligado antes mesmo de definir como OUTPUT
  digitalWrite(PIN_RELE, RELE_DESLIGADO);
  pinMode(PIN_RELE, OUTPUT);

  Serial.begin(9600);
  Serial.println(F("=== SISTEMA DE IRRIGACAO - ARDUINO UNO ==="));
  Serial.println(F("Bruto\tUmidade\tSolenoide\tStatus"));
}

void loop() {
  unsigned long agora = millis();

  // Proteção contra tempo máximo de válvula aberta
  if (solenoideAberta && (agora - inicioRegaMs >= TEMPO_MAX_REGA_MS)) {
   //acionarSolenoide(false);
    //bloqueioSeguranca = true;
    //Serial.println(F("ALERTA: Tempo maximo de rega atingido! Valvula fechada."));
  }

  if (agora - ultimoTempoLeitura >= INTERVALO_LEITURA) {
    ultimoTempoLeitura = agora;

    int bruto = lerSensorFiltrado();
    int umidade = calcularUmidade(bruto);

    // Falha de leitura (fio solto ou curto): fecha a válvula imediatamente
    if (bruto <= 10 || bruto >= 1022) {
      acionarSolenoide(false);
      Serial.print(bruto);
      Serial.println(F("\t--\tFECHADA\t\tERRO_SENSOR"));
      return;
    }

    // Controle por histerese (se não estiver em bloqueio de segurança)
    if (!bloqueioSeguranca) {
      if (!solenoideAberta && umidade < UMIDADE_LIGAR) {
        acionarSolenoide(true);
      } else if (solenoideAberta && umidade >= UMIDADE_DESLIGAR) {
        acionarSolenoide(false);
      }
    }

    Serial.print(bruto);
    Serial.print(F("\t"));
    Serial.print(umidade);
    Serial.print(F("%\t"));
    Serial.print(solenoideAberta ? F("ABERTA") : F("FECHADA"));
    Serial.print(F("\t\t"));
    Serial.println(bloqueioSeguranca ? F("BLOQUEIO_TEMPO") : F("OK"));
  }
}
