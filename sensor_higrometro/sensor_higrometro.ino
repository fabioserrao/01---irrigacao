void setup() {
  pinMode(A1, INPUT);  // Define o pino analógico 1 como entrada de dados
  Serial.begin(9600);  // Inicia o monitor Serial
}

void loop() {
  int sensor = analogRead(A1);  // Cria uuma váriavel do tipo inteira para registrar o valor de leitura do pino analógico 1 que recebe o sinal do sensor
  Serial.println(sensor);       // Escreve o valor da váriavel sensor no monitor serial pulando uma linha
  delay(1000);                  // Aguarda 1 segundo para realizar a leitura novamente
}