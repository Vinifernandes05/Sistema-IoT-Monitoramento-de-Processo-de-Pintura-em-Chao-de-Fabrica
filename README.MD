# Sistema IoT de Monitoramento de Pintura de Blocos de Madeira

Projeto desenvolvido no **SENAI CiMATEC** para a disciplina de Práticas Integradas – Camada de Serviço (AV2).

Sistema IoT que monitora o processo de pintura de blocos de madeira em um ambiente simulado de chão de fábrica, utilizando dois ESP32 que se comunicam via **ESP-NOW**, com dados armazenados no **Supabase** e visualizados em dashboard em tempo real.

---

## Estrutura do Repositório

```
iot-sistema-pintura/
├── chao_de_fabrica/
│   └── chao_de_fabrica.ino      # Firmware ESP32-S3 (sensores + envio ESP-NOW)
├── monitoramento/
│   └── monitoramento.ino        # Firmware ESP32-WROOM-32 (recepção + Supabase)
├── banco_de_dados/
│   └── criar_tabela.sql         # SQL para criar tabela no Supabase
├── docs/
│   ├── montagem.md              # Guia de montagem dos circuitos
│   └── supabase.md              # Configuração do banco de dados
└── README.md
```

---

## 🏗️ Arquitetura do Sistema

```
[Sensores] ──► [ESP32-S3 Chão de Fábrica]
                        │
                    ESP-NOW
                        │
               [ESP32-WROOM-32 Monitoramento]
                        │
                  HTTPS / REST
                        │
                   [Supabase]
                        │
                  [Dashboard Lovable]
```

---

## Componentes

### ESP32-S3-WROOM-1 — Chão de Fábrica

| Componente | Pino | Função |
|---|---|---|
| HC-SR04 TRIG | GPIO 10 | Pulso de disparo do ultrassônico |
| HC-SR04 ECHO | GPIO 9 | Recepção do eco (usar divisor de tensão se 5V) |
| DHT22 DATA | GPIO 16 | Temperatura e umidade |
| LDR | GPIO 7 | Luminosidade (ADC1_CH6) |
| PIR OUT | GPIO 13 | Detecção de presença |
| LED Verde | GPIO 2 | Operação normal |
| LED Vermelho | GPIO 4 | Alerta (nível < 20%) |

### ESP32-WROOM-32 — Monitoramento

| Componente | Pino | Função |
|---|---|---|
| LED Verde | GPIO 2 | Comunicação ESP-NOW OK |
| LED Vermelho | GPIO 4 | Timeout (sem dados por 5 s) |

---

## Como Montar o Projeto

Consulte o guia completo em [`docs/montagem.md`](docs/montagem.md).

**Resumo:**
1. Monte o circuito do chão de fábrica conforme a tabela de pinos acima
2. Ligação do LDR: `3.3V ──[LDR]── GPIO7 ──[10kΩ]── GND`
3. PIR: VCC no 5V, GND no GND, OUT no GPIO 13
4. HC-SR04: se o módulo for 5V, use divisor de tensão no pino ECHO
5. Monte o circuito do monitoramento (apenas dois LEDs)

---

## Como Gravar os Firmwares

### Pré-requisitos

- Arduino IDE 2.x
- Pacote ESP32 da Espressif (versão ≥ 3.x)

### Bibliotecas (instalar pelo Library Manager)

| Biblioteca | Autor | Para qual ESP32 |
|---|---|---|
| DHT sensor library for ESPx | beegee-tokyo | Chão de fábrica |

### Configurações da Board

**Chão de fábrica (ESP32-S3-WROOM-1):**
- Board: `ESP32S3 Dev Module`
- Flash Size: `16MB`
- USB CDC On Boot: `Disabled`

**Monitoramento (ESP32-WROOM-32):**
- Board: `ESP32-WROOM-DA Module` ou `ESP32 Dev Module`
- Upload Speed: `921600`

### Ordem de gravação

1. Grave `monitoramento.ino` no ESP32-WROOM-32
2. Abra o Monitor Serial (115200) → copie o MAC exibido em `[WiFi] MAC deste ESP32: XX:XX:XX:XX:XX:XX`
3. Cole esse MAC no campo `MAC_MONITORAMENTO` em `chao_de_fabrica.ino`
4. Grave `chao_de_fabrica.ino` no ESP32-S3

### Variáveis a preencher antes de gravar

**`chao_de_fabrica.ino`:**
```cpp
const char* WIFI_SSID = "SEU_WIFI";
const char* WIFI_PASS = "SUA_SENHA";
uint8_t MAC_MONITORAMENTO[6] = {0xXX, 0xXX, 0xXX, 0xXX, 0xXX, 0xXX};
```

**`monitoramento.ino`:**
```cpp
const char* WIFI_SSID    = "SEU_WIFI";
const char* WIFI_PASS    = "SUA_SENHA";
const char* SUPABASE_URL = "https://SEU_PROJETO.supabase.co/rest/v1/leituras";
const char* SUPABASE_KEY = "SUA_ANON_KEY";
```

---

## Como Configurar o Banco de Dados

Consulte o guia completo em [`docs/supabase.md`](docs/supabase.md).

**Resumo:**
1. Crie um projeto em [supabase.com](https://supabase.com)
2. Acesse **SQL Editor → New Query**
3. Cole e execute o conteúdo de `banco_de_dados/criar_tabela.sql`
4. Em **Settings → API**, copie a Project URL e a anon key

---

## Como Consultar os Dados Salvos

**Via Supabase Table Editor:**
- Acesse seu projeto → **Table Editor → leituras**

**Via SQL (Supabase SQL Editor):**
```sql
-- Últimas 20 leituras
SELECT * FROM leituras ORDER BY criado_em DESC LIMIT 20;

-- Média de temperatura das últimas 24h
SELECT AVG(temperatura) FROM leituras
WHERE criado_em > NOW() - INTERVAL '24 hours';

-- Alertas de nível baixo
SELECT * FROM leituras
WHERE nivel_tinta < 20
ORDER BY criado_em DESC;
```

**Via API REST:**
```bash
curl "https://SEU_PROJETO.supabase.co/rest/v1/leituras?order=criado_em.desc&limit=10" \
  -H "apikey: SUA_ANON_KEY"
```

**Via Dashboard:**
- Acesse o dashboard criado no Lovable para visualização em tempo real

---

## Estrutura do Pacote ESP-NOW

Enviado a cada 2 segundos do chão de fábrica para o monitoramento:

```cpp
typedef struct {
  float   nivel_tinta;      // % do tanque (0–100)
  float   temperatura;      // °C
  float   umidade;          // %
  int32_t luminosidade;     // ADC 0–4095
  uint8_t presenca;         // 0 = ausente, 1 = detectada
  char    timestamp[24];    // "HH:MM:SS" relativo ao boot
} DadosSensores;
```

---

## Avaliação

| Etapa | Descrição | Pontos |
|---|---|---|
| 01 | ESP32 Chão de Fábrica | 2,0 |
| 02 | ESP32 Monitoramento | 2,0 |
| 03 | Banco de Dados e Dashboard | 2,0 |
| 04 | GitHub | 2,0 |
| 05 | Apresentação oral | 2,0 |
| | **Total** | **10,0** |

---

## Referências

- [Arduino IDE](https://www.arduino.cc/en/software)
- [Espressif ESP32 Technical Reference](https://www.espressif.com/en/products/socs/esp32-s3)
- [Supabase Documentation](https://supabase.com/docs)
- [Random Nerd Tutorials – ESP-NOW](https://randomnerdtutorials.com/esp-now-esp32-arduino-ide/)

---
