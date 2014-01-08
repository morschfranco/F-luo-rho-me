
/*
 PODEM SER UTILIZADOS:
 DETECTOR00.SCH
 DETECTOR01.SCH
 DETECTOR02.SCH
 DETECTOR03.SCH
 DETECTOR04.SCH
 
 Mede fluorescência modulada por pulso (pulse modulated fluorescence (PMF))
 a partir de pulso de saturação e pulso de medição.
 O LED do pulso de medição (Rebel LUXEON Orange) pode produzir fluorescência na faixa de INFRA-VERMELHO 
 a qual é detectada. Este sinal (IR) proporciona uma saída com níveis mais altos que o esperado. 
 De maneira a considerar a parcela do IR DEVE-SE REALIZAR A CALIBRAÇÃO.
 A CALIBRAÇÃO consiste num pequeno conjunto de pulsos que ocorrem no início do programa, emitidos por led de infravermelh (810 nm).
 O sinal indicado pelo detector, indicam o quanto a amostra é reflexiva para o IR. A curva de calibração é realizada a partir da
 inserção de um papael alumínio (alta reflexividade) e fita isolante de cor preta (não reflexiva). SEMPRE COM O LED DE CALIBRAÇÃO E DE MEDIÇÃO!
 A curva obtida informa o valor real da resposta do detector VERSUS o LED de calibração, que corresponde à base da resposta do eixo Y para o LED de medição.
 Uma vez obtida a curva de calibração, pode-se efetuar a medição, sem a interferência do IR.
 
 ver 0.5
 -ciclos 0/1 do LED realizado por temporizador. Um timer aciona o pulso de medição (dispara analogRead()), outro desliga e outro temporizador controla o ciclo do processo
 
 ver 0.6
 -bibliotecas necessárias:
 <SdFat.h>, <SdFatUtil.h>, <Time.h>, <Wire.h>, <DS1307RTC.h>, <PITimer.h>
 -ambiente utilizado:
  =>Se utilizado ARDUINO
    Arduino 1.0.3
  =>Se utilizado RASPBERRY Pi
    Wiring PI
  =>Se utilizado TEENSY
    Arduino 1.0.3 com Teensyduino http://www.pjrc.com/teensy/td_113/
 ver 0.7 
   NOVO MÉTODO DE SELEÇÃO DE PROCESSADOR.
 
 
 */

// Destaques e lembranças para o USUÁRIO para versão 0.6
//
// Ao mudar o método (protocolo), mudar o nome do arquivo salvo (ex: data-I.csv), ASSIM SABERÁ DO QUE SE TRATA
// O nome do arquivo NÃO pode ter mais que 12 caracteres ( 8 caracteres + .csv). Não há distinção entre caixa-alta e baixa.
// Arquivos possuem um nome base (Algas, Diesel, ...) para cada subrotina é adicionada uma extensão (Algas-I, Diesel-I, ...) para indicar de onde veio a subrotina
// Quando um novo experimento é criado, dados foram salvos (DIESEL-I) na pasta. Se quiseres interromper o experimento e reiniciá-lo, salvando com o mesmo nome (DIESEL-I), deves reabrir o arquivo antes de realizar o experimento.
// De outra maneira, podes criar um novo arquivo (DIESEL-I) num diretório diferente.
// Calibração é realizada para definir a reflectância na amostra a partir da excitação em 850 nm (LED). 
// MAIS DETALHES SOBRE ESTE PROCEDIMENTO:  https://opendesignengine.net/documents/14
// DEVE-SE REALIZAR A CALIBRAÇÃO SEMPRE QUE AS CONDIÇÕES DA AMOSTRA FOREM ALTERADAS (TEMP, PRESSÃO, HUMIDADE...AINDA N MONITORADAS!)
// A calibração anterior é salva no SD, se não for efetuada a calibração, os valores antigos serão utilizados para fundamentar o experimento.
// 10 bit, min = 0, max = 10^2 = 1023; 16 bit, min = 0, max = 16^2 = 65535; etc.
// ... : 3.3*((valor medido) / (máximo resolução)).
//

// Saturação e leitura ~ 500 ns. Processo < 2 us

// 
/* ANTIGO MÉTODO DE SELEÇÃO
//#define RASPBERRY
//define TEENSY
//#define MAPLE
//#define MEGA2560
*/

/////// NOVO MÉTODO DE SELEÇÃO DE PROCESSADOR
//#define RASPBERRY
//#define ARDUINO

#define TEENSY

#include <SdFat.h>
#include <SdFatUtil.h>   // funçoes para utilização, formato FAT, Windows e compatíveis
#include <Wire.h>        // I2C
#include <DS1307RTC.h>   // DS1307 informa tempo em atime_t
#ifdef TEENSY
#include <PITimer.h>     // temporizador interno
#include <Time.h>        // habilita RTCC
#endif


/* Código para sincronizar mensagens na serial. */
#define TIME_MSG_LEN 11 // informação de tempo é T mais 10 dígitos ASCII
#define TIME_HEADER 'T' // Cabeçaho da informação de tempo
const uint8_t SD_CHIP_SELECT = SS;

SdFat sd;
#define error(s) sd.errorHalt_P(PSTR(s)) // string de erro são salvas no cartão de memória

//variáveis do PROTOCOLO
int Medidas = 8;                     // Medidas por pulso (min 1 por pulso de 6 us)
unsigned long duracao_pulso = 50;    // duração em us. minimo = 6us
float duracao_ciclo = .01;           // em segnndos. Mínimo = duracao_pulso + 7.5us
unsigned long inicia_temporizador0, inicia_temporizador1, inicia_temporizador2;
float duracao_experimento = 2;       // em segnndos. Mínimo = duracao_ciclo
char nome_arquivo[13] = "DIESEL";    // nome_arquivo usado para salvar os dados no cartão de memória SD
const char* sufixo = "-I.CSV";       // terminação do arquivo
int Satura_Ciclo_Alto = 50;          // número de ciclos que a luz de saturação liga, se = 0, sem saturação
int Satura_Ciclo_Baixo = 125;        // número de ciclos que a luz de saturação apaga.

// CALIBRAÇÃO DO PROCESSO, INVESTIGAR VARIÁVEIS NÃO UTILIZADAS PARA LIBERAR RAM.

unsigned long Cal_Ciclos_Pulso = 50;     // Numero de ciclos alto e baixo
// tempo de medição e saturação = = ciclos*(duracao_pulso + perman)
unsigned long Cal_T_Alto_Pulso = 30;     // Tempo LED ligado (minimo = 5 us fundamentado no tempo de analogRead  (~4 us) -/+ 5 us para cada medida (analogRead) no pulso).
unsigned long Cal_T_Baixo_Pulso = 49970; // Tempo LED desligado (minimo = 20 us + operações adicionais).
float referencia = 1.2;                  // Referencia (AREF) para o ADC - ajuste do INTERNAL = 1,2 V
int Resolucao_AD = 16;                   // Resolução do conversor A/D, varia com PROCESSADOR USADO! (mÁx 16 bit, 13 bit efetivos)
int Pino_Mede_Lux1 = 3;                  // pino para medição da luz
int Pino_Sat_Lux1 = 4;                   // pino para luz de saturação
int Pino_Cal_Lux1 = 2;                   // pino para luz de calibração
int detector1 = A10;                     // pino do ADC

// Variáveis internas, constantes, contadores, matrizes e outros.
unsigned long inicia1, inicia1orig, fim1, Cal_inicia1_orig, Cal_fim1, inicia5, inicia6, inicia7, fim5;
unsigned long Verif_Pulso_Alto, Verif_Pulso_Baixo, Verif_Ciclo_Pulso, Verif_Tempo_Total, Cal_Verif_Tempo_Total;
float dados1f, dados2f, dados3f, dados4f, IR_Valor_Reflexivo, IR_Valor_Escuro, Rebel_Valor_Escuro, Rebel_Valor_Reflexivo, IR_Valor_Amostra, Rebel_Valor_Amostra, IR_Valor_Base, Valor_Medio, Cal_Valor_Medio1, Cal_Valor_Medio2, Rebel_Rampa, IR_Rampa, Valor_Base = 0;
char Nome_Diretorio[13];
char Local_Arquivo[13];
int Dados1=0, Dados2=0, Dados3=0, Dados4=0, Dados5=0, Dados6=0, Dados7=0, Dados8=0, Dados9=0, Dados10=0, Cal_Dados1, Cal_Dados2, Cal_Dados3, Cal_Dados4, Valor_Resolucao_AD;
int i = 0;               // contadorcontador
int j = 0;               // contador
int k = 0;               // contador
int z = 0;               // contador
int* Cal_Dado_Escuro;    // ponteiro
int* Cal_Dado_Reflexivo; // ponteiro
int* Cal_Amostra_Dado;   // ponteiro 
int* Amostra_Dado;       // ponteiro
int* Amostra_Dado_Rebel; // ponteiro 
int val = 0;
int cal = 0;
int cal2 = 0;
int cal3 = 0;
int val2 = 0;
int flag = 0;
int flag2 = 0;
int Botao_Pressionado = 0;
SdFile file;
SdFile sub1;
SdBaseFile* dir = &sub1;


//////////////////////////////// Rotina de configuração do processador ////////////////////////////////

void setup() {

  delay(3000);         // tempo para energização (carregar capacitâncias) e establização do sistema
  Serial.begin(38400); // 384000 é a taxa de transmissão mínima

#ifdef ARDUINO
  PITimer0.period(duracao_ciclo); // Intervalo (segundos) entre pulsos
  PITimer1.period(duracao_ciclo); // carrega com mesmo valor de PITimer0
  PITimer2.period(duracao_experimento);

  // ajusta o relógio do sistema, formato ASCII  "T" + 10 dígitos de informação
  //setSyncProvider(Teensy3Clock.get);
#endif
#ifdef MAPLE
  Timer0.period(duracao_ciclo); // Intervalo (segundos) entre pulsos
  Timer1.period(duracao_ciclo); // carrega com mesmo valor de PITimer0
  Timer2.period(duracao_experimento);

  // ajusta o relógio do sistema, formato ASCII  "T" + 10 dígitos de informação
  //setSyncProvider(Teensy3Clock.get);
#endif

#ifdef TEENSY
  // interrupção dos temporizadores utilizados
  PITimer0.period(duracao_ciclo); // Intervalo (segundos) entre pulsos
  PITimer1.period(duracao_ciclo); // carrega com mesmo valor de PITimer0
  PITimer2.period(duracao_experimento);

  // ajusta o relógio do sistema, formato ASCII  "T" + 10 dígitos de informação
  setSyncProvider(Teensy3Clock.get);
#endif

  while (!Serial);               // aguarda abertura do monitor da porta serial
  if (timeStatus()!= timeSet)
    Serial.println(" Não sincronizado com RTC");
  else
    Serial.println("Tempo sincronizado com RTC"); //

  // for (i=0;i<15;i++) {
  Serial.println(" Digite T seguido de tempo (10digASCII, T1234567890)");
  Serial.println(" (Em caso de erro, reinicie o processador)");
  // Serial.println(Serial.available());
  while (Serial.available() < 11) {
    // c = Serial.read();
    // Serial.print(Serial.available());
    // Serial.print(",");
    // Serial.println(Serial.peek());
    // delay(500);
  }
  atime_t t = processSyncMessage();
  //t = processSyncMessage();
  Serial.print(" 'T' acrescido de 10 digitos ASCII: ");
  Serial.println(t);
  Serial.print(" Buffer Serial disponivel: ");
  Serial.println(Serial.available());
  setTime(t);
  Serial.print("Tempo UTC ");
  MostraTempoSerial();
  // delay(3000);
  // digitalClockDisplay();
  // }
  Serial.println("");


  pinMode(Pino_Mede_Lux1, OUTPUT); // configura pino como saída
  pinMode(Pino_Sat_Lux1, OUTPUT); // configura pino como saída
  pinMode(Pino_Cal_Lux1, OUTPUT); // configura pino como saída
#ifdef TEENSY  
  analogReadAveraging(1); // Média ADC = 1(apenas uma medição, demora ~3 us)
  pinMode(detector1, EXTERNAL);
  analogReadRes(Resolucao_AD);
  Valor_Resolucao_AD = pow(2, Resolucao_AD); // determina o valor máximo lido a partir da resolução
#endif
#ifdef ARDUINO  
//  analogReadAveraging(1); // Média ADC = 1(apenas uma medição, demora ~3 us)
  pinMode(detector1, INPUT);
//  analogReadRes(Resolucao_AD);
  Valor_Resolucao_AD = 1023; // determina o valor máximo lido a partir da resolução
#endif
#ifdef MAPLE  
//  analogReadAveraging(1); // Média ADC = 1(apenas uma medição, demora ~3 us)
  pinMode(detector1, INPUT);
//  analogReadRes(Resolucao_AD);
  Valor_Resolucao_AD = 4095; // determina o valor máximo lido a partir da resolução
#endif


  if (!sd.begin(SD_CHIP_SELECT, SPI_FULL_SPEED)) sd.initErrorHalt(); // Taxa máxima de E/S para o SD.

  // Informações do cartão SD
  PgmPrint("RAM disponivel: ");
  Serial.println(FreeRam());

  PgmPrint(" Volume FAT");
  Serial.println(sd.vol()->fatType(), DEC);
  Serial.println();

  // arquivos no diretório raiz
  PgmPrintln("Arquivos DIR raiz:");
  sd.ls(LS_DATE | LS_SIZE);
  Serial.println();

  // Lista diretórios
  PgmPrintln("Arquivos nos diretórios:");
  sd.ls(LS_R);

  // Mostra outras informações
  Relatorio();

  //Cria novo diretório, arquivo ou abre existente
  //Se nome não corresponde a diretório ou arquivo, cria diretório p/ guardar arquivos (ex: DIESEL-I.CSV, DIESEL.C.CSV...)
  //Caso já exista arquivo ou pasta com o nome, cria NOVA pasta para armazenar dados.

  strcpy(Nome_Diretorio, nome_arquivo);
  Serial.println(sd.exists(Nome_Diretorio));

  if (sd.exists(Nome_Diretorio) == 1) {
    Serial.print("Pressione 1 para armazenar dados no diretório ");
    Serial.print(Nome_Diretorio);
    Serial.println(" ou pressione 2 para criar novo diretório.");

    while (cal3 != 1 && cal3 !=2) {
      cal3 = Serial.read()-48; // bug! entrar 0 desloca valores do terminal serial de 48.
      if (cal3 == 2) {
        for (j = 0; j < 100; j++) {
          Nome_Diretorio[0] = j/10 + '0';
          Nome_Diretorio[1] = j%10 + '0';
          if (sd.exists(Nome_Diretorio) == 0) {
            break;
          }
        }
        Serial.print("Criando novo diretório chamado: ");
        Serial.println(Nome_Diretorio);
        SdFile sub1;
        sub1.makeDir(sd.vwd(), Nome_Diretorio); // Cria novo diretório
        Serial.print(". Arquivos são salvos no diretório como ");
        Serial.print(nome_arquivo);
        Serial.println(" adicionando-(Denominador Subrotina).CSV");
      }
      else if (cal3 == 1) {
        for (j = 0; j < 100; j++) {
          Nome_Diretorio[0] = j/10 + '0';
          Nome_Diretorio[1] = j%10 + '0';
          if (sd.exists(Nome_Diretorio) == 0) {
            Nome_Diretorio[0] = (j-1)/10 + '0';
            Nome_Diretorio[1] = (j-1)%10 + '0';
            break;
          }
        }
        Serial.print("Abrindo arquivos mais recentes no diretório ");
        Serial.println(Nome_Diretorio);
        break;
      }
    }
  }
  else if (sd.exists(Nome_Diretorio) == 0) {

    Serial.print("Criando novo diretório chamado: ");
    Serial.print(Nome_Diretorio);
    Serial.print(". Data files are saved in that directory as ");
    Serial.print(Nome_Diretorio);
    Serial.println(" adicionando-(Denominador Subrotina).CSV");
    sub1.makeDir(sd.vwd(), Nome_Diretorio); // Cria novo diretório
  }



  //  -> Experimento!!!
  Serial.println("Deseja calibrar? --digite 1 para SIM ou 2 para NÃO");
  while (cal == 0 | flag == 0) {
    cal = Serial.read()-48; //
    if (cal == 1) {
      delay(50);
      Serial.println("Módulo de Calibração");
      Serial.println("Coloque o PADRÃO REFLEXIVO no orifício");
      Serial.println("Pressione qualquer tecla para continuar");
      Serial.flush();
      while (Serial.read () <= 0) {
      }

      Serial.println("Entendido, calibrando");
      CalibraRebel(); // CalibraRebel() antes de CalibraReflexivo()!
      CalibraReflexivo();

      Serial.println("Coloque o PADRÃO ESCURO no orifício");
      Serial.println("Pressione qualquer tecla para continuar");
      Serial.flush();
      while (Serial.read () <= 0) {
      }

      Serial.println("Entendido, calibrando");
      CalibraRebel(); //  CalibraRebel() antes de CalibraEscuro()
      CalibraEscuro();
      Serial.println("Calibração concluída!");
      cal = 2;
    }
    if (cal == 2) {
      delay(50);
      Serial.println("Iniciando procedimentos de medição");
      Serial.println("Coloque amostra no orifício e pressione algo para continuar");
      Serial.flush();
      while (Serial.read () <= 0) {
      }
      Serial.println("Entendido, realizando procedimento...");
      while (1) {
        CalibraAmostra(); // CalibraAmostra() antes de Fluoresce()
        Fluoresce();
        Calcula();
        Serial.println("");
        Serial.println("Pressione alguma tecla para realizar medida ou RESET para calibrar ou iniciar novo arquivo");
        while (Serial.read () <= 0) {
        }
      }
    }

    else if (cal > 2) {
      delay(50);
      Serial.println("Tecla inválida, entre 1 para SIM ou 2 para NÃO.");
      cal = 0;
    }
  }
}


void CalibraRebel() {

  // Rotina de calibração REBEL
  // Emite pulsos e amostra no LED de medição (REBEL ORANGE)

  Amostra_Dado_Rebel = (int*)malloc(Cal_Ciclos_Pulso*sizeof(int)); // Matriz para armazenar valores de cada ciclo (LIGA/DESLIGA)
  noInterrupts();

  inicia1orig = micros();
  inicia1 = micros();
  for (i=0; i < Cal_Ciclos_Pulso; i++) {
  #ifdef ARDUINO
    digitalWrite(Pino_Mede_Lux1, HIGH);
  #endif
  #ifdef MAPLE
    digitalWrite(Pino_Mede_Lux1, HIGH);
  #endif
  #ifdef TEENSY
    digitalWriteFast(Pino_Mede_Lux1, HIGH);
  #endif  
  #ifdef RASPBERRY
    digitalWrite(Pino_Mede_Lux1, HIGH);
  #endif

    Dados1 = analogRead(detector1);
    inicia1 = inicia1 + Cal_T_Alto_Pulso;
    while (micros() < inicia1) {
    }
    inicia1 = inicia1 + Cal_T_Baixo_Pulso;
  #ifdef ARDUINO
    digitalWrite(Pino_Mede_Lux1, LOW);
  #endif
  #ifdef MAPLE
    digitalWrite(Pino_Mede_Lux1, LOW);
  #endif
  #ifdef TEENSY
    digitalWriteFast(Pino_Mede_Lux1, LOW);
  #endif  
  #ifdef RASPBERRY
    digitalWrite(Pino_Mede_Lux1, LOW);
  #endif
//    digitalWriteFast(Pino_Mede_Lux1, LOW);
    Amostra_Dado_Rebel[i] = Dados1;
    while (micros() < inicia1) {
    }
  }
  fim1 = micros();
  interrupts();

  free(Amostra_Dado_Rebel); // libera memória alocada

  for (i=0; i < Cal_Ciclos_Pulso; i++) {
    Rebel_Valor_Amostra += Amostra_Dado_Rebel[i]; // Faz somatório das leituras analógicas.
  }
  delay(50);

  Rebel_Valor_Amostra = (float) Rebel_Valor_Amostra; // Converte Rebel_Valor_Amostra para float.
  Rebel_Valor_Amostra = (Rebel_Valor_Amostra / Cal_Ciclos_Pulso);
  Serial.print(" Valor amostrado (REBEL): ");
  Serial.println(Rebel_Valor_Amostra);
  Serial.println("");
  for (i=0; i < Cal_Ciclos_Pulso; i++) { //  Mostra o resultado
    Serial.print(Amostra_Dado_Rebel[i]);
    Serial.print(", ");
    Serial.print(" ");
  }
  Serial.println("");
  Serial.print("Visualização dos dados: ");
  Serial.println(Dados1);
}


void CalibraReflexivo() {

  // -> Calibra reflexivo
  // Identifica quão reflexiva é a amostra de PAPEL ALUMÍNIO, utilizado pulso do REBEL ORANGE .

  Cal_Dado_Reflexivo = (int*)malloc(Cal_Ciclos_Pulso*sizeof(int)); // Matriz para armazenar valores de cada ciclo (LIGA/DESLIGA)
  noInterrupts(); // Desabilita interrupções para evitar interferência de outras rotinas

    inicia1 = micros();
  for (i=0; i < Cal_Ciclos_Pulso; i++) {
    digitalWriteFast(Pino_Cal_Lux1, HIGH);
    Cal_Dados1 = analogRead(detector1);
    inicia1 = inicia1 + Cal_T_Alto_Pulso;
    while (micros()<inicia1) {
    }
    inicia1 = inicia1 + Cal_T_Baixo_Pulso;
    digitalWriteFast(Pino_Cal_Lux1, LOW);
    Cal_Dado_Reflexivo[i] = Cal_Dados1;
    while (micros() < inicia1) {
    }
  }

  interrupts();

  for (i=0; i < Cal_Ciclos_Pulso; i++) {
    IR_Valor_Reflexivo += Cal_Dado_Reflexivo[i]; // Faz somatório das leituras analógicas.
  }
  Serial.println(IR_Valor_Reflexivo);
  IR_Valor_Reflexivo = (float) IR_Valor_Reflexivo;
  IR_Valor_Reflexivo = (IR_Valor_Reflexivo / Cal_Ciclos_Pulso); // Divide pelo número de amostras
  Rebel_Valor_Reflexivo = Rebel_Valor_Amostra;
  Rebel_Valor_Amostra = (int) Rebel_Valor_Amostra; // converte Rebel_Valor_Amostra para int
  for (i=0; i < Cal_Ciclos_Pulso; i++) { //  Mostra o resultado
    Serial.print(Cal_Dado_Reflexivo[i]);
    Serial.print(", ");
    Serial.print(" ");
  }
  Serial.println(" ");
  Serial.print("Valor de base para amostra REFLEXIVA: ");
  Serial.println(IR_Valor_Reflexivo, 7);
  Serial.print("Últimos quatro valores de calibração: ");
  Serial.println(Cal_Dados1);
}

void CalibraEscuro() {

  // Valor de base para amostra ESCURA:
  // Determina absorção (~850 nm) a partir da amostra de fita isolante elétrica fosca.

  Cal_Dado_Escuro = (int*)malloc(Cal_Ciclos_Pulso*sizeof(int)); // Matriz para armazenar valores de cada ciclo (LIGA/DESLIGA)
  noInterrupts(); // Desabilita interrupções para evitar interferência de outras rotinas

  inicia1 = micros();
  for (i=0; i < Cal_Ciclos_Pulso; i++) {
    digitalWriteFast(Pino_Cal_Lux1, HIGH);
    Cal_Dados1 = analogRead(detector1);
    inicia1 = inicia1 + Cal_T_Alto_Pulso;
    while (micros() < inicia1) {
    }
    inicia1 = inicia1 + Cal_T_Baixo_Pulso;
    digitalWriteFast(Pino_Cal_Lux1, LOW);
    Cal_Dado_Escuro[i] = Cal_Dados1;
    while (micros()< inicia1) {
    }
  }

  interrupts();

  for (i = 0; i < Cal_Ciclos_Pulso; i++) {
    IR_Valor_Escuro += Cal_Dado_Escuro[i]; // Faz somatório das leituras analógicas.
  }
  Serial.println(IR_Valor_Escuro);
  IR_Valor_Escuro = (float) IR_Valor_Escuro;
  IR_Valor_Escuro = (IR_Valor_Escuro / Cal_Ciclos_Pulso);
  Rebel_Valor_Escuro = Rebel_Valor_Amostra;
  Rebel_Valor_Amostra = (int) Rebel_Valor_Amostra; // converte Rebel_Valor_Amostra para int
  for (i=0; i < Cal_Ciclos_Pulso; i++) { //  Mostra o resultado
    Serial.print(Cal_Dado_Escuro[i]);
    Serial.print(", ");
    Serial.print(" ");
  }
  Serial.println(" ");
  Serial.print("Valor de base para baixa Reflectância: ");
  Serial.println(IR_Valor_Escuro, 7);
  Serial.print("Últimos quatro valores de calibração: ");
  Serial.println(Cal_Dados1);



  //Calcula faixa de valores e salva calibração no SD
  Rebel_Rampa = Rebel_Valor_Reflexivo - Rebel_Valor_Escuro;
  IR_Rampa = IR_Valor_Reflexivo - IR_Valor_Escuro;

  file.open("REscuro.CSV", O_CREAT | O_WRITE);
  file.seekSet(0);
  file.print(Rebel_Valor_Escuro, 8);
  file.close();

  file.open("RReflex.CSV", O_CREAT | O_WRITE);
  file.seekSet(0);
  file.print(Rebel_Valor_Reflexivo, 8);
  file.close();

  file.open("IEscuro.CSV", O_CREAT | O_WRITE);
  file.seekSet(0);
  file.print(IR_Valor_Escuro, 8);
  file.close();

  file.open("IReflex.CSV", O_CREAT | O_WRITE);
  file.seekSet(0);
  file.print(IR_Valor_Reflexivo, 8);
  file.close();

  file.open("RFaixa.CSV", O_CREAT | O_WRITE);
  file.seekSet(0);
  file.print(Rebel_Rampa, 8);
  file.close();

  file.open("IFaixa.CSV", O_CREAT | O_WRITE);
  file.seekSet(0);
  file.print(IR_Rampa, 8);
  file.close();

  Serial.print("Faixa IR: ");
  Serial.println(IR_Rampa, 8);
  Serial.print("Faixa REBELe: ");
  Serial.println(Rebel_Rampa, 8);
  Serial.print("IR reflexivo: ");
  Serial.println(IR_Valor_Reflexivo, 8);
  Serial.print("IR escuro: ");
  Serial.println(IR_Valor_Escuro, 8);
  Serial.print("REBEL reflexivo: ");
  Serial.println(Rebel_Valor_Reflexivo, 8);
  Serial.print("REBEL escuro: ");
  Serial.println(Rebel_Valor_Escuro, 8);
}

void CalibraAmostra() {

  // Calibra amostra
  // Dispara luz de calibraçaõ para determinar quanto a amostra é reflexiva para 850 nm

  Cal_Amostra_Dado = (int*)malloc(Cal_Ciclos_Pulso*sizeof(int)); // Matriz para armazenar valores de cada ciclo (LIGA/DESLIGA)
  noInterrupts(); // Desabilita interrupções para evitar interferência de outras rotinas

  Cal_inicia1_orig = micros();
  inicia1 = micros();
  for (i=0;i<Cal_Ciclos_Pulso;i++) {
    digitalWriteFast(Pino_Cal_Lux1, HIGH);
    Cal_Dados1 = analogRead(detector1);
    inicia1 = inicia1 + Cal_T_Alto_Pulso;
    while (micros() < inicia1) {
    }
    inicia1 = inicia1 + Cal_T_Baixo_Pulso;
    digitalWriteFast(Pino_Cal_Lux1, LOW);
    Cal_Amostra_Dado[i] = Cal_Dados1;
    while (micros() < inicia1) {
    }
  }
  Cal_fim1 = micros();

  interrupts();

  for (i=0; i < Cal_Ciclos_Pulso; i++) {
    IR_Valor_Amostra += Cal_Amostra_Dado[i]; // Faz somatório das leituras analógicas.Faz somatório das leituras analógicas.
  }
  // Serial.println(IR_Valor_Amostra);
  IR_Valor_Amostra = (float) IR_Valor_Amostra;
  IR_Valor_Amostra = (IR_Valor_Amostra / Cal_Ciclos_Pulso);
  for (i=0; i < Cal_Ciclos_Pulso; i++) { //  Mostra o resultado
    Serial.print(Cal_Amostra_Dado[i]);
    Serial.print(", ");
    Serial.print(" ");
  }
  Serial.println(" ");
  Serial.print("Valor de base para reflectância da amostra: ");
  Serial.println(IR_Valor_Amostra, 7);

  // Acessa valores de calibração salvos para determinar valor de base

  int c;
  char local[12];

  Serial.println("Dados obtidos do cartão SD");

  file.open("IFaixa.CSV", O_READ);
  i = 0; //assegura que o contator está zerado
  while ( (c = file.read ()) > 0) {
    local[i] = (char) c;
    i++;
  }
  i = 0; //zera contador
  IR_Rampa = strtod(local, NULL);
  Serial.println(IR_Rampa, 8);
  file.close();

  file.open("RFaixa.CSV", O_READ);
  i = 0; //assegura que o contator está zerado
  while ( (c = file.read ()) > 0) {
    local[i] = (char) c;
    i++;
  }
  i = 0; //zera contador
  Rebel_Rampa = strtod(local, NULL);
  Serial.println(Rebel_Rampa, 8);
  file.close();

  file.open("IReflex.CSV", O_READ);
  i = 0; //assegura que o contator está zerado
  while ( (c = file.read ()) > 0) {
    local[i] = (char) c;
    i++;
  }
  i = 0; //zera contador
  IR_Valor_Reflexivo = strtod(local, NULL);
  Serial.println(IR_Valor_Reflexivo, 8);
  file.close();

  file.open("IEscuro.CSV", O_READ);
  i = 0; //assegura que o contator está zeradozera contador
  while ( (c = file.read ()) > 0) {
    local[i] = (char) c;
    i++;
  }
  i = 0; //zera contador
  IR_Valor_Escuro = strtod(local, NULL);
  Serial.println(IR_Valor_Escuro, 8);
  file.close();

  file.open("RReflex.CSV", O_READ);
  i = 0; //assegura que o contator está zerado
  while ( (c = file.read ()) > 0) {
    local[i] = (char) c;
    i++;
  }
  i = 0; //zera contador
  Rebel_Valor_Reflexivo = strtod(local, NULL);
  Serial.println(Rebel_Valor_Reflexivo, 8);
  file.close();

  file.open("REscuro.CSV", O_READ);
  i = 0; //assegura que o contator está zerado
  while ( (c = file.read ()) > 0) {
    local[i] = (char) c;
    i++;
  }
  i = 0; //zera contador
  Rebel_Valor_Escuro = strtod(local, NULL);
  Serial.println(Rebel_Valor_Escuro, 8);

  file.close();
  sub1.close();

  // Calcula valor de base
  Valor_Base = (Rebel_Rampa*(IR_Valor_Amostra-IR_Valor_Reflexivo)/(IR_Rampa-Rebel_Valor_Reflexivo));
}

void Fluoresce() {
  // Dispara LED no ciclo LIGA/DESLIGA, realiza amostragens a partir de analogRead no ciclo LIGA

  strcpy(Local_Arquivo, nome_arquivo); // Nome do arquivo e sua extensão
  strcat(Local_Arquivo, sufixo); // Adiciona extensão à unidade a partir do cabeçalho
  sub1.open(Nome_Diretorio, O_READ);
  file.open(dir, Local_Arquivo, O_CREAT | O_WRITE | O_APPEND);
  strcpy(Local_Arquivo, nome_arquivo); // ajusta nome do arquivo;

  // salva hora/tempo
  MostraTempo();

  Serial.print(" Valor de base: ");
  Serial.println(Valor_Base);

  Amostra_Dado = (int*)malloc((duracao_experimento/duracao_ciclo)*sizeof(int)); // Matriz para armazenar valores de cada ciclo (LIGA/DESLIGA)

  inicia_temporizador0 = micros()+100; // Aguarda processador se "acomodar" antes de iniciar
  inicia_temporizador1 = inicia_temporizador0+duracao_pulso;
  while (micros()<inicia_temporizador0) {
  }
  PITimer0.start(PulsoAlto); // LED ligado, demora 1 us para chamar "inicio" em PITimer mais 5 us por analogRead() realizado
  while (micros()<inicia_temporizador1) {
  }
  PITimer1.start(PulsoBaixo); // LED desligado.
  PITimer2.start(InterrompeTemporizador); // Desliga temporizadores após conclusão do experimento

  // Aguarda > 10 ms para tudo desligar
  delay(duracao_experimento*1000+10);

  z = 0; //zera contador z

  file.println("");
  file.close(); // Fecha arquivo
  sub1.close(); // Fecha dirretório
}

// relata principais parâmetros
void Relatorio() {

  Serial.println("");
  Serial.print("Ciclos de calibração ");
  Serial.println(Cal_Ciclos_Pulso); // Numero de LIGA/DESLIGA do LED de calibração (máximo = 1000)
  Serial.print("Tempo LED (saturação) ligado ");
  Serial.println(Satura_Ciclo_Alto*duracao_ciclo); //(ajuste = 0 para não-saturação)
  Serial.print("Tempo de LED(saturação) desligado ");
  Serial.println(Satura_Ciclo_Baixo*duracao_ciclo); // tempo de LED desligado 
  Serial.print(" Tempo de um pulso de medição em us ");
  Serial.println(duracao_pulso); // Tempo de pulso LED em us (mínimo =5 us, obtido de ~4us analogRead +/- 5us para cada analogRead adicional)
  Serial.print("Tempo de espera entre pulsos em us ");
  Serial.println(duracao_ciclo-((float)duracao_pulso/1000000)); // Tempo de pulso LED em us (mínimo =5 us, obtido de ~4us analogRead +/- 5us para cada analogRead adicional)
  Serial.print("Arquivo usado no experimento ");
  Serial.println(nome_arquivo);
  Serial.println("");
  Serial.print(" Pino de medição ");
  Serial.println(Pino_Mede_Lux1);
  Serial.print("Pino de saturação ");
  Serial.println(Pino_Sat_Lux1);
  Serial.print("Pino de calibração ");
  Serial.println(Pino_Cal_Lux1);
  Serial.print("Pino do detector ");
  Serial.println(detector1);
  Serial.println("");
}

//  Informa resultados ao usuário
void Calcula() {

  Serial.println("Dados, leitura analógica em uV");
  for (i=0; i<(duracao_experimento/duracao_ciclo); i++) {
    Amostra_Dado[i] = 1000000*((referencia*Amostra_Dado[i])/(Valor_Resolucao_AD*Medidas));
  }
  for (i=0; i<(duracao_experimento/duracao_ciclo); i++) { //  Mostra o resultado
    Serial.print(Amostra_Dado[i], 8);
    Serial.print(",");
  }
  Serial.println("");

  Serial.println("Diretório dos dados");

  int16_t c;
  sub1.open(Nome_Diretorio, O_READ);
  strcpy(Local_Arquivo, nome_arquivo); // ajusta nome e extensão do arquivo
  strcat(Local_Arquivo, sufixo);
  Serial.print("Diretório do arquivo: ");
  Serial.println(Nome_Diretorio);
  Serial.print("Nome do arquivo ");
  Serial.println(Local_Arquivo);
  file.open(dir, Local_Arquivo, O_READ);
  while ((c = file.read()) > 0) Serial.write((char)c); // publica informações na porta serial
  strcpy(Local_Arquivo, nome_arquivo); // ajusta nome do arquivo;

  file.close(); // Fecha arquivo
  sub1.close(); // Fecha diretório

  Verif_Tempo_Total = fim1 - inicia1orig;
  Cal_Verif_Tempo_Total = Cal_fim1 - Cal_inicia1_orig;

  Serial.println("");
  Serial.print("Valor de base: ");
  Serial.println(Valor_Base, 8);

  Verif_Tempo_Total = fim1 - inicia1orig;
  Cal_Verif_Tempo_Total = Cal_fim1 - Cal_inicia1_orig;

  Serial.println("");
  Serial.println("Informações:");
  Serial.println("");

  Serial.print("Tempo de exposição(pulsos medição): ");
  Serial.println(Verif_Tempo_Total);

  Serial.print("Tempo esperado de execução(pulsos medição): ");
  Serial.println(duracao_experimento);

  Serial.print("Tempo de execução (pulsos calibração): ");
  Serial.println(Cal_Verif_Tempo_Total);

  Serial.println("");
  Serial.println("Dados de calibração");
  Serial.println("");


  Serial.print("Valor de base para amostra: ");
  Serial.println(Valor_Base);
  Serial.print("Valor de calibração reflexivo: ");
  Serial.print(Rebel_Valor_Reflexivo);
  Serial.print("e ");
  Serial.println(IR_Valor_Reflexivo);
  Serial.print("Valor de calibração escuro: ");
  Serial.print(Rebel_Valor_Escuro);
  Serial.print("eValor de calibração escuro ");
  Serial.println(IR_Valor_Escuro);

  delay(50);
}


atime_t processSyncMessage() {
  //  informa tempo, se a mensagem na serial é válida
  while (Serial.available() >= TIME_MSG_LEN ) { // mensagem de tempo = cabeçalho + 10 ASCCI
    char c = Serial.read() ;
    Serial.print(c);
    if (c == TIME_HEADER ) {
      atime_t pctime = 0;
      for (int i=0; i < TIME_MSG_LEN -1; i++) {
        c = Serial.read();
        if ( c >= '0' && c <= '9') {
          pctime = (10 * pctime) + (c - '0') ; // Converte dígitos em número
        }
      }
      return pctime;
      Serial.println(" hora no pc");
      Serial.println(pctime);
    }
  }
  return 0;
  i=0; // reset i
}

// Mostra tempo na porta serial
void MostraTempoSerial() {
  Serial.print(month());
  Serial.print("/");
  Serial.print(day());
  Serial.print("/");
  Serial.print(year());
  Serial.print(" ");
  Serial.print(hour());
  MostraDigitoSerial(minute());
  MostraDigitoSerial(second());
  Serial.println();
}

void MostraDigitoSerial(int digitos) {
  // função relógio digital, informa tempo como..
  Serial.print(":");
  if (digitos < 10)
    Serial.print('0');
  Serial.print(digitos);
}

/*
void fileprintDigits(int digitos){
 // função relógio digital, informa tempo como..
 file.print(":");
 if(digitos < 10)
 file.print('0');
 file.print(minute());
 }
 */

void MostraTempo() {
  file.print(month());
  file.print("/");
  file.print(day());
  file.print("/");
  file.print(year());
  file.print(" ");
  file.print(hour());
  file.print(":");
  if (minute() < 10)
    file.print('0');
  file.print(minute());
  file.print(":");
  if (second() < 10)
    file.print('0');
  file.print(second());
  file.print(",");
}

void PulsoAlto() {
  // Serial.print(z);
  if (z==Satura_Ciclo_Alto-1) { // liga luz de saturação no início do período de medição
    digitalWriteFast(Pino_Sat_Lux1, HIGH);
  }
  digitalWriteFast(Pino_Mede_Lux1, HIGH);
  Dados1 = analogRead(detector1);
  i = 0;
}

void PulsoBaixo() {

  // OBS.: Para ciclos DESLIGADO muito curtos, armazene dados em Amostra_Dado[], escreva
  // no SD ao final. Se DESLIGADO for maior que 50 us, podes escrever
  // diretamente no SD. Amostra_Dado[] limita a ~1500 dados
  // antes de ficar cheio

  digitalWriteFast(Pino_Mede_Lux1, LOW);
  if (z==Satura_Ciclo_Baixo-1) { // desliga luz de saturação no final do pulso de medição
    digitalWriteFast(Pino_Sat_Lux1, LOW);
  }
  Amostra_Dado[z] = Dados1;
  Serial.println(Amostra_Dado[z]);
  file.print(Amostra_Dado[z]);
  file.print(",");
  Dados1 = 0; // Reinicia datal para a próxima rodada
  z=z+1;
}


void InterrompeTemporizador() { // para temporizadores, fecha arquivo e diretório no SD, libera memório de Amostra_DAdo, desliga luzes, zera contadores, move para a próxima linha no arquivo .csv
  PITimer0.stop();
  PITimer1.stop();
  PITimer2.stop();
  fim1 = micros();
  free(Amostra_Dado); // libera memória alocada
  digitalWriteFast(Pino_Mede_Lux1, LOW);
  digitalWriteFast(Pino_Cal_Lux1, LOW);
  digitalWriteFast(Pino_Sat_Lux1, LOW);
  z=0; //zera contadores
  i=0;
  Serial.print("duração do experimento ~: ");
  Serial.println(fim1-inicia_temporizador0);
}

void loop() {
}

