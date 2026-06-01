const express = require("express");
const cors = require("cors");

const { createClient } = require("@supabase/supabase-js");

const app = express();

app.use(cors());
app.use(express.json());

const supabase = createClient(
  'https://lfkhyybivxccvqaoqyui.supabase.co',
  "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJpc3MiOiJzdXBhYmFzZSIsInJlZiI6Imxma2h5eWJpdnhjY3ZxYW9xeXVpIiwicm9sZSI6ImFub24iLCJpYXQiOjE3ODAzMzAwMjUsImV4cCI6MjA5NTkwNjAyNX0.MU2jpxAaAC8S8Fl2Ct9y0yopylqx4L26aR7iGrw4bso"
);

app.post("/dados", async (req, res) => {

  const {
    nivel,
    temperatura,
    umidade,
    luminosidade,
    presenca
  } = req.body;

  const { error } = await supabase
    .from("leituras")
    .insert([
      {
        nivel,
        temperatura,
        umidade,
        luminosidade,
        presenca
      }
    ]);

  if (error) {
    return res.status(500).json(error);
  }

  res.json({ status: "ok" });
});

app.listen(3001, () => {
  console.log("Servidor rodando na porta 3001");
});