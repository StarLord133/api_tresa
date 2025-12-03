require('dotenv').config();
const express = require('express');
const mysql = require('mysql2');
const cors = require('cors');
const axios = require('axios');

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

// Crear tabla de grabaciones si no existe
const createTableQuery = `
    CREATE TABLE IF NOT EXISTS grabaciones (
        id INT AUTO_INCREMENT PRIMARY KEY,
        fecha DATETIME DEFAULT CURRENT_TIMESTAMP,
        archivo_url VARCHAR(255),
        transcripcion TEXT
    )
`;

db.query(createTableQuery, (err) => {
    if (err) console.error("Error creando tabla grabaciones:", err);
    else console.log("Tabla 'grabaciones' verificada/creada");
});

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

// --- ENDPOINT PARA GUARDAR GRABACIÓN (Desde Python) ---
app.post('/api/recordings', (req, res) => {
    const { archivo_url, transcripcion } = req.body;
    const query = 'INSERT INTO grabaciones (archivo_url, transcripcion) VALUES (?, ?)';
    db.query(query, [archivo_url, transcripcion], (err, result) => {
        if (err) {
            console.error(err);
            res.status(500).send('Error guardando grabación');
        } else {
            res.json({ status: 'ok', id: result.insertId });
        }
    });
});

// --- ENDPOINT PARA LISTAR GRABACIONES (Para el Dashboard) ---
app.get('/api/recordings', (req, res) => {
    const query = 'SELECT * FROM grabaciones ORDER BY id DESC LIMIT 50';
    db.query(query, (err, results) => {
        if (err) {
            console.error(err);
            res.status(500).send('Error obteniendo grabaciones');
        } else {
            res.json(results);
        }
    });
});

// --- ENDPOINT PARA OBTENER ESTADO DEL LED (DASHBOARD) ---
app.get('/api/led', (req, res) => {
    res.json({ led: ledState });
});

// ============================================
// ENDPOINTS PARA SISTEMA DE EXAMEN
// ============================================

const PYTHON_SERVER = process.env.PYTHON_SERVER_URL || 'http://localhost:5001';

// --- TABLA PARA ALERTAS DE EXAMEN ---
const createExamAlertsTable = `
    CREATE TABLE IF NOT EXISTS exam_alerts (
        id INT AUTO_INCREMENT PRIMARY KEY,
        timestamp DATETIME DEFAULT CURRENT_TIMESTAMP,
        incident_number INT,
        image_url TEXT,
        detections JSON,
        severity VARCHAR(20),
        reviewed BOOLEAN DEFAULT FALSE
    )
`;

db.query(createExamAlertsTable, (err) => {
    if (err) console.error("Error creando tabla exam_alerts:", err);
    else console.log("Tabla 'exam_alerts' verificada/creada");
});

// --- ENDPOINT PARA RECIBIR ALERTAS DE EXAMEN (Desde Python) ---
app.post('/api/exam-alerts', (req, res) => {
    const { timestamp, incident_number, image_url, detections, severity } = req.body;

    const query = `
        INSERT INTO exam_alerts (timestamp, incident_number, image_url, detections, severity) 
        VALUES (?, ?, ?, ?, ?)
    `;

    db.query(query, [
        timestamp,
        incident_number,
        image_url,
        JSON.stringify(detections),
        severity
    ], (err, result) => {
        if (err) {
            console.error('Error guardando alerta de examen:', err);
            res.status(500).send('Error guardando alerta');
        } else {
            console.log(`✓ Alerta de examen guardada: Incidente #${incident_number}`);
            res.json({ status: 'ok', id: result.insertId });
        }
    });
});

// --- ENDPOINT PARA OBTENER ALERTAS DE EXAMEN (Dashboard) ---
app.get('/api/exam-alerts', (req, res) => {
    const limit = req.query.limit || 50;
    const query = 'SELECT * FROM exam_alerts ORDER BY id DESC LIMIT ?';

    db.query(query, [parseInt(limit)], (err, results) => {
        if (err) {
            console.error(err);
            res.status(500).send('Error obteniendo alertas');
        } else {
            // Parsear JSON de detections
            const parsed = results.map(alert => ({
                ...alert,
                detections: JSON.parse(alert.detections)
            }));
            res.json(parsed);
        }
    });
});

// --- ENDPOINT PARA MARCAR ALERTA COMO REVISADA ---
app.patch('/api/exam-alerts/:id/review', (req, res) => {
    const { id } = req.params;
    const query = 'UPDATE exam_alerts SET reviewed = TRUE WHERE id = ?';

    db.query(query, [id], (err) => {
        if (err) {
            console.error(err);
            res.status(500).send('Error actualizando alerta');
        } else {
            res.json({ status: 'ok' });
        }
    });
});

// --- PROXY ENDPOINTS PARA CONTROLAR SERVIDOR PYTHON ---

// Iniciar examen
app.post('/api/exam/start', async (req, res) => {
    try {
        const response = await axios.post(`${PYTHON_SERVER}/api/exam/start`);
        res.json(response.data);
    } catch (error) {
        console.error('Error iniciando examen:', error.message);
        res.status(500).json({ error: 'Failed to start exam', details: error.message });
    }
});

// Detener examen
app.post('/api/exam/stop', async (req, res) => {
    try {
        const response = await axios.post(`${PYTHON_SERVER}/api/exam/stop`);
        res.json(response.data);
    } catch (error) {
        console.error('Error deteniendo examen:', error.message);
        res.status(500).json({ error: 'Failed to stop exam', details: error.message });
    }
});

// Obtener estado del examen
app.get('/api/exam/status', async (req, res) => {
    try {
        const response = await axios.get(`${PYTHON_SERVER}/api/exam/status`);
        res.json(response.data);
    } catch (error) {
        console.error('Error obteniendo estado:', error.message);
        res.status(500).json({ error: 'Failed to get exam status', details: error.message });
    }
});

// Obtener snapshot de la cámara
app.get('/api/exam/snapshot', async (req, res) => {
    try {
        const response = await axios.get(`${PYTHON_SERVER}/api/exam/snapshot`);
        res.json(response.data);
    } catch (error) {
        console.error('Error obteniendo snapshot:', error.message);
        res.status(500).json({ error: 'Failed to get snapshot', details: error.message });
    }
});

const PORT = process.env.PORT || 3000;
app.listen(PORT, '0.0.0.0', () => {
    console.log(`Servidor corriendo en puerto ${PORT}`);
});
