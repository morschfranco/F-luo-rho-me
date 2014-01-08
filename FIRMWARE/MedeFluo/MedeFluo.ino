/*
DESCRIÇÃO:
 mede fluorescência a partir de pulso de saturação
PREFERENCIALMENTE DETECT04.SCH

 */

unsigned long Ciclo_Pulso = 150; // Número de ciclos
unsigned long Satura_Ciclo_Alto = 10; //(ajuste = 0 para não-saturação)
unsigned long Satura_Ciclo_Baixo = 30; //tempo de LED desligado
unsigned long duracao_pulso = 25; // Tempo de pulso LED em us (mínimo =5 us, obtido de ~4us analogRead +/- 5us para cada analogRead adicional).
unsigned long Dur_Pulso_Baixo = 49975; // PulsO LED desligado 
int Pino_Mede_Lux1 = 4;
int Pino_Sat_Lux1 = 3;
int detector1 = A0;
unsigned long inicia1, inicia1orig, fim1;
unsigned long duracao_pulsoverif, Dur_Pulso_Baixoverif, Ciclo_Pulsoverif, Tempo_Totalverif;
int Dados1, Dados2, Dados3, Dados4, Valor_Medio;
float dados1f, dados2f, dados3f, dados4f;
int i = 0; // 
String data = 0;

void setup() {

  Serial.begin(38400); // 
  pinMode(Pino_Mede_Lux1, OUTPUT); // configura pino como saída
  pinMode(Pino_Sat_Lux1, OUTPUT); // configura pino como saída
  analogReadAveraging(1); // 1 medida
  inicia1 = 0;
  inicia1orig = 0;
  fim1 = 0;
  Dados1 = 0;
  Dados2 = 0;
  Dados3 = 0;
  Dados4 = 0;
}

void loop() {

  delay(3000); // 
  noInterrupts(); // Desabilita interrupções para evitar interferência de outras rotinas

    // Dispara LED no ciclo LIGA/DESLIGA, realiza amostragens a partir de analogRead no ciclo LIGA
  inicia1orig = micros();
  inicia1 = micros();
  for (i=0;i<Ciclo_Pulso;i++) {
    digitalWriteFast(Pino_Mede_Lux1, HIGH); //
    if (Satura_Ciclo_Alto == i+1) {
      digitalWriteFast(Pino_Sat_Lux1, HIGH); //
    }
    Dados1 = analogRead(detector1); // 
    Dados2 = analogRead(detector1);
    Dados3 = analogRead(detector1);
    Dados4 = analogRead(detector1);
    inicia1=inicia1+duracao_pulso;
    while (micros ()<inicia1) {
    }
    inicia1=inicia1+Dur_Pulso_Baixo;
    digitalWriteFast(Pino_Mede_Lux1, LOW); // média é feita quando o LED está desligado
    if (Satura_Ciclo_Baixo == i+1) {
      digitalWriteFast(Pino_Sat_Lux1, LOW); //
    }
    Valor_Medio = (Dados1+Dados2+Dados3+Dados4)/4; // 
    data += Valor_Medio;
    data += ",";
    while (micros ()<inicia1) {
    }
  }
  fim1 = micros();

  interrupts();

  // determina tempo de execução
  Tempo_Totalverif = fim1 - inicia1orig;

  dados1f = Dados1*3.3/1023; // 
  dados2f = Dados2*3.3/1023; // 
  dados3f = Dados3*3.3/1023; // 
  dados4f = Dados4*3.3/1023; // 

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

