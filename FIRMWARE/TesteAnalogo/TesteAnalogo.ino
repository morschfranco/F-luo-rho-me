/*
Descrição
 Determina o tempo necessário e o erro para analogRead()

PREFERENCIALMENTE DETECT04.SCH

 */

unsigned long Ciclo_Pulso = 1; // Ciclos de pulso
unsigned long duracao_pulso = 50; // Tempo LED ligado
unsigned long Dur_Pulso_Baixo = 9975; // tempo LED desligado
int Pino_Mede_Lux1 = 5;
int detector1 = A0;
unsigned long inicia1, inicia1orig, fim1;
unsigned long duracao_pulsoverif, Dur_Pulso_Baixoverif, Ciclo_Pulsoverif, Tempo_Totalverif;
int Dados1, Dados2, Dados3, Dados4;
float dados1f, dados2f, dados3f, dados4f;
int i = 0; // 

void setup() {


  Serial.begin(38400); // velocidade mínima recomendada
  pinMode(Pino_Mede_Lux1, OUTPUT); // configura pino como saída
  analogReadAveraging(1); // realiza apenas uma medida
  inicia1 = 0;
  inicia1orig = 0;
  fim1 = 0;
  Dados1 = 0;
  Dados2 = 0;
  Dados3 = 0;
  Dados4 = 0;
}

void loop() {

  // METODO 1: 
  // Laço for determina ciclos LIGA/DESLIGA,
  // Laço while realiza a medição do tempo
  digitalWriteFast(Pino_Mede_Lux1, HIGH); //
  delay(3000); // 
  noInterrupts(); // Desabilita interrupções para evitar interferência de outras rotinas

    // Dispara LED no ciclo LIGA/DESLIGA, realiza amostragens a partir de analogRead no ciclo LIGA
  inicia1orig = micros();
  inicia1 = micros();
  for (i=0;i<Ciclo_Pulso;i++) {
    digitalWriteFast(Pino_Mede_Lux1, LOW); // 
    Dados1 = analogRead(detector1); //
    Dados2 = analogRead(detector1);
    Dados3 = analogRead(detector1);
    Dados4 = analogRead(detector1);
    inicia1=inicia1+duracao_pulso;
    while (micros ()<inicia1) {
    }
    //  digitalWriteFast(Pino_Mede_Lux1, LOW); // 
    inicia1=inicia1+Dur_Pulso_Baixo;
    while (micros ()<inicia1) {
    }
  }
  fim1 = micros();

  interrupts();

  // Mede o tepmo de execução, o tempo no ciclo LIGADO E DESLIGADO
  Tempo_Totalverif = fim1 - inicia1orig;

  dados1f = Dados1*3.3/1023; // converte dados
  dados2f = Dados2*3.3/1023; // converte dados
  dados3f = Dados3*3.3/1023; // converte dados
  dados4f = Dados4*3.3/1023; // converte dados

  // RELATÓRIO
  delay(50);

  Serial.print("CICLOS:  ");
  Serial.println(Ciclo_Pulso);

  Serial.print("TEMPO DE EXECUÇÃO:  ");
  Serial.println(Tempo_Totalverif);

  Serial.print("valor de i:  ");
  Serial.println(i);

  // medição LIGA/DESLIGA
  //Serial.print("duração pulso nivel alto:  ");
  //Serial.println(duracao_pulsoverif);
  //Serial.print("pulso nivel baixo:  ");
  //Serial.println(Dur_Pulso_Baixoverif);

  Serial.println("valores médios para cada ciclo LIGADO");
  Serial.println(dados1f);
  Serial.println(dados2f);
  Serial.println(dados3f);
  Serial.println(dados4f);

  delay(50);

  while (1) {
  };
}  

