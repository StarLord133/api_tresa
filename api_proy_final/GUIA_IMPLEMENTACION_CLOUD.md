# 🚀 Guía de Implementación Cloud - Sistema de Examen

## 📋 Arquitectura Final

```
┌─────────────────────────────────────────────────────────────┐
│                    DASHBOARD REACT                          │
│              (Vercel/Netlify - Frontend)                    │
│                                                             │
│  ┌──────────────────────────────────────────────────┐     │
│  │  ExamMonitor Component                           │     │
│  │  - Botón Iniciar/Finalizar Examen                │     │
│  │  - Vista de alertas en tiempo real               │     │
│  │  - Snapshots de cámara                           │     │
│  └──────────────────────────────────────────────────┘     │
└──────────────────────┬──────────────────────────────────────┘
                       │ HTTP/WebSocket
                       ↓
┌─────────────────────────────────────────────────────────────┐
│              NODE.JS BACKEND (Render)                       │
│                                                             │
│  Endpoints:                                                 │
│  - POST /api/exam/start  → Proxy a Python                  │
│  - POST /api/exam/stop   → Proxy a Python                  │
│  - GET  /api/exam/status → Proxy a Python                  │
│  - POST /api/exam-alerts → Recibe alertas de Python        │
│  - GET  /api/exam-alerts → Lista alertas para dashboard    │
│                                                             │
│  Base de Datos MySQL:                                       │
│  - Tabla: exam_alerts (id, timestamp, image_url, etc.)     │
└──────────────────────┬──────────────────────────────────────┘
                       │ HTTP
                       ↓
┌─────────────────────────────────────────────────────────────┐
│         PYTHON SERVER - YOLO (Render)                       │
│                                                             │
│  server_exam_detection.py                                   │
│  - Carga YOLOv8                                             │
│  - Loop de monitoreo en thread                              │
│  - Detecta objetos prohibidos                               │
│  - Sube imágenes a Google Cloud Storage                    │
│  - Notifica a Node.js cuando detecta algo                   │
│                                                             │
│  Endpoints:                                                 │
│  - POST /api/exam/start  → Inicia monitoreo                 │
│  - POST /api/exam/stop   → Detiene monitoreo                │
│  - GET  /api/exam/status → Estado actual                    │
│  - GET  /api/exam/snapshot → Captura instantánea            │
└──────────────────────┬──────────────────────────────────────┘
                       │ HTTP GET /capture
                       ↓
┌─────────────────────────────────────────────────────────────┐
│              ESP32-CAM (WiFi Local)                         │
│                                                             │
│  - Conectada a WiFi local                                   │
│  - Endpoint /capture → Devuelve JPEG                        │
│  - Endpoint /led → Control de LED                           │
│  - Resolución: 160x120 (ajustable)                          │
└─────────────────────────────────────────────────────────────┘
                       ↓
┌─────────────────────────────────────────────────────────────┐
│         GOOGLE CLOUD STORAGE                                │
│                                                             │
│  Bucket: exam-monitoring-tresa                              │
│  - exam_incidents/incident_1_20231202_221959.jpg            │
│  - exam_incidents/incident_2_20231202_222557.jpg            │
│  - URLs públicas para el dashboard                          │
└─────────────────────────────────────────────────────────────┘
```

---

## 🔧 Paso 1: Configurar Google Cloud Storage

### 1.1 Crear Bucket
```bash
# En Google Cloud Console
1. Ve a Cloud Storage → Buckets
2. Click "Create Bucket"
3. Nombre: exam-monitoring-tresa
4. Location: us-central1 (o tu región)
5. Storage class: Standard
6. Access control: Fine-grained
7. Create
```

### 1.2 Crear Service Account
```bash
1. IAM & Admin → Service Accounts
2. Create Service Account
3. Nombre: exam-monitoring-service
4. Role: Storage Object Admin
5. Create Key → JSON
6. Descarga el archivo JSON
```

### 1.3 Hacer Bucket Público (para las imágenes)
```bash
# En Cloud Shell o local con gcloud CLI
gsutil iam ch allUsers:objectViewer gs://exam-monitoring-tresa
```

---

## 🐍 Paso 2: Desplegar Servidor Python en Render

### 2.1 Crear requirements.txt
```txt
flask
flask-cors
opencv-python-headless
ultralytics
numpy
requests
google-cloud-storage
gunicorn
```

### 2.2 Crear Procfile (para Render)
```
web: gunicorn server_exam_detection:app --timeout 120
```

### 2.3 Configurar en Render.com
```
1. New → Web Service
2. Conectar tu repositorio GitHub
3. Build Command: pip install -r requirements.txt
4. Start Command: gunicorn server_exam_detection:app --timeout 120
5. Instance Type: Standard (mínimo)

Environment Variables:
- ESP32_IP: 192.168.0.139 (o tu IP)
- CONFIDENCE: 0.45
- NODE_BACKEND_URL: https://api-tresa.onrender.com
- GCS_BUCKET: exam-monitoring-tresa
- GOOGLE_CLOUD_CREDENTIALS: <pegar contenido del JSON>
```

### 2.4 Nota Importante sobre ESP32-CAM
⚠️ **PROBLEMA**: Render no puede acceder a tu ESP32-CAM local (192.168.0.139)

**SOLUCIONES**:

#### Opción A: Usar Ngrok (Recomendado para pruebas)
```bash
# En tu computadora local
ngrok http 80 --host-header=192.168.0.139

# Esto te da una URL pública:
# https://abc123.ngrok.io

# Usar esa URL en ESP32_IP:
ESP32_IP=https://abc123.ngrok.io
```

#### Opción B: Servidor Python Local + Ngrok
```bash
# Ejecutar Python localmente
python server_exam_detection.py

# En otra terminal
ngrok http 5001

# Usar la URL de ngrok en PYTHON_SERVER_URL del Node.js
```

#### Opción C: ESP32 con IP Pública (Avanzado)
- Configurar port forwarding en tu router
- Asignar IP estática a ESP32-CAM
- Usar DynDNS para dominio

---

## 🟢 Paso 3: Actualizar Node.js Backend

### 3.1 Instalar Dependencia
```bash
npm install axios
```

### 3.2 Agregar Endpoints
Copia el contenido de `server_exam_endpoints.js` al final de `server.js` (antes de `app.listen`)

### 3.3 Variables de Entorno en Render
```
PYTHON_SERVER_URL=https://tu-python-server.onrender.com
```

### 3.4 Re-desplegar
```bash
git add .
git commit -m "Add exam monitoring endpoints"
git push origin main
```

---

## ⚛️ Paso 4: Integrar Dashboard React

### 4.1 Agregar Componente al Router
```tsx
// src/App.tsx
import { ExamMonitor } from './components/exam-monitor';

// Agregar ruta:
<Route path="/exam-monitor" element={<ExamMonitor />} />
```

### 4.2 Agregar al Sidebar
```tsx
// src/components/nav-secondary.tsx o donde tengas el menú
{
  title: "Modo Examen",
  url: "/exam-monitor",
  icon: Camera,
}
```

### 4.3 Variables de Entorno
```env
# .env
VITE_API_URL=https://api-tresa.onrender.com
```

### 4.4 Desplegar
```bash
npm run build
# Subir a Vercel/Netlify
```

---

## 🎯 Paso 5: Flujo de Uso

### Desde el Dashboard:

1. **Usuario abre Dashboard** → `/exam-monitor`

2. **Click "Iniciar Examen"**:
   ```
   Dashboard → POST /api/exam/start
   Node.js → POST https://python-server/api/exam/start
   Python → Inicia thread de monitoreo
   Python → Cada 0.5s captura de ESP32-CAM
   Python → YOLO detecta objetos
   ```

3. **Si detecta celular/libro**:
   ```
   Python → Activa LED ESP32
   Python → Sube imagen a GCS
   Python → POST /api/exam-alerts a Node.js
   Node.js → Guarda en MySQL
   Dashboard → Polling cada 5s muestra alerta
   ```

4. **Click "Finalizar Examen"**:
   ```
   Dashboard → POST /api/exam/stop
   Node.js → POST https://python-server/api/exam/stop
   Python → Detiene thread
   Python → Retorna estadísticas
   Dashboard → Muestra resumen
   ```

---

## 📊 Paso 6: Monitoreo y Logs

### Ver Logs de Python (Render)
```
1. Dashboard de Render
2. Tu servicio Python
3. Tab "Logs"
4. Verás:
   - 📦 Cargando YOLO...
   - ✓ YOLO cargado
   - 🎬 Iniciando monitoreo...
   - 🚨 INCIDENTE #1
```

### Ver Logs de Node.js
```
Similar en Render:
- ✓ Alerta de examen guardada: Incidente #1
```

### Ver Imágenes en GCS
```
1. Google Cloud Console
2. Cloud Storage → exam-monitoring-tresa
3. Carpeta: exam_incidents/
4. Verás las imágenes con detecciones
```

---

## 🔋 Optimización de Energía

### ESP32-CAM en Modo Ahorro:

```cpp
// Agregar a ESP32CAM_ModoExamen.ino

void loop() {
  // Verificar si hay solicitud de captura
  // Si no hay examen activo, deep sleep
  
  if (!examActive) {
    esp_sleep_enable_timer_wakeup(60 * 1000000); // 60 segundos
    esp_deep_sleep_start();
  }
}
```

### Python Server:
- Solo consume recursos cuando `exam_state["active"] == True`
- Thread se detiene automáticamente al finalizar examen
- Render puede escalar a 0 si no hay tráfico (plan gratuito)

---

## 💰 Costos Estimados

### Render (Python + Node.js):
- **Free Tier**: $0/mes (con limitaciones)
- **Starter**: $7/mes por servicio = $14/mes total
- **Nota**: Free tier duerme después de 15 min inactividad

### Google Cloud Storage:
- **Almacenamiento**: $0.02/GB/mes
- **Operaciones**: $0.004 por 10,000 operaciones
- **Estimado**: ~$1/mes para 100 imágenes/mes

### Total: ~$15/mes (o $0 con free tiers)

---

## 🧪 Pruebas Locales

### 1. Probar Python Server Localmente:
```bash
cd api_proy_final
python server_exam_detection.py

# En otra terminal:
curl http://localhost:5001/health
curl -X POST http://localhost:5001/api/exam/start
```

### 2. Probar Node.js Localmente:
```bash
# Actualizar .env
PYTHON_SERVER_URL=http://localhost:5001

npm start

# Probar:
curl -X POST http://localhost:3000/api/exam/start
```

### 3. Probar Dashboard Localmente:
```bash
cd dashboard_tessa
npm run dev

# Abrir http://localhost:5173/exam-monitor
```

---

## 🐛 Troubleshooting

### Error: "ESP32-CAM not reachable"
- ✅ Verifica que ESP32 esté encendida
- ✅ Usa ngrok para exponer ESP32 públicamente
- ✅ O ejecuta Python server localmente

### Error: "Failed to upload to GCS"
- ✅ Verifica credenciales JSON
- ✅ Verifica permisos del service account
- ✅ Verifica nombre del bucket

### Error: "YOLO model not found"
- ✅ Render descarga automáticamente yolov8n.pt
- ✅ Puede tardar ~30 segundos en primera ejecución
- ✅ Verifica logs de Render

### Alertas no aparecen en Dashboard:
- ✅ Verifica que NODE_BACKEND_URL sea correcto
- ✅ Verifica tabla exam_alerts en MySQL
- ✅ Verifica CORS en Node.js

---

## 🚀 Próximos Pasos

1. ✅ Desplegar Python server en Render
2. ✅ Configurar Google Cloud Storage
3. ✅ Actualizar Node.js con nuevos endpoints
4. ✅ Agregar componente ExamMonitor al dashboard
5. ✅ Configurar ngrok para ESP32-CAM
6. ✅ Probar flujo completo
7. ✅ Ajustar resolución de ESP32-CAM
8. ✅ Configurar notificaciones (opcional)

---

## 📞 Comandos Útiles

```bash
# Ver logs de Render
render logs --service=python-exam-server

# Reiniciar servicio
render restart --service=python-exam-server

# Ver imágenes en GCS
gsutil ls gs://exam-monitoring-tresa/exam_incidents/

# Descargar imagen
gsutil cp gs://exam-monitoring-tresa/exam_incidents/incident_1.jpg .
```

---

**¡Sistema listo para producción en la nube! 🎉**
