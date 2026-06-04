// ============================================================
//  SISTEMA DE PINTURA – ESP32-S3-WROOM-1
//  Etapa 01 – Chão de Fábrica
//  SENAI CiMATEC – Camada de Serviço
// ============================================================
//  ⚠ POR QUE CONECTAR AO WIFI AQUI?
//    O ESP-NOW usa o canal de rádio WiFi. Se o ESP32 de
//    monitoramento conecta ao roteador (canal 6, por ex.) e
//    este ESP32 fica no canal padrão (1), os dois ficam em
//    canais diferentes e a comunicação falha.
//    Conectar os dois ao mesmo WiFi garante canal idêntico.
//    Este ESP32 NÃO precisa da internet – só do canal.
// ============================================================

#include <WiFi.h>
#include <esp_now.h>
#include "DHTesp.h"

// ── ⚙ WiFi (mesmo do monitoramento) ─────────────────────────
const char* WIFI_SSID = "SEU_WIFI";   // ← igual ao monitoramento
const char* WIFI_PASS = "SUA_SENHA";  // ← igual ao monitoramento

// ── ⚙ MAC do ESP32 de monitoramento ─────────────────────────
uint8_t MAC_MONITORAMENTO[6] = {0xE8, 0x31, 0xCD, 0xAD, 0x4A, 0x90};

// ── Pinos ────────────────────────────────────────────────────
#define PIN_TRIG        10
#define PIN_ECHO         9
#define PIN_DHT         16
#define PIN_LDR          7
#define PIN_PIR         13
#define PIN_LED_VERDE    2
#define PIN_LED_VERM     4

// ── Calibração do tanque ─────────────────────────────────────
#define TANQUE_VAZIO_CM  25.0f
#define TANQUE_CHEIO_CM   3.0f
#define NIVEL_ALERTA     20.0f

// ── Intervalo de envio ───────────────────────────────────────
#define INTERVALO_MS     2000UL

// ── Estrutura do pacote ESPNOW ───────────────────────────────
typedef struct __attribute__((packed)) {
  float   nivel_tinta;
  float   temperatura;
  float   umidade;
  int32_t luminosidade;
  uint8_t presenca;
  char    timestamp[24];
} DadosSensores;

// ── Objetos globais ──────────────────────────────────────────
DHTesp              dht;
esp_now_peer_info_t peerInfo;
DadosSensores       pacote;

volatile bool cbFeito   = false;
volatile bool cbSucesso = false;
unsigned long ultimaLeitura = 0;

// ============================================================
//  Callback de envio ESPNOW (core 3.x)
// ============================================================
void onEnvio(const wifi_tx_info_t* info, esp_now_send_status_t status) {
  cbFeito   = true;
  cbSucesso = (status == ESP_NOW_SEND_SUCCESS);
}

// ============================================================
//  WiFi com timeout (só para sincronizar canal)
// ============================================================
void conectarWifi() {
  Serial.print(F("[WiFi] Conectando para sincronizar canal: "));
  Serial.println(WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  int tentativas = 0;
  while (WiFi.status() != WL_CONNECTED && tentativas < 30) {
    delay(500); Serial.print('.'); tentativas++;
  }
  Serial.println();

  if (WiFi.status() == WL_CONNECTED) {
    Serial.print(F("[WiFi] Canal sincronizado: ")); Serial.println(WiFi.channel());
  } else {
    Serial.println(F("[WiFi] FALHA – ESPNOW pode nao funcionar se canais diferirem."));
  }
}

// ============================================================
//  Timestamp relativo à sessão
// ============================================================
void gerarTimestamp(char* buf, size_t tamanho) {
  unsigned long s = millis() / 1000;
  snprintf(buf, tamanho, "%02lu:%02lu:%02lu",
           (s / 3600) % 24, (s / 60) % 60, s % 60);
}

// ============================================================
//  Sensor ultrassônico
// ============================================================
float medirDistancia() {
  digitalWrite(PIN_TRIG, LOW);  delayMicroseconds(2);
  digitalWrite(PIN_TRIG, HIGH); delayMicroseconds(10);
  digitalWrite(PIN_TRIG, LOW);
  long dur = pulseIn(PIN_ECHO, HIGH, 30000UL);
  if (dur == 0) return -1.0f;
  return (dur * 0.0343f) / 2.0f;
}

float calcularNivel(float dist) {
  if (dist < 0.0f) return -1.0f;
  return constrain(((TANQUE_VAZIO_CM - dist) /
                    (TANQUE_VAZIO_CM - TANQUE_CHEIO_CM)) * 100.0f, 0.0f, 100.0f);
}

// ============================================================
//  LEDs
// ============================================================
void ledVerde()    { digitalWrite(PIN_LED_VERDE, HIGH); digitalWrite(PIN_LED_VERM, LOW);  }
void ledVermelho() { digitalWrite(PIN_LED_VERDE, LOW);  digitalWrite(PIN_LED_VERM, HIGH); }
void piscarLed(uint8_t pino, int vezes) {
  bool est = digitalRead(pino);
  for (int i = 0; i < vezes; i++) { digitalWrite(pino,LOW);delay(80);digitalWrite(pino,HIGH);delay(80); }
  digitalWrite(pino, est);
}

// ============================================================
//  SETUP
// ============================================================
void setup() {
  Serial.begin(115200);
  delay(1500);

  Serial.println(F("\n╔══════════════════════════════════════╗"));
  Serial.println(F("║  CHAO DE FABRICA – iniciando...      ║"));
  Serial.println(F("╚══════════════════════════════════════╝"));

  pinMode(PIN_TRIG,      OUTPUT);
  pinMode(PIN_ECHO,      INPUT);
  pinMode(PIN_PIR,       INPUT);
  pinMode(PIN_LED_VERDE, OUTPUT);
  pinMode(PIN_LED_VERM,  OUTPUT);
  digitalWrite(PIN_TRIG, LOW);

  ledVerde(); delay(400); ledVermelho(); delay(400); ledVerde();

  dht.setup(PIN_DHT, DHTesp::DHT22);
  delay(2500);

  Serial.println(F("[HW] Diagnostico inicial:"));
  Serial.print(F("     Ultrassonico: ")); Serial.print(medirDistancia()); Serial.println(F(" cm"));
  Serial.print(F("     LDR  (GPIO7): ")); Serial.println(analogRead(PIN_LDR));
  Serial.print(F("     PIR (GPIO13): ")); Serial.println(digitalRead(PIN_PIR));

  // WiFi para sincronizar canal com o monitoramento
  WiFi.mode(WIFI_STA);
  conectarWifi();
  Serial.print(F("[WiFi] MAC deste ESP32: ")); Serial.println(WiFi.macAddress());

  // ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println(F("[ESPNOW] ERRO! Reiniciando...")); delay(3000); ESP.restart();
  }
  esp_now_register_send_cb(onEnvio);

  memset(&peerInfo, 0, sizeof(peerInfo));
  memcpy(peerInfo.peer_addr, MAC_MONITORAMENTO, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println(F("[ESPNOW] ERRO ao adicionar peer!"));
  } else {
    Serial.println(F("[ESPNOW] Peer adicionado com sucesso."));
  }

  Serial.println(F("[Sistema] Pronto! Enviando a cada 2 s."));
  Serial.println();
}

// ============================================================
//  LOOP
// ============================================================
void loop() {
  unsigned long agora = millis();
  if (agora - ultimaLeitura < INTERVALO_MS) return;
  ultimaLeitura = agora;

  float distancia = medirDistancia();
  float nivel     = calcularNivel(distancia);
  bool  nivelErro = (nivel < 0.0f);

  TempAndHumidity th = dht.getTempAndHumidity();
  float temp    = isnan(th.temperature) ? 0.0f : th.temperature;
  float umidade = isnan(th.humidity)    ? 0.0f : th.humidity;

  long soma = 0;
  for (int i = 0; i < 5; i++) { soma += analogRead(PIN_LDR); delay(2); }
  int32_t ldrVal = soma / 5;

  bool p1 = (digitalRead(PIN_PIR) == HIGH); delay(10);
  bool p2 = (digitalRead(PIN_PIR) == HIGH);
  uint8_t pres = (p1 || p2) ? 1 : 0;

  Serial.println(F("========================================"));
  if (nivelErro) Serial.println(F("Nivel do tanque: ERRO (verifique sensor)"));
  else {
    Serial.print(F("Nivel do tanque: ")); Serial.print(nivel,1); Serial.println(F("%"));
    if (nivel < NIVEL_ALERTA) Serial.println(F("Alerta! Nivel de tinta baixo."));
  }
  Serial.print(F("Temperatura: ")); Serial.print(temp,1);
  Serial.print(F(" C | Umidade: ")); Serial.print(umidade,1); Serial.println(F("%"));
  Serial.print(F("Luminosidade: ")); Serial.println(ldrVal);
  Serial.println(pres ? F("Presenca detectada") : F("Sem presenca"));

  bool alerta = (!nivelErro && nivel < NIVEL_ALERTA);
  if (alerta) { ledVermelho(); Serial.println(F("Estado: Alerta - verificar tanque de tinta")); }
  else        { ledVerde();    Serial.println(F("Estado: Operacao normal")); }

  pacote.nivel_tinta  = nivelErro ? 0.0f : nivel;
  pacote.temperatura  = temp;
  pacote.umidade      = umidade;
  pacote.luminosidade = ldrVal;
  pacote.presenca     = pres;
  gerarTimestamp(pacote.timestamp, sizeof(pacote.timestamp));

  cbFeito = false;
  esp_err_t res = esp_now_send(MAC_MONITORAMENTO, (uint8_t*)&pacote, sizeof(pacote));

  if (res != ESP_OK) {
    Serial.println(F("[ESPNOW] Falha no envio (esp_now_send erro)"));
    piscarLed(PIN_LED_VERM, 3);
  } else {
    unsigned long t0 = millis();
    while (!cbFeito && millis() - t0 < 150) delay(1);
    if (cbSucesso) {
      Serial.printf("[ESPNOW] Enviado: {nivel=%.1f%% temp=%.1fC umd=%.1f%% lux=%d prs=%d}\n",
                    pacote.nivel_tinta, pacote.temperatura, pacote.umidade,
                    pacote.luminosidade, pacote.presenca);
      piscarLed(PIN_LED_VERDE, 2);
    } else {
      Serial.println(F("[ESPNOW] Falha no envio (sem confirmacao)"));
      piscarLed(PIN_LED_VERM, 3);
    }
  }
  Serial.println(F("========================================"));
}
