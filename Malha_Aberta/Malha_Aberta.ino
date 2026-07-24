// ============================================================================
// PROJETO CRIA 7 - Passo Controlo em Malha Aberta
// ============================================================================

// =================================
// PINOS
// =================================

const int DIR1 = A0;
const int PWM1 = 5;

const int DIR2 = A2;
const int PWM2 = 6;

const int SENSOR_ESQUERDO = 2;
const int SENSOR_DIREITO = 3;


// =================================
// CALIBRACAO E CONFIGURACAO
// =================================

const float TRANSICOES_POR_ROTACAO = 40.0;

const unsigned long INTERVALO_AMOSTRAGEM = 100; // 100 ms = 10 Hz


// =================================
// VARIAVEIS DO SENSOR ESQUERDO
// =================================

volatile unsigned long transicoesEsquerda = 0;

volatile unsigned long ultimaTransicaoEsquerda = 0;


// =================================
// VARIAVEIS DO SENSOR DIREITO
// =================================

volatile unsigned long transicoesDireita = 0;

volatile unsigned long ultimaTransicaoDireita = 0;


// =================================
// TEMPOS E COMANDOS
// =================================

unsigned long ultimoTempoAmostragem = 0;

const int PWM_FIXO = 100;


// =================================
// SETUP
// =================================

void setup() {

  pinMode(DIR1, OUTPUT);
  pinMode(PWM1, OUTPUT);

  pinMode(DIR2, OUTPUT);
  pinMode(PWM2, OUTPUT);

  pinMode(SENSOR_ESQUERDO, INPUT);
  pinMode(SENSOR_DIREITO, INPUT);


  // Define o sentido de marcha
  digitalWrite(DIR1, LOW);
  digitalWrite(DIR2, LOW);


  attachInterrupt(
    digitalPinToInterrupt(SENSOR_ESQUERDO),
    contarTransicaoEsquerda,
    CHANGE
  );


  attachInterrupt(
    digitalPinToInterrupt(SENSOR_DIREITO),
    contarTransicaoDireita,
    CHANGE
  );


  Serial.begin(9600);

  delay(2000);


  Serial.println("--- PASSO 1: MALHA ABERTA (PWM FIXO) ---");

  Serial.println("Tempo_ms,PWM_Esq,RPM_Esq,PWM_Dir,RPM_Dir");


  // Aplicar comando fixo aos motores
  analogWrite(PWM1, PWM_FIXO);

  analogWrite(PWM2, PWM_FIXO);


  ultimoTempoAmostragem = millis();
}


// =================================
// LOOP PRINCIPAL
// =================================

void loop() {

  unsigned long agora = millis();


  // Leitura e amostragem em tempo real (cada 100 ms)
  if (agora - ultimoTempoAmostragem >= INTERVALO_AMOSTRAGEM) {


    // Leitura atómica das transições acumuladas
    noInterrupts();

    unsigned long medidesEsq = transicoesEsquerda;

    unsigned long medidesDir = transicoesDireita;


    transicoesEsquerda = 0;

    transicoesDireita = 0;

    interrupts();


    // Tempo real decorrido em segundos
    float dt = (agora - ultimoTempoAmostragem) / 1000.0;

    ultimoTempoAmostragem = agora;


    // Cálculo de RPM usando a tua calibração de 40 transições por rotação
    float rpmEsquerda =
      (medidesEsq / TRANSICOES_POR_ROTACAO)
      * (60.0 / dt);


    float rpmDireita =
      (medidesDir / TRANSICOES_POR_ROTACAO)
      * (60.0 / dt);


    // Impressão dos dados em formato CSV para o Serial Monitor / Plotter
    Serial.print(agora);

    Serial.print(",");

    Serial.print(PWM_FIXO);

    Serial.print(",");

    Serial.print(rpmEsquerda, 2);

    Serial.print(",");

    Serial.print(PWM_FIXO);

    Serial.print(",");

    Serial.println(rpmDireita, 2);
  }
}


// =================================
// INTERRUPCAO MOTOR ESQUERDO
// =================================

void contarTransicaoEsquerda() {

  unsigned long agora =
    micros();


  if (
    agora - ultimaTransicaoEsquerda
    > 1000
  ) {

    transicoesEsquerda++;

    ultimaTransicaoEsquerda =
      agora;
  }
}


// =================================
// INTERRUPCAO MOTOR DIREITO
// =================================

void contarTransicaoDireita() {

  unsigned long agora =
    micros();


  if (
    agora - ultimaTransicaoDireita
    > 1000
  ) {

    transicoesDireita++;

    ultimaTransicaoDireita =
      agora;
  }
}