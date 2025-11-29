require('dotenv').config();
const express = require('express');
const mysql = require('mysql2');
const cors = require('cors');

const app = express();
app.use(cors());
app.use(express.json()); // Important for parsing JSON body in POST requests

// --- CONFIGURACIÓN DE LA BASE DE DATOS (MODO POOL) ---
const db = mysql.createPool({
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

console.log('Configuración de Pool MySQL lista');

let ledState = false; // Estado del LED (false = apagado, true = encendido)

// --- ENDPOINT PARA RECIBIR DATOS DEL ESP8266 ---
// El ESP enviará los datos en la URL: /api/log?temp=XX&hum=YY&dist=ZZ
app.get('/api/log', (req, res) => {
    // Desestructuramos los datos que vienen en la URL
    const { temp, hum, dist } = req.query;

    console.log(`Recibido -> Temp: ${temp}, Hum: ${hum}, Dist: ${dist}`);

    if (!temp || !hum || !dist) {
        return res.status(400).send('Faltan datos');
    }

    const query = 'INSERT INTO registros (temperatura, humedad, distancia) VALUES (?, ?, ?)';

    db.query(query, [temp, hum, dist], (err, result) => {
        if (err) {
            console.error(err);
            res.status(500).send('Error al guardar en BD');
        } else {
            // --- LIMPIEZA DE DATOS ANTIGUOS ---
            // Mantenemos solo los ultimos 100 registros
            const cleanupQuery = `
                DELETE FROM registros 
                WHERE id NOT IN (
                    SELECT id FROM (
                        SELECT id FROM registros ORDER BY id DESC LIMIT 100
                    ) sub
                )
            `;

            db.query(cleanupQuery, (cleanupErr) => {
                if (cleanupErr) {
                    console.error("Error limpiando registros antiguos:", cleanupErr);
                    // No fallamos la petición principal, solo logueamos el error
                } else {
                    console.log("Limpieza de registros completada");
                }
            });

            // Respondemos con el estado del LED para que el ESP lo lea
            res.json({
                status: 'ok',
                message: 'Datos guardados correctamente',
                led: ledState
            });
        }
    });
});

// --- ENDPOINT PARA OBTENER DATOS ---
app.get('/api/data', (req, res) => {
    const query = 'SELECT * FROM registros ORDER BY id DESC LIMIT 100';
    db.query(query, (err, results) => {
        if (err) {
            console.error(err);
            res.status(500).send('Error al obtener datos');
        } else {
            res.json(results);
        }
    });
});

// --- ENDPOINT PARA OBTENER EL ULTIMO REGISTRO (PARA EL ESP ACTUADOR) ---
app.get('/api/latest', (req, res) => {
    const query = 'SELECT * FROM registros ORDER BY id DESC LIMIT 1';
    db.query(query, (err, results) => {
        if (err) {
            console.error(err);
            res.status(500).send('Error al obtener datos');
        } else {
            if (results.length > 0) {
                res.json(results[0]);
            } else {
                res.json({}); // Retornar objeto vacio si no hay datos
            }
        }
    });
});

// --- ENDPOINT PARA CONTROLAR EL LED DESDE EL DASHBOARD ---
app.post('/api/led', (req, res) => {
    const { state } = req.body; // Esperamos { "state": true/false }
    if (typeof state === 'boolean') {
        ledState = state;
        console.log(`Estado del LED actualizado a: ${ledState}`);
        res.json({ status: 'ok', led: ledState });
    } else {
        res.status(400).json({ error: 'Formato inválido. Se espera { "state": boolean }' });
    }
});

// --- ENDPOINT PARA OBTENER ESTADO DEL LED (DASHBOARD) ---
app.get('/api/led', (req, res) => {
    res.json({ led: ledState });
});

const PORT = process.env.PORT || 3000;
app.listen(PORT, '0.0.0.0', () => {
    console.log(`Servidor corriendo en puerto ${PORT}`);
});
