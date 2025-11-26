require('dotenv').config(); // <--- Agrega esto al mero principio
const express = require('express');
const mysql = require('mysql2');
const cors = require('cors'); // Para evitar bloqueos si accedes desde web

const app = express();
app.use(cors());

// --- CONFIGURACIÓN DE LA BASE DE DATOS ---
// --- CONFIGURACIÓN DE LA BASE DE DATOS (MODO POOL) ---
const db = mysql.createPool({ // <--- CAMBIO AQUÍ: createPool en lugar de createConnection
    host: process.env.DB_HOST,
    user: process.env.DB_USER,
    password: process.env.DB_PASSWORD,
    database: process.env.DB_NAME,
    port: process.env.DB_PORT,
    waitForConnections: true,
    connectionLimit: 10,
    queueLimit: 0,
    ssl: {
        rejectUnauthorized: false
    }
});

// Nota: Con pool ya no necesitas llamar a db.connect(), lo hace solo.
console.log('Configuración de Pool MySQL lista');


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
// Busca la parte final de tu código y cámbiala por esto:

const PORT = process.env.PORT || 3000; // <--- CAMBIO CLAVE: Usa el puerto de Render si existe
app.listen(PORT, '0.0.0.0', () => {    // <--- CAMBIO CLAVE: '0.0.0.0' asegura que escuche hacia afuera
    console.log(`Servidor corriendo en puerto ${PORT}`);
});
