-- ============================================================
--  Corrigir tabela leituras no Supabase
--  Execute no SQL Editor: supabase.com → seu projeto → SQL Editor
-- ============================================================

-- Remove a tabela antiga (se existir) e recria com colunas corretas
DROP TABLE IF EXISTS leituras;

CREATE TABLE leituras (
  id               SERIAL       PRIMARY KEY,
  criado_em        TIMESTAMPTZ  NOT NULL DEFAULT NOW(),
  nivel_tinta      FLOAT        NOT NULL,
  temperatura      FLOAT        NOT NULL,
  umidade          FLOAT        NOT NULL,
  luminosidade     INTEGER      NOT NULL,
  presenca         SMALLINT     NOT NULL CHECK (presenca IN (0, 1)),
  timestamp_sessao TEXT
);

-- Índice para consultas por data
CREATE INDEX idx_leituras_criado_em ON leituras (criado_em DESC);

-- Permissões para a chave anon (usada pelo ESP32)
ALTER TABLE leituras ENABLE ROW LEVEL SECURITY;

CREATE POLICY "leitura publica"
  ON leituras FOR SELECT USING (true);

CREATE POLICY "insercao publica"
  ON leituras FOR INSERT WITH CHECK (true);
