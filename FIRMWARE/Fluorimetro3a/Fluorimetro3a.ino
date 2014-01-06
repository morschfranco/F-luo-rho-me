
//bibiotecas:
#include <Keypad.h>        // http://playground.arduino.cc/Code/Keypad
#include <LiquidCrystal.h> // http://playground.arduino.cc/Code/LCD 

// descomente para habilitar modo de depuração
//#define DEBUG


//conexão do sensor no arduino
//TSL_FREQ_PIN só pode ser 2 ou 3 no 328. No mega pode ser 2, 3, 18, 19, ...
#define TSL_FREQ_PIN 2 // 
#define TSL_S0	     5
#define TSL_S1	     6
#define TSL_S2	     7
#define TSL_S3       8


// parâmetros do teclado
const byte LINHAS = 4; // quatro linhas
const byte COLUNAS = 3; // três colunas
// define mapeamento
// topologia do teclado
char teclas[LINHAS][COLUNAS] = {
  {'1','2','3'},
  {'4','5','6'},
  {'7','8','9'},
  {'*','0','#'}
};
// conexão dos terminais do teclado LINHA0, LINHA1, LINHA2 e LINHA3 aos pinos do Arduino
byte PinoLinha[LINHAS] = { 17, 18, 19, 4};
// Conexão dos terminais do teclado COL0, COL1 e COL2 aos pinos do Arduino
byte PinoColuna[COLUNAS] = { 14, 15, 16 }; 
// cria o keypad
Keypad kpd = Keypad( makeKeymap(teclas), PinoLinha, PinoColuna, LINHAS, COLUNAS );


// inicializa a biblioteca do LCD com a pinagem (observar RS /E e R/W)
LiquidCrystal lcd(11, 12, 0, 1, 9, 10);

unsigned long conta_pulso = 0;

int ConvertInt(String texto)
{
  char temp[20];
  texto.toCharArray(temp, 19);
  int x = atoi(temp);
  if (x == 0 && texto != "0")
  {
    x = -1;
  }
  return x;
}  

void setup() {
  
  lcd.begin(16, 2);
  #ifdef DEBUG
    lcd.print("teste");
    // taxa de transmissão da porta serial 0
    Serial.begin(9600);
  #endif
  
  // interrupção no pino 2 (INT0), disparada por subida de borda do
  // sinal de saída do TLR230R
  attachInterrupt(0, Incrementa_Pulso, RISING);

  // configuração do modo E/S dos pinos
  pinMode(TSL_FREQ_PIN, INPUT);
  pinMode(TSL_S0, OUTPUT);
  pinMode(TSL_S1, OUTPUT);
  pinMode(TSL_S2, OUTPUT);
  pinMode(TSL_S3, OUTPUT);
  
  // habilita pull-up's
  digitalWrite(TSL_S0, HIGH);
  digitalWrite(TSL_S1, HIGH);
  digitalWrite(TSL_S2, HIGH);
  digitalWrite(TSL_S3, HIGH);
}

void loop() {
  lcd.setCursor(0, 0);
  lcd.print("Tempo de Exposicao?");  // solicita o tempo de exposição 

String mostrador = "";
boolean varredura = true;
while(varredura){
 char key = kpd.getKey();
  if(key)  // se tecla pressionada..
  {
    switch (key)
    {
      case '*':
      
        lcd.setCursor(0, 0);
        lcd.print("*"); // "*"
        varredura = false;
        break;
      case '#':
        lcd.setCursor(0, 0);
        lcd.print("#");
        break;
      default:
      mostrador = mostrador + key;
        lcd.setCursor(0, 1);
        lcd.print(mostrador);
    }
  }
  }
  int temporiza = ConvertInt(mostrador);
  lcd.setCursor(0, 0);
  lcd.print("realiza   ");
 
  for (int i=20; i>0 ; i--){
    lcd.setCursor(0, 1);
    lcd.print(i);
    lcd.print("   ");
    delay(1000);
  } 
  
  lcd.setCursor(0, 0);
  lcd.print("temporiza   ");
  conta_pulso = 0;
 
  for (int i = temporiza; i>0 ; i--){
    lcd.setCursor(0, 1);
    lcd.print(i);
    lcd.print("   ");
    delay(1000);
  } 
  
  lcd.setCursor(0, 0);
  lcd.print("concluido");
  int total_pulsos = conta_pulso;
  lcd.setCursor(0, 1);
  lcd.print(total_pulsos);
  digitalWrite(13,HIGH);
  delay(500);
  digitalWrite(13,LOW);
  delay(1000);
  digitalWrite(13,HIGH);
  delay(1000);
  digitalWrite(13,LOW);
  delay(1000000);
}

void Incrementa_Pulso() {
  
  // incrementa contagem dos pulsos
  conta_pulso++;
  #ifdef DEBUG
    Serial.println(conta_pulso);
  #endif
  return;
}


