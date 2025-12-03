// ============================================
// ENDPOINTS PARA SISTEMA DE EXAMEN
// Agregar al final de server.js (antes del app.listen)
// ============================================

// Instalar axios si no lo tienes: npm install axios
const axios = require('axios');
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
