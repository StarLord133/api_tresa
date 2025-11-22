const express = require('express');
const mysql = require('mysql2');
const cors = require('cors'); // Para evitar bloqueos si accedes desde web

const app = express();
app.use(cors());

// --- CONFIGURACIÓN DE LA BASE DE DATOS ---
const db = mysql.createConnection({
    host: 'mysql-2288c034-proyecto130607.b.aivencloud.com',
    user: 'avnadmin',      // Tu usuario de SQL
    password: 'AVNS_dPJpPMWi0fA73thaJ5j',      // Tu contraseña de SQL
    database: 'defaultdb'
});

db.connect((err) => {
    if (err) {
        console.error('Error conectando a MySQL:', err);
        return;
    }
    console.log('Conectado a MySQL');
});

// --- ENDPOINT PARA RECIBIR DATOS DEL ESP8266 ---
// El ESP enviará los datos en la URL: /api/log?temp=XX&hum=YY&dist=ZZ&mov=A
app.get('/api/log', (req, res) => {
    // Desestructuramos los datos que vienen en la URL
    const { temp, hum, dist, mov } = req.query;

    console.log(`Recibido -> Temp: ${temp}, Hum: ${hum}, Dist: ${dist}, Mov: ${mov}`);

    if (!temp || !hum || !dist || !mov) {
        return res.status(400).send('Faltan datos');
    }

    const query = 'INSERT INTO registros (temperatura, humedad, distancia, movimiento) VALUES (?, ?, ?, ?)';
    
    db.query(query, [temp, hum, dist, mov], (err, result) => {
        if (err) {
            console.error(err);
            res.status(500).send('Error al guardar en BD');
        } else {
            res.send('Datos guardados correctamente');
        }
    });
});

// Iniciar servidor
const PORT = 3000;
app.listen(PORT, () => {
    console.log(`Servidor corriendo en puerto ${PORT}`);
});