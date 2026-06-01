# 🏭 Sistema IoT para Monitoramento de Processo de Pintura em Chão de Fábrica

## 📌 Sobre o Projeto

Este projeto foi desenvolvido na disciplina de **Práticas Integradas: Camada de Serviço**, com o objetivo de simular um sistema de monitoramento de um processo de pintura em ambiente industrial utilizando dispositivos IoT.

A solução utiliza dois microcontroladores ESP32 comunicando-se via **ESPNOW**, permitindo a coleta, transmissão, armazenamento e visualização de dados em tempo real.

O sistema monitora variáveis importantes do processo de pintura, como nível do tanque de tinta, temperatura, umidade, luminosidade e presença de operadores próximos à área de trabalho.

---

## 🎯 Objetivos

- Implementar comunicação sem fio utilizando ESPNOW.
- Realizar aquisição de dados através de sensores.
- Desenvolver um backend para armazenamento das leituras.
- Persistir informações em banco de dados.
- Criar dashboards para monitoramento em tempo real.
- Aplicar conceitos de IoT, APIs REST, Banco de Dados e Visualização de Dados.

---

## 🛠️ Tecnologias Utilizadas

### Hardware

- ESP32 (Chão de Fábrica)
- ESP32 (Monitoramento)
- Sensor Ultrassônico HC-SR04
- Sensor DHT11/DHT22
- Sensor PIR
- Fotorresistor (LDR)
- LEDs de Status
- Matriz de LEDs

### Software

- Arduino IDE
- ESPNOW
- Node.js
- Express.js
- Supabase
- Grafana
- Git
- GitHub

---

## 🏗️ Arquitetura do Sistema

```text
ESP32 Chão de Fábrica
        │
        ▼
      ESPNOW
        │
        ▼
ESP32 Monitoramento
        │
        ▼
    API REST
(Node.js + Express)
        │
        ▼
     Supabase
(PostgreSQL)
        │
        ▼
      Grafana
```

---

## 📡 ESP32 Chão de Fábrica

Responsável pela coleta dos dados dos sensores instalados no ambiente industrial.

### Sensores Utilizados

- Sensor Ultrassônico → Nível do tanque de tinta.
- Sensor DHT → Temperatura e umidade.
- Fotorresistor (LDR) → Luminosidade do ambiente.
- Sensor PIR → Detecção de presença.

### Funcionalidades

- Leitura contínua dos sensores.
- Acionamento de LEDs de status.
- Monitoramento do nível do tanque.
- Envio periódico dos dados via ESPNOW.

### Dados Enviados

- Nível do tanque (%)
- Temperatura (°C)
- Umidade (%)
- Luminosidade
- Presença
- Timestamp

---

## 🖥️ ESP32 de Monitoramento

Responsável por receber os dados enviados pelo ESP32 principal.

### Funcionalidades

- Recepção de dados via ESPNOW.
- Exibição dos dados na matriz de LEDs.
- Indicação visual de falhas de comunicação.
- Encaminhamento das informações para o backend.

---

## 🗄️ Banco de Dados

O projeto utiliza o Supabase para armazenamento das leituras coletadas pelos sensores.

### Estrutura da Tabela

```sql
CREATE TABLE leituras (
    id BIGSERIAL PRIMARY KEY,
    timestamp TIMESTAMPTZ DEFAULT NOW(),
    nivel NUMERIC,
    temperatura NUMERIC,
    umidade NUMERIC,
    luminosidade NUMERIC,
    presenca BOOLEAN
);
```

---

## 🚀 Backend

O backend foi desenvolvido utilizando Node.js e Express.

### Funcionalidades

- Receber dados enviados pelo ESP32.
- Validar informações recebidas.
- Armazenar leituras no banco de dados.
- Disponibilizar API REST para integração.

### Endpoint

```http
POST /dados
```

### Exemplo de Requisição

```json
{
  "nivel": 80,
  "temperatura": 25,
  "umidade": 60,
  "luminosidade": 400,
  "presenca": true
}
```

---

## ▶️ Como Executar o Backend

### Instalar Dependências

```bash
npm install
```

### Executar o Servidor

```bash
node backend.js
```

Saída esperada:

```text
Servidor rodando na porta 3001
```

---

## 📊 Dashboard Grafana

O Grafana é utilizado para visualização dos dados armazenados no banco.

### Indicadores Monitorados

- 📦 Nível do tanque de tinta
- 🌡️ Temperatura
- 💧 Umidade
- 💡 Luminosidade
- 👤 Presença

### Recursos

- Atualização automática.
- Histórico das leituras.
- Monitoramento em tempo real.
- Visualização gráfica dos indicadores.

---

## 📂 Estrutura do Projeto

```text
chao-de-fabrica/
│
├── Backend/
│   ├── backend.js
│   ├── package.json
│   └── node_modules/
│
├── ESP32_ChaoDeFabrica/
│   └── codigo.ino
│
├── ESP32_Monitoramento/
│   └── codigo.ino
│
└── README.md
```

---

## 📚 Conceitos Aplicados

- Internet das Coisas (IoT)
- Comunicação ESPNOW
- Sistemas Embarcados
- APIs REST
- Banco de Dados em Nuvem
- Monitoramento Industrial
- Visualização de Dados
- Versionamento com Git e GitHub

---

## 👨‍💻 Autores

**Vinicius Fernandes**
**Rafael Guerra**
**Lucca Badaró**

Estudantes de Engenharia da Computação.