# 🌐📡 Sistema IoT de Monitoramento de Pintura de Blocos de Madeira

## 📌 Sobre o Projeto

Este projeto consiste no desenvolvimento de uma solução completa de **Internet das Coisas (IoT)** para monitoramento do processo de pintura de blocos de madeira em um ambiente simulado de chão de fábrica.

A arquitetura foi construída utilizando **dois ESP32**, responsáveis pela coleta e transmissão das informações através do protocolo **ESP-NOW**, permitindo o monitoramento em tempo real de variáveis como:

- Nível de tinta
- Temperatura
- Umidade
- Luminosidade
- Presença

Os dados são enviados para o **Supabase**, onde permanecem armazenados para consultas históricas e visualização através de um **Dashboard Web**.

Além do desenvolvimento dos firmwares embarcados, o projeto contempla documentação técnica completa, banco de dados, arquitetura do sistema e organização do código utilizando Git e GitHub.

🔗 **Dashboard do Projeto:** https://readings-radiant-view.lovable.app/

---

## 🎯 Objetivos do Projeto

- Desenvolver uma solução completa de Internet das Coisas (IoT)
- Integrar hardware e software
- Aplicar comunicação entre microcontroladores utilizando ESP-NOW
- Desenvolver firmware para ESP32
- Armazenar dados em banco de dados na nuvem
- Desenvolver Dashboard para monitoramento em tempo real
- Trabalhar com APIs REST

---

## 🚀 Tecnologias Utilizadas

### 🔹 Hardware

- ESP32-S3-WROOM-1
- ESP32-WROOM-32
- HC-SR04
- DHT22
- Sensor PIR
- LDR
- LEDs

---

### 🔹 Firmware

- C++
- Arduino IDE
- ESP-NOW

---

### 🔹 Banco de Dados

- Supabase

---

### 🔹 Dashboard

- Lovable IA
- HTML
- CSS
- TypeScript

---

### 🔹 Outros

- Git
- GitHub
- HTTPS
- REST API

---

## ⚙️ Funcionalidades

✔️ Coleta automática das informações dos sensores

✔️ Comunicação entre dois ESP32 utilizando ESP-NOW

✔️ Monitoramento do nível de tinta

✔️ Monitoramento de temperatura

✔️ Monitoramento da umidade

✔️ Monitoramento da luminosidade

✔️ Monitoramento de presença

✔️ Persistência das leituras no Supabase

✔️ Dashboard Web em tempo real

✔️ Histórico completo das leituras

✔️ Indicadores estatísticos

✔️ Gráficos históricos

✔️ Atualização automática dos dados

---

## 🏗️ Arquitetura do Sistema

```text
[Sensores]
      │
      ▼
ESP32-S3 (Chão de Fábrica)
      │
   ESP-NOW
      │
      ▼
ESP32-WROOM-32 (Monitoramento)
      │
 HTTPS / REST API
      │
      ▼
   Supabase
      │
      ▼
Dashboard Web
```

---

## 📂 Estrutura do Projeto

```text
Sistema-IoT-Monitoramento/
│
├── Backend/
│   ├── package.json             # Dependências do backend
│   └── package-lock.json        # Controle de versões das dependências
│
├── .gitignore                   # Arquivos ignorados pelo Git
├── README.md                    # Documentação do projeto
├── chao_de_fabrica.ino          # Firmware do ESP32-S3 (Coleta dos sensores e envio via ESP-NOW)
├── monitoramento.ino            # Firmware do ESP32-WROOM-32 (Recepção dos dados e integração com Supabase)
└── criar_tabela.sql             # Script SQL para criação da tabela no Supabase
```

---

## 🔌 Componentes Utilizados

## ESP32-S3-WROOM-1 — Chão de Fábrica

| Componente | Pino | Função |
|------------|------|---------|
| HC-SR04 TRIG | GPIO 10 | Pulso de disparo do ultrassônico |
| HC-SR04 ECHO | GPIO 9 | Recepção do eco (usar divisor de tensão se 5V) |
| DHT22 DATA | GPIO 16 | Temperatura e Umidade |
| LDR | GPIO 7 | Luminosidade (ADC1_CH6) |
| PIR OUT | GPIO 13 | Detecção de presença |
| LED Verde | GPIO 2 | Operação normal |
| LED Vermelho | GPIO 4 | Alerta (nível < 20%) |

---

## ESP32-WROOM-32 — Monitoramento

| Componente | Pino | Função |
|------------|------|---------|
| LED Verde | GPIO 2 | Comunicação ESP-NOW OK |
| LED Vermelho | GPIO 4 | Timeout (sem dados por 5 s) |

---

## 📞 Contato

👤 **Vinicius Sousa Fernandes**

📧 Email: vinifernandes2005@gmail.com

💼 LinkedIn: https://linkedin.com/in/viniciussousaf

💻 GitHub: https://github.com/Vinifernandes05
