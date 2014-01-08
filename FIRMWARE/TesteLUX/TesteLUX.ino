/* teste de saturação/medição

PREFERENCIALMENTE DETECT04.SCH
 
 */

int Pino_Mede_Lux1 = 15;
int Pino_Mede_Lux2 = 16;
int Pino_Mede_Lux_pwm = 23;
int Pino_Sat_Lux1 = 20;
int Pino_Sat_Lux1_intensidade2 = 3;
int Pino_Sat_Lux1_intensidade1 = 4;
int Pino_Sat_Lux1_intensidade_comuta = 5;
int Pino_Cal_Lux1 = 14;
int Pino_Cal_Lux1_pwm = 9;
int detector1 = A10;
int Resolucao_AD = 16;
int Resolucao_ADvalor;
int i;
int dados0;
unsigned long inicia1;
unsigned long duracao_pulso = 25; //  us
unsigned long duracao_ciclo = 10; //  ms
unsigned long ciclos = 200;
int aguarda = 20;
int pwmalto = 255;
int pwmmed = 100;
int pwmbaixo = 1;
int x;

void setup() {
  delay(500);
  Serial.begin(115200); // Comunicaçao PC

  pinMode(Pino_Mede_Lux1, OUTPUT); // configura pino como saída
  pinMode(Pino_Mede_Lux2, OUTPUT); // configura pino como saída
  pinMode(Pino_Mede_Lux_pwm, OUTPUT); // configura pino como saída  
  pinMode(Pino_Sat_Lux1, OUTPUT); // configura pino como saída
  pinMode(Pino_Sat_Lux1_intensidade2, OUTPUT); // configura pino como saída
  pinMode(Pino_Sat_Lux1_intensidade1, OUTPUT); // configura pino como saída
  pinMode(Pino_Sat_Lux1_intensidade_comuta, OUTPUT); // configura pino como saída
  pinMode(Pino_Cal_Lux1, OUTPUT); // configura pino como saída
  pinMode(Pino_Cal_Lux1_pwm, OUTPUT); // configura pino como saída  
  //  pinMode(Luz_Actinica1, OUTPUT); // configura pino como saída
  analogReadAveraging(1); // Média ADC = 1(apenas uma medição, demora ~3 us)
  pinMode(detector1, INTERNAL);
  analogReadRes(Resolucao_AD);
  Resolucao_ADvalor = pow(2, Resolucao_AD); // determina o valor máximo lido a partir da resolução
}


void loop() {
  // testsat();
  testalux();
}

void testsats() {

  digitalWriteFast(Pino_Sat_Lux1_intensidade_comuta, LOW); // intensidade 1

  for (x=0;x<255;x++) {
    analogWrite(Pino_Sat_Lux1_intensidade1, x); // intensidade sat
    analogWrite(Pino_Sat_Lux1_intensidade2, x); // intensidade sat
    delay(15);
    digitalWriteFast(Pino_Sat_Lux1, HIGH);
    delay(1);
    digitalWriteFast(Pino_Sat_Lux1, LOW);
  }

  digitalWriteFast(Pino_Sat_Lux1_intensidade_comuta, HIGH); // intensidade 2
  x=0;
  delay(1000);

  for (x=0;x<255;x++) {
    analogWrite(Pino_Sat_Lux1_intensidade1, x); // intensidade sat
    analogWrite(Pino_Sat_Lux1_intensidade2, x); // intensidade sat
    delay(15);
    digitalWriteFast(Pino_Sat_Lux1, HIGH);
    delay(1);
    digitalWriteFast(Pino_Sat_Lux1, LOW);
  }

  delay(1000);
}

void testalux() {

  analogWrite(Pino_Sat_Lux1_intensidade1, pwmmed); // intensidade sat
  analogWrite(Pino_Sat_Lux1_intensidade2, pwmalto); // intensidade sat

  digitalWriteFast(Pino_Sat_Lux1_intensidade_comuta, LOW); // 
  pulse1(Pino_Mede_Lux_pwm, Pino_Mede_Lux1, pwmalto);
  pulse1(Pino_Mede_Lux_pwm, Pino_Mede_Lux1, pwmbaixo);
  pulse1(Pino_Mede_Lux_pwm, Pino_Mede_Lux2, pwmalto);
  pulse1(Pino_Mede_Lux_pwm, Pino_Mede_Lux2, pwmbaixo);
  // pulse1(Pino_Cal_Lux1_pwm, Pino_Cal_Lux1, pwmbaixo);
  /*
digitalWriteFast(Pino_Sat_Lux1_intensidade_comuta, HIGH); // 
   pulse1(Pino_Mede_Lux_pwm, Pino_Mede_Lux1, pwmalto);
   pulse1(Pino_Mede_Lux_pwm, Pino_Mede_Lux1, pwmbaixo);
   pulse1(Pino_Mede_Lux_pwm, Pino_Mede_Lux2, pwmalto);
   pulse1(Pino_Mede_Lux_pwm, Pino_Mede_Lux2, pwmbaixo);
   pulse1(Pino_Cal_Lux1_pwm, Pino_Cal_Lux1, pwmbaixo);
   */

  delay(1000);
}

void pulse1(int a, int b, int c) { // a=pwm , b=1/0, c=pwm , d= pwm 2

  analogWrite(a, c); // 
  delay(10); // 
  for (i=0;i<ciclos;i++) { 
    if (i == ciclos/2) {
      digitalWriteFast(Pino_Sat_Lux1, HIGH);
    }
    dados0 = analogRead(detector1);
    inicia1 = micros();
    digitalWriteFast(b, HIGH);
    inicia1=inicia1+duracao_pulso;
    while (micros ()<inicia1) {
    }
    digitalWriteFast(b, LOW);
    if (i == ciclos*3/4) {
      digitalWriteFast(Pino_Sat_Lux1, LOW);
    }
    Serial.print(dados0);
    Serial.print(",");
    delay(duracao_ciclo);
  }
  i=0;
  delay(50);
  Serial.println();
}


