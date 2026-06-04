// ============================================================
//  SISTEMA DE PINTURA – ESP32-WROOM-32
//  Etapa 02 – Monitoramento (ESPNOW + Supabase)
//  SENAI CiMATEC – Camada de Serviço
// ============================================================
//  MAPA DE PINOS:
//    GPIO 2   → LED Verde    (ESPNOW OK / dados recebidos)
//    GPIO 4   → LED Vermelho (timeout / sem dados)
// ============================================================

#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>   // ← obrigatório para HTTPS
#include <esp_now.h>

// ── ⚙ WiFi ──────────────────────────────────────────────────
const char* WIFI_SSID = "iPhone";
const char* WIFI_PASS = "12345678";

// ── ⚙ Supabase ───────────────────────────────────────────────
// ⚠ URL deve terminar com o nome da tabela, sem barra final
const char* SUPABASE_URL = "https://lfkhyybivxccvqaoqyui.supabase.co/rest/v1/leituras";
const char* SUPABASE_KEY = "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJpc3MiOiJzdXBhYmFzZSIsInJlZiI6Imxma2h5eWJpdnhjY3ZxYW9xeXVpIiwicm9sZSI6ImFub24iLCJpYXQiOjE3ODAzMzAwMjUsImV4cCI6MjA5NTkwNjAyNX0.MU2jpxAaAC8S8Fl2Ct9y0yopylqx4L26aR7iGrw4bso";

// ── ⚙ MAC do ESP32 chão de fábrica ──────────────────────────
uint8_t MAC_CHAO_FABRICA[6] = {0x80, 0xB5, 0x4E, 0xC6, 0x55, 0xA0};

// ── Pinos ────────────────────────────────────────────────────
#define PIN_LED_VERDE  2
#define PIN_LED_VERM   4

// ── Timeout de comunicação ───────────────────────────────────
#define TIMEOUT_RX_MS  5000UL

// ── Estrutura do pacote ESPNOW ───────────────────────────────
typedef struct __attribute__((packed)) {
  float   nivel_tinta;
  float   temperatura;
  float   umidade;
  int32_t luminosidade;
  uint8_t presenca;
  char    timestamp[24];
} DadosSensores;

// ── Estado global ────────────────────────────────────────────
DadosSensores dadosParaEnvio;
volatile bool dadosNovos  = false;
bool          dadosValidos = false;
unsigned long lastRxMillis = 0;

// ============================================================
//  LEDs
// ============================================================
void ledVerde()    { digitalWrite(PIN_LED_VERDE, HIGH); digitalWrite(PIN_LED_VERM, LOW);  }
void ledVermelho() { digitalWrite(PIN_LED_VERDE, LOW);  digitalWrite(PIN_LED_VERM, HIGH); }

// ============================================================
//  WiFi com timeout
// ============================================================
void conectarWifi() {
  Serial.print(F("[WiFi] Conectando a: "));
  Serial.println(WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  int tentativas = 0;
  while (WiFi.status() != WL_CONNECTED && tentativas < 30) {
    delay(500);
    Serial.print('.');
    tentativas++;
  }
  Serial.println();

  if (WiFi.status() == WL_CONNECTED) {
    Serial.print(F("[WiFi] Conectado! IP: ")); Serial.println(WiFi.localIP());
    Serial.print(F("[WiFi] Canal: "));         Serial.println(WiFi.channel());
  } else {
    Serial.println(F("[WiFi] FALHA na conexao. Supabase indisponivel."));
  }
}

// ============================================================
//  Callback de recepção ESPNOW (core 3.x)
// ============================================================
void onReceive(const esp_now_recv_info_t* recv_info,
               const uint8_t* data, int len) {

  const uint8_t* mac_addr = recv_info->src_addr;

  // Validar MAC (ignora se 0xFF = aceita qualquer um)
  bool broadcast = true;
  for (int i = 0; i < 6; i++) {
    if (MAC_CHAO_FABRICA[i] != 0xFF) { broadcast = false; break; }
  }
  if (!broadcast && memcmp(mac_addr, MAC_CHAO_FABRICA, 6) != 0) return;
  if (len != (int)sizeof(DadosSensores)) return;

  memcpy(&dadosParaEnvio, data, sizeof(DadosSensores));
  lastRxMillis = millis();
  dadosNovos   = true;
  dadosValidos = true;
}

// ============================================================
//  Enviar dados ao Supabase via HTTPS POST
// ============================================================
void enviarSupabase(const DadosSensores& d) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println(F("[Supabase] Sem WiFi – dado nao enviado."));
    return;
  }

  // Monta JSON com os nomes das colunas da tabela
  char json[256];
  snprintf(json, sizeof(json),
    "{\"nivel_tinta\":%.1f"
    ",\"temperatura\":%.1f"
    ",\"umidade\":%.1f"
    ",\"luminosidade\":%d"
    ",\"presenca\":%d"
    ",\"timestamp_sessao\":\"%s\"}",
    d.nivel_tinta,
    d.temperatura,
    d.umidade,
    (int)d.luminosidade,
    (int)d.presenca,      // ← %d correto para uint8_t
    d.timestamp);

  Serial.print(F("[Supabase] Enviando: ")); Serial.println(json);

  // Cliente SSL sem verificação de certificado (necessário para HTTPS no ESP32)
  WiFiClientSecure client;
  client.setInsecure();

  HTTPClient http;
  http.begin(client, SUPABASE_URL);
  http.addHeader("Content-Type",  "application/json");
  http.addHeader("apikey",        SUPABASE_KEY);
  http.addHeader("Authorization", String("Bearer ") + SUPABASE_KEY);
  http.addHeader("Prefer",        "return=minimal");
  http.setTimeout(10000);

  int codigo = http.POST(json);

  if (codigo == 201 || codigo == 200) {
    Serial.println(F("[Supabase] Leitura salva com sucesso!"));
  } else {
    Serial.print(F("[Supabase] Erro HTTP: ")); Serial.println(codigo);
    Serial.print(F("[Supabase] Resposta:  ")); Serial.println(http.getString());
  }
  http.end();
}

// ============================================================
//  SETUP
// ============================================================
void setup() {
  Serial.begin(115200);
  delay(1500);

  Serial.println(F("\n╔══════════════════════════════════════╗"));
  Serial.println(F("║  ESP32 MONITORAMENTO – iniciando     ║"));
  Serial.println(F("╚══════════════════════════════════════╝"));

  pinMode(PIN_LED_VERDE, OUTPUT);
  pinMode(PIN_LED_VERM,  OUTPUT);

  // Teste de LEDs
  ledVerde();    delay(400);
  ledVermelho(); delay(400);
  ledVermelho(); // estado inicial: aguardando dados

  // WiFi (deve conectar ANTES do esp_now_init para sincronizar canal)
  WiFi.mode(WIFI_STA);
  conectarWifi();

  // ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println(F("[ESPNOW] ERRO ao inicializar! Reiniciando..."));
    delay(3000); ESP.restart();
  }
  esp_now_register_recv_cb(onReceive);

  Serial.print(F("[WiFi] MAC deste ESP32: ")); Serial.println(WiFi.macAddress());
  Serial.println(F("[Sistema] Pronto! Aguardando dados do chao de fabrica."));
  Serial.println();
}

// ============================================================
//  LOOP
// ============================================================
void loop() {
  unsigned long agora = millis();

  // Pacote recebido via ESPNOW
  if (dadosNovos) {
    dadosNovos = false;
    ledVerde();

    Serial.println(F("LED VERDE ON - comunicacao OK"));
    Serial.printf("RX: nivel=%.0f%% temp=%.0fC umd=%.0f%% lux=%d prs=%d ts=%s\n",
                  dadosParaEnvio.nivel_tinta, dadosParaEnvio.temperatura,
                  dadosParaEnvio.umidade,     (int)dadosParaEnvio.luminosidade,
                  dadosParaEnvio.presenca,    dadosParaEnvio.timestamp);

    // Envia ao Supabase (feito aqui no loop, nunca dentro do callback)
    enviarSupabase(dadosParaEnvio);
  }

  // Timeout: sem dados por 5 s → LED vermelho
  if (dadosValidos && (agora - lastRxMillis > TIMEOUT_RX_MS)) {
    ledVermelho();
    Serial.println(F("LED VERMELHO ON - timeout de comunicacao"));
    dadosValidos = false;
  }
}
