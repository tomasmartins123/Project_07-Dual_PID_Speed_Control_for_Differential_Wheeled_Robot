// ============================================================================
// PROJETO CRIA 7 - Controlo PID 2 rodas
// ============================================================================

const int DIR1 = A0;
const int PWM1 = 5;
const int SENSOR_ESQUERDO = 2;

const int DIR2 = A1;
const int PWM2 = 6;
const int SENSOR_DIREITO = 3;

const float TRANSICOES_POR_ROTACAO = 40.0;
const unsigned long INTERVALO_AMOSTRAGEM = 150;

float rpmDesejado = 100.0;

// --- GANHOS SELECCIONADOS DO PID ---
const float Kp = 2.00;
const float Ki = 1.25;
const float Kd = 0.06;

// --- DURAÇÃO DO ENSAIO ---
const unsigned long DURACAO_ENSAIO_MS = 15000; // 15 segundos
unsigned long tempoInicioEnsaio = 0;
bool testeConcluido = false;

// --- VARIÁVEIS DO SENSOR E CONTROLADOR (ESQUERDA) ---
volatile unsigned long transicoesEsquerda = 0;
volatile unsigned long ultimaTransicaoEsquerda = 0;
float rpmFiltradoEsq = 0.0;
float acumuladorErroEsq = 0.0;
float erroAnteriorEsq = 0.0;
int pwmEsquerda = 0;

// --- VARIÁVEIS DO SENSOR E CONTROLADOR (DIREITA) ---
volatile unsigned long transicoesDireita = 0;
volatile unsigned long ultimaTransicaoDireita = 0;
float rpmFiltradoDir = 0.0;
float acumuladorErroDir = 0.0;
float erroAnteriorDir = 0.0;
int pwmDireita = 0;

unsigned long ultimoTempoAmostragem = 0;

void contarTransicaoEsquerda() {
  unsigned long agora = micros();
  if (agora - ultimaTransicaoEsquerda > 1000) {
    transicoesEsquerda++;
    ultimaTransicaoEsquerda = agora;
  }
}

void contarTransicaoDireita() {
  unsigned long agora = micros();
  if (agora - ultimaTransicaoDireita > 1000) {
    transicoesDireita++;
    ultimaTransicaoDireita = agora;
  }
}

void setup() {
  pinMode(DIR1, OUTPUT); pinMode(PWM1, OUTPUT); pinMode(SENSOR_ESQUERDO, INPUT);
  pinMode(DIR2, OUTPUT); pinMode(PWM2, OUTPUT); pinMode(SENSOR_DIREITO, INPUT);

  digitalWrite(DIR1, HIGH);
  digitalWrite(DIR2, HIGH);

  attachInterrupt(digitalPinToInterrupt(SENSOR_ESQUERDO), contarTransicaoEsquerda, CHANGE);
  attachInterrupt(digitalPinToInterrupt(SENSOR_DIREITO), contarTransicaoDireita, CHANGE);

  Serial.begin(115200);
  delay(2000);

  Serial.println("Tempo_ms,Setpoint,RPM_Esq,RPM_Dir,PWM_Esq,PWM_Dir");

  tempoInicioEnsaio = millis();
  ultimoTempoAmostragem = millis();
}

void loop() {
  if (testeConcluido) return;

  unsigned long agora = millis();

  // Condição de terminação aos 15 segundos
  if (agora - tempoInicioEnsaio >= DURACAO_ENSAIO_MS) {
    testeConcluido = true;
    analogWrite(PWM1, 0);
    analogWrite(PWM2, 0);
    Serial.println("# FIM_TESTE");
    return;
  }

  if (agora - ultimoTempoAmostragem >= INTERVALO_AMOSTRAGEM) {

    noInterrupts();
    unsigned long medEsq = transicoesEsquerda;
    unsigned long medDir = transicoesDireita;
    transicoesEsquerda = 0;
    transicoesDireita = 0;
    interrupts();

    float dt = (agora - ultimoTempoAmostragem) / 1000.0;
    ultimoTempoAmostragem = agora;

    // RPM e Filtro EMA
    float rpmBrutoEsq = (medEsq / TRANSICOES_POR_ROTACAO) * (60.0 / dt);
    rpmFiltradoEsq = (0.3 * rpmBrutoEsq) + (0.7 * rpmFiltradoEsq);

    float rpmBrutoDir = (medDir / TRANSICOES_POR_ROTACAO) * (60.0 / dt);
    rpmFiltradoDir = (0.3 * rpmBrutoDir) + (0.7 * rpmFiltradoDir);

    // PID Roda Esquerda
    float erroEsq = rpmDesejado - rpmFiltradoEsq;
    acumuladorErroEsq += erroEsq * dt;
    acumuladorErroEsq = constrain(acumuladorErroEsq, -100.0, 100.0);
    float derivadoErroEsq = (erroEsq - erroAnteriorEsq) / dt;
    float sinalEsq = (Kp * erroEsq) + (Ki * acumuladorErroEsq) + (Kd * derivadoErroEsq);
    pwmEsquerda = constrain((int)sinalEsq, 0, 255);
    erroAnteriorEsq = erroEsq;

    // PID Roda Direita
    float erroDir = rpmDesejado - rpmFiltradoDir;
    acumuladorErroDir += erroDir * dt;
    acumuladorErroDir = constrain(acumuladorErroDir, -100.0, 100.0);
    float derivadoErroDir = (erroDir - erroAnteriorDir) / dt;
    float sinalDir = (Kp * erroDir) + (Ki * acumuladorErroDir) + (Kd * derivadoErroDir);
    pwmDireita = constrain((int)sinalDir, 0, 255);
    erroAnteriorDir = erroDir;

    // Atuação
    analogWrite(PWM1, pwmEsquerda);
    analogWrite(PWM2, pwmDireita);

    // Telemetria
    Serial.print(agora); Serial.print(",");
    Serial.print(rpmDesejado, 1); Serial.print(",");
    Serial.print(rpmFiltradoEsq, 1); Serial.print(",");
    Serial.print(rpmFiltradoDir, 1); Serial.print(",");
    Serial.print(pwmEsquerda); Serial.print(",");
    Serial.println(pwmDireita);
  }
}