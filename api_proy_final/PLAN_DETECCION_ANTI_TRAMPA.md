# 🎓 Sistema de Detección Anti-Trampa para Exámenes
# Implementación con ESP32-CAM + Python + YOLO

## 📋 Arquitectura Propuesta

```
ESP32-CAM (Captura) 
    ↓ HTTP POST cada 5 segundos
Python Server (Detección YOLO)
    ↓ Guarda imagen + metadata
Node.js Backend (Almacenamiento)
    ↓ WebSocket
Dashboard React (Alertas en tiempo real)
```

---

## 🔧 Componentes a Implementar

### 1. ESP32-CAM: Captura Automática
**Archivo:** `ESP32CAM_ModoExamen.ino`

**Funcionalidad:**
- Captura foto cada 5 segundos
- Envía a servidor Python vía HTTP POST
- Incluye timestamp y ID del estudiante

**Código a agregar:**
```cpp
// En loop()
void loop() {
  static unsigned long lastCapture = 0;
  unsigned long now = millis();
  
  if (now - lastCapture > 5000) {  // Cada 5 segundos
    captureAndSend();
    lastCapture = now;
  }
}

void captureAndSend() {
  camera_fb_t *fb = esp_camera_fb_get();
  if (!fb) return;
  
  // Enviar a Python server
  HTTPClient http;
  http.begin("http://TU_SERVIDOR_PYTHON:5001/detect_exam");
  http.addHeader("Content-Type", "image/jpeg");
  
  int httpCode = http.POST(fb->buf, fb->len);
  
  esp_camera_fb_return(fb);
  http.end();
}
```

---

### 2. Python Server: Detección con YOLO
**Archivo:** `server_detection.py`

**Dependencias a agregar en `requirements.txt`:**
```
flask
opencv-python
ultralytics  # YOLOv8
pillow
numpy
google-cloud-storage  # Para guardar en Cloud Storage
```

**Funcionalidad:**
- Recibe imagen de ESP32-CAM
- Detecta objetos con YOLOv8
- Identifica:
  - 📱 Celulares (class: cell phone)
  - 👥 Personas (class: person) - debe ser solo 1
  - 📖 Libros (class: book)
  - 💻 Laptops (class: laptop)
- Guarda imagen si hay detección sospechosa
- Envía alerta a Node.js backend

**Código base:**
```python
from flask import Flask, request, jsonify
from ultralytics import YOLO
import cv2
import numpy as np
from datetime import datetime
import os
import requests

app = Flask(__name__)

# Cargar modelo YOLO
model = YOLO('yolov8n.pt')  # Modelo nano (rápido)

# Objetos sospechosos
SUSPICIOUS_CLASSES = ['cell phone', 'book', 'laptop']
IMAGES_DIR = "exam_captures"
os.makedirs(IMAGES_DIR, exist_ok=True)

NODE_BACKEND = "https://api-tresa.onrender.com/api/exam-alerts"

@app.route('/detect_exam', methods=['POST'])
def detect_exam():
    try:
        # Recibir imagen
        img_bytes = request.data
        nparr = np.frombuffer(img_bytes, np.uint8)
        img = cv2.imdecode(nparr, cv2.IMREAD_COLOR)
        
        # Detectar objetos
        results = model(img)
        
        # Analizar detecciones
        detections = []
        is_suspicious = False
        person_count = 0
        
        for r in results:
            for box in r.boxes:
                class_name = model.names[int(box.cls)]
                confidence = float(box.conf)
                
                if class_name == 'person':
                    person_count += 1
                
                if class_name in SUSPICIOUS_CLASSES and confidence > 0.5:
                    is_suspicious = True
                    detections.append({
                        'object': class_name,
                        'confidence': confidence
                    })
        
        # Verificar número de personas
        if person_count != 1:
            is_suspicious = True
            detections.append({
                'object': 'multiple_persons' if person_count > 1 else 'no_person',
                'confidence': 1.0
            })
        
        # Guardar imagen si es sospechosa
        if is_suspicious:
            timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
            filename = f"suspicious_{timestamp}.jpg"
            filepath = os.path.join(IMAGES_DIR, filename)
            
            # Dibujar detecciones en la imagen
            annotated = results[0].plot()
            cv2.imwrite(filepath, annotated)
            
            # Enviar alerta al backend
            alert_data = {
                'timestamp': timestamp,
                'image_url': f'/exam_images/{filename}',
                'detections': detections,
                'severity': 'high' if len(detections) > 1 else 'medium'
            }
            
            try:
                requests.post(NODE_BACKEND, json=alert_data)
            except Exception as e:
                print(f"Error enviando alerta: {e}")
            
            return jsonify({
                'status': 'suspicious',
                'detections': detections,
                'image_saved': filename
            }), 200
        
        return jsonify({
            'status': 'ok',
            'person_count': person_count
        }), 200
        
    except Exception as e:
        return jsonify({'error': str(e)}), 500

@app.route('/exam_images/<filename>')
def serve_image(filename):
    return send_from_directory(IMAGES_DIR, filename)

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5001)
```

---

### 3. Node.js Backend: Endpoint de Alertas
**Archivo:** `server.js` (agregar endpoint)

```javascript
// Endpoint para recibir alertas de examen
app.post('/api/exam-alerts', async (req, res) => {
  try {
    const { timestamp, image_url, detections, severity } = req.body;
    
    // Guardar en base de datos
    const alert = await ExamAlert.create({
      timestamp: new Date(timestamp),
      imageUrl: image_url,
      detections: JSON.stringify(detections),
      severity: severity,
      reviewed: false
    });
    
    // Emitir evento WebSocket para alerta en tiempo real
    io.emit('exam_alert', {
      id: alert.id,
      timestamp: alert.timestamp,
      detections: detections,
      severity: severity
    });
    
    res.json({ status: 'alert_received' });
  } catch (error) {
    res.status(500).json({ error: error.message });
  }
});
```

---

### 4. Dashboard React: Panel de Alertas
**Componente:** `ExamMonitor.tsx`

```tsx
import { useEffect, useState } from 'react';
import io from 'socket.io-client';

interface ExamAlert {
  id: string;
  timestamp: Date;
  detections: Array<{object: string, confidence: number}>;
  severity: 'high' | 'medium' | 'low';
  imageUrl: string;
}

export function ExamMonitor() {
  const [alerts, setAlerts] = useState<ExamAlert[]>([]);
  
  useEffect(() => {
    const socket = io('https://api-tresa.onrender.com');
    
    socket.on('exam_alert', (alert: ExamAlert) => {
      setAlerts(prev => [alert, ...prev]);
      
      // Notificación del navegador
      if (alert.severity === 'high') {
        new Notification('⚠️ Alerta de Examen', {
          body: `Detección sospechosa: ${alert.detections.map(d => d.object).join(', ')}`,
          icon: '/alert-icon.png'
        });
      }
    });
    
    return () => socket.disconnect();
  }, []);
  
  return (
    <div className="exam-monitor">
      <h2>Monitor de Examen en Tiempo Real</h2>
      
      {alerts.map(alert => (
        <div key={alert.id} className={`alert alert-${alert.severity}`}>
          <img src={alert.imageUrl} alt="Captura" />
          <div>
            <strong>{new Date(alert.timestamp).toLocaleString()}</strong>
            <ul>
              {alert.detections.map((d, i) => (
                <li key={i}>
                  {d.object} ({(d.confidence * 100).toFixed(1)}%)
                </li>
              ))}
            </ul>
          </div>
        </div>
      ))}
    </div>
  );
}
```

---

## 🎯 Objetos que YOLO Puede Detectar

### Clases Útiles para Anti-Trampa:
- ✅ `cell phone` - Celulares
- ✅ `book` - Libros/apuntes
- ✅ `laptop` - Computadoras
- ✅ `person` - Personas (contar cuántas hay)
- ✅ `tv` - Pantallas adicionales
- ✅ `keyboard` - Teclados externos
- ✅ `mouse` - Mouse

### Reglas de Detección:
1. **Debe haber exactamente 1 persona** → Si hay 0 o >1, alerta
2. **No debe haber celulares** → Alerta inmediata
3. **No debe haber libros** (si es examen cerrado)
4. **No debe haber laptops** (si es examen en papel)

---

## 📊 Flujo Completo

```
1. ESP32-CAM captura foto cada 5 seg
2. Envía a Python /detect_exam
3. YOLO analiza la imagen
4. Si detecta algo sospechoso:
   a. Guarda imagen con anotaciones
   b. Envía alerta a Node.js
   c. Node.js emite WebSocket
   d. Dashboard muestra alerta en tiempo real
   e. Notificación al profesor
5. Si todo está OK, descarta la imagen
```

---

## 💾 Almacenamiento

### Opción A: Local (Desarrollo)
```python
IMAGES_DIR = "exam_captures"
```

### Opción B: Google Cloud Storage (Producción)
```python
from google.cloud import storage

def upload_to_gcs(filepath, filename):
    client = storage.Client()
    bucket = client.bucket('exam-monitoring')
    blob = bucket.blob(f'suspicious/{filename}')
    blob.upload_from_filename(filepath)
    return blob.public_url
```

---

## 🚀 Pasos de Implementación

### Fase 1: Setup Básico
1. ✅ Instalar dependencias YOLO
2. ✅ Descargar modelo YOLOv8
3. ✅ Crear endpoint `/detect_exam`
4. ✅ Probar con imagen de prueba

### Fase 2: Integración ESP32
1. ✅ Modificar ESP32 para captura periódica
2. ✅ Implementar POST a servidor Python
3. ✅ Probar flujo completo

### Fase 3: Backend y Frontend
1. ✅ Crear modelo ExamAlert en Node.js
2. ✅ Implementar WebSocket
3. ✅ Crear componente ExamMonitor
4. ✅ Agregar notificaciones

### Fase 4: Optimización
1. ✅ Ajustar intervalos de captura
2. ✅ Optimizar modelo YOLO
3. ✅ Implementar almacenamiento en cloud
4. ✅ Agregar dashboard de estadísticas

---

## 📈 Mejoras Futuras

1. **Detección de mirada** → OpenCV + Face landmarks
2. **Análisis de audio** → Detectar conversaciones
3. **Detección de movimiento** → Alertar si sale del encuadre
4. **Reconocimiento facial** → Verificar identidad del estudiante
5. **Análisis de comportamiento** → ML para patrones sospechosos

---

## 🔒 Consideraciones de Privacidad

⚠️ **IMPORTANTE:**
- Informar a estudiantes sobre monitoreo
- Cumplir con GDPR/leyes locales
- Guardar imágenes solo el tiempo necesario
- Encriptar almacenamiento
- Permitir revisión de falsas alarmas

---

## 💰 Costos Estimados

### Opción 1: YOLOv8 Local
- **Servidor:** $5-10/mes (VPS básico)
- **Almacenamiento:** $0.02/GB
- **Total:** ~$10/mes

### Opción 2: Google Cloud Vision API
- **API Calls:** $1.50 por 1000 imágenes
- **Storage:** $0.02/GB
- **Total:** Variable según uso

**Recomendación:** Empezar con YOLO local

---

## 🎓 Recursos de Aprendizaje

- [YOLOv8 Docs](https://docs.ultralytics.com/)
- [ESP32-CAM HTTP POST](https://randomnerdtutorials.com/esp32-cam-post-image-photo-server/)
- [Object Detection Tutorial](https://www.youtube.com/watch?v=tFNJGim3FXw)

