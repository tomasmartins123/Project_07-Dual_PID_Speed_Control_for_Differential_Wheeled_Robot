// ============================================================================
// PROJETO CRIA 7 - Ensaio Automático Kd com Filtro Passa-Baixo (EMA)
// ============================================================================

const int DIR1 = A0;
const int PWM1 = 5;
const int SENSOR_ESQUERDO = 2;

const float TRANSICOES_POR_ROTACAO = 40.0;
const unsigned long INTERVALO_AMOSTRAGEM = 150; // 150 ms para melhor resolução (10 RPM por salto)

float rpmDesejado = 100.0;

// --- PARÂMETROS FIXOS (KP e KI SELECIONADOS) ---
const float Kp_fixo = 2.0;  // Ganho Proporcional ótimo
const float Ki_fixo = 1.25; // Ganho Integral ótimo

// --- TESTE AUTOMÁTICO DE KD ---
float valoresKd[5] = {0.02, 0.04, 0.06, 0.08, 0.1};
int subEnsaio = 0;
bool testeConcluido = false;
unsigned long tempoInicioSubEnsaio = 0;

// --- VARIÁVEIS DO SENSOR E CONTROLADOR ---
volatile unsigned long transicoesEsquerda = 0;
volatile unsigned long ultimaTransicaoEsquerda = 0;

unsigned long ultimoTempoAmostragem = 0;
int pwmEsquerda = 0;
float rpmFiltrado = 0.0;    // Guarda a velocidade suavizada
float acumuladorErro = 0.0; // Termo integral acumulado
float erroAnterior = 0.0;   // Guarda o erro do ciclo anterior para o termo derivativo

// =================================
// INTERRUPÇÃO (Filtro Anti-Ruído)
// =================================
void contarTransicaoEsquerda() {
  unsigned long agora = micros();
  if (agora - ultimaTransicaoEsquerda > 1000) {
    transicoesEsquerda++;
    ultimaTransicaoEsquerda = agora;
  }
}

void setup() {
  pinMode(DIR1, OUTPUT);
  pinMode(PWM1, OUTPUT);
  pinMode(SENSOR_ESQUERDO, INPUT);

  digitalWrite(DIR1, LOW);

  attachInterrupt(digitalPinToInterrupt(SENSOR_ESQUERDO), contarTransicaoEsquerda, CHANGE);

  Serial.begin(115200);
  delay(2000);

  Serial.println("Tempo_ms,Ensaio,Kd,Setpoint,RPM_Medido,PWM_Out");

  tempoInicioSubEnsaio = millis();
  ultimoTempoAmostragem = millis();
}

void loop() {
  if (testeConcluido) return;

  unsigned long agora = millis();

  // Executa apenas a cada INTERVALO_AMOSTRAGEM (150 ms)
  if (agora - ultimoTempoAmostragem >= INTERVALO_AMOSTRAGEM) {

    // 1. Leitura das transições com proteção de interrupção
    noInterrupts();
    unsigned long medidesEsq = transicoesEsquerda;
    transicoesEsquerda = 0;
    interrupts();

    // 2. Tempo decorrido
    float dt = (agora - ultimoTempoAmostragem) / 1000.0;
    ultimoTempoAmostragem = agora;

    // 3. Cálculo de RPM Instantâneo
    float rpmBruto = (medidesEsq / TRANSICOES_POR_ROTACAO) * (60.0 / dt);

    // 4. FILTRO PASSA-BAIXO (Exponential Moving Average)
    rpmFiltrado = (0.3 * rpmBruto) + (0.7 * rpmFiltrado);

    // 5. Gestão do Tempo do Ensaio
    unsigned long tempoNoSubEnsaio = agora - tempoInicioSubEnsaio;

    if (tempoNoSubEnsaio >= 12000) { // 10s de Teste + 2s de Pausa concluídos
      subEnsaio++;
      tempoInicioSubEnsaio = agora;
      tempoNoSubEnsaio = 0; // Reinicia contador interno do ciclo

      // Se já testou os 5 valores de Kd, encerra o ensaio
      if (subEnsaio >= 5) {
        testeConcluido = true;
        analogWrite(PWM1, 0);
        Serial.println("# FIM_TESTE");
        return;
      }
    }

    float Kd_atual = valoresKd[subEnsaio];

    // 6. Lógica de Controlo vs Pausa
    if (tempoNoSubEnsaio >= 10000) {
      // --- FASE DE PAUSA (Últimos 2 segundos) ---
      // Força motor parado, limpa o filtro e reseta os termos de memória
      pwmEsquerda = 0;
      rpmFiltrado = 0.0;
      acumuladorErro = 0.0;
      erroAnterior = rpmDesejado; // Prepara arranque limpo sem pico derivativo inicial
    } else {
      // --- FASE DE TESTE (Primeiros 10 segundos) ---
      // Controlador PID Ativo
      float erroEsq = rpmDesejado - rpmFiltrado;

      // Acumula o erro no tempo (Termo Integral)
      acumuladorErro += erroEsq * dt;
      acumuladorErro = constrain(acumuladorErro, -100.0, 100.0); // Anti-Windup

      // Variação do erro no tempo (Termo Derivativo)
      float derivadoErro = (erroEsq - erroAnterior) / dt;

      // Sinal de controlo PID
      float sinalEsq = (Kp_fixo * erroEsq) + (Ki_fixo * acumuladorErro) + (Kd_atual * derivadoErro);
      pwmEsquerda = constrain((int)sinalEsq, 0, 255);

      // Guarda o erro para o próximo ciclo
      erroAnterior = erroEsq;
    }

    // 7. Atuação no Motor
    analogWrite(PWM1, pwmEsquerda);

    // 8. Telemetria
    Serial.print(agora); Serial.print(",");
    Serial.print(subEnsaio + 1); Serial.print(",");
    Serial.print(Kd_atual, 2); Serial.print(",");
    Serial.print(rpmDesejado, 1); Serial.print(",");
    Serial.print(rpmFiltrado, 1); Serial.print(",");
    Serial.println(pwmEsquerda);
  }
}