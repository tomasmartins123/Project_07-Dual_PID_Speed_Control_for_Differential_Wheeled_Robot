// ============================================================================
// PROJETO CRIA 7 - Ensaio Automático Kp com Filtro Passa-Baixo (EMA)
// ============================================================================

const int DIR1 = A0;
const int PWM1 = 5;
const int SENSOR_ESQUERDO = 2;

const float TRANSICOES_POR_ROTACAO = 40.0;
const unsigned long INTERVALO_AMOSTRAGEM = 150; // 150 ms para melhor resolução (10 RPM por salto)

float rpmDesejado = 100.0;

// --- TESTE AUTOMÁTICO DE KP ---
float valoresKp[5] = {1, 1.5, 2 , 2.5, 3};
int subEnsaio = 0;
bool testeConcluido = false;
unsigned long tempoInicioSubEnsaio = 0;

// --- VARIÁVEIS DO SENSOR ---
volatile unsigned long transicoesEsquerda = 0;
volatile unsigned long ultimaTransicaoEsquerda = 0;

unsigned long ultimoTempoAmostragem = 0;
int pwmEsquerda = 0;
float rpmFiltrado = 0.0; // Guarda a velocidade suavizada

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

  Serial.println("Tempo_ms,Ensaio,Kp,Setpoint,RPM_Medido,PWM_Out");

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

    if (tempoNoSubEnsaio >= 7000) { // 5s de Teste + 2s de Pausa concluídos
      subEnsaio++;
      tempoInicioSubEnsaio = agora;
      tempoNoSubEnsaio = 0; // Reinicia contador interno do ciclo

      // Se já testou os 5 valores de Kp, encerra o ensaio
      if (subEnsaio >= 5) {
        testeConcluido = true;
        analogWrite(PWM1, 0);
        Serial.println("# FIM_TESTE");
        return;
      }
    }

    float Kp_atual = valoresKp[subEnsaio];

    // 6. Lógica de Controlo vs Pausa
    if (tempoNoSubEnsaio >= 5000) {
      // --- FASE DE PAUSA (Últimos 2 segundos) ---
      // Força motor parado e limpa a memória do filtro para um arranque limpo no próximo Kp
      pwmEsquerda = 0;
      rpmFiltrado = 0.0;
    } else {
      // --- FASE DE TESTE (Primeiros 5 segundos) ---
      // Controlador Proporcional Ativo
      float erroEsq = rpmDesejado - rpmFiltrado;
      float sinalEsq = Kp_atual * erroEsq;
      pwmEsquerda = constrain((int)sinalEsq, 0, 255);
    }

    // 7. Atuação no Motor
    analogWrite(PWM1, pwmEsquerda);

    // 8. Telemetria
    Serial.print(agora); Serial.print(",");
    Serial.print(subEnsaio + 1); Serial.print(",");
    Serial.print(Kp_atual, 2); Serial.print(",");
    Serial.print(rpmDesejado, 1); Serial.print(",");
    Serial.print(rpmFiltrado, 1); Serial.print(",");
    Serial.println(pwmEsquerda);
  }
}