# ✅ Checklist de Implementación - Sistema de Examen

## 📦 Google Cloud Storage - COMPLETADO ✅

- [x] Bucket creado: `exam-monitoring-tresa`
- [x] Bucket configurado como público
- [x] Service Account creado
- [x] Credenciales JSON descargadas y guardadas

---

## 🐍 Servidor Python - PENDIENTE

### Archivos Necesarios:
- [x] `server_exam_detection.py` - Servidor principal
- [x] `requirements_exam.txt` - Dependencias
- [x] `.env.exam.example` - Variables de entorno de ejemplo
- [x] `credenciales.txt` - Credenciales de GCS (guardadas)

### Pasos para Desplegar en Render:

1. [ ] Subir archivos a GitHub
   ```bash
   git add server_exam_detection.py requirements_exam.txt
   git commit -m "Add exam detection server"
   git push
   ```

2. [ ] Crear nuevo servicio en Render.com
   - Ir a: https://render.com
   - New → Web Service
   - Conectar repositorio GitHub

3. [ ] Configurar Build Settings:
   - **Build Command**: `pip install -r requirements_exam.txt`
   - **Start Command**: `gunicorn server_exam_detection:app --timeout 120`
   - **Instance Type**: Starter ($7/mes) o Free

4. [ ] Configurar Environment Variables:
   ```
   ESP32_IP = (URL de ngrok o IP pública)
   CONFIDENCE = 0.45
   NODE_BACKEND_URL = https://api-tresa.onrender.com
   GCS_BUCKET = exam-monitoring-tresa
   GOOGLE_CLOUD_CREDENTIALS = (pegar JSON completo de credenciales.txt)
   PORT = 5001
   ```

5. [ ] Deploy y esperar ~5 minutos

6. [ ] Verificar logs en Render:
   - Buscar: "📦 Cargando YOLO..."
   - Buscar: "✓ YOLO cargado"
   - Buscar: "✓ Google Cloud Storage conectado"

7. [ ] Probar endpoint de health:
   ```
   https://tu-servicio.onrender.com/health
   ```

---

## 🟢 Node.js Backend - PENDIENTE

### Pasos:

1. [ ] Instalar axios:
   ```bash
   npm install axios
   ```

2. [ ] Agregar código de `server_exam_endpoints.js` a `server.js`
   - Copiar todo el contenido
   - Pegar ANTES de `app.listen()`

3. [ ] Agregar variable de entorno en Render:
   ```
   PYTHON_SERVER_URL = https://tu-python-server.onrender.com
   ```

4. [ ] Commit y push:
   ```bash
   git add server.js package.json package-lock.json
   git commit -m "Add exam monitoring endpoints"
   git push
   ```

5. [ ] Render auto-desplegará

6. [ ] Verificar tabla en MySQL:
   - Conectar a base de datos
   - Verificar que existe tabla `exam_alerts`

---

## ⚛️ Dashboard React - COMPLETADO ✅

- [x] Componente `exam-monitor.tsx` creado
- [x] Componente `alert.tsx` creado
- [x] Página `exam-monitor/page.tsx` creada
- [x] Ruta agregada en `App.tsx`
- [x] Enlace agregado en sidebar

### Pendiente:

1. [ ] Verificar que `VITE_API_URL` esté configurado:
   ```env
   VITE_API_URL=https://api-tresa.onrender.com
   ```

2. [ ] Build y deploy:
   ```bash
   npm run build
   # Subir a Vercel/Netlify
   ```

---

## 🔌 ESP32-CAM - PENDIENTE

### Opción A: Ngrok (Temporal)

1. [ ] En tu PC, ejecutar:
   ```bash
   ngrok http 80 --host-header=192.168.0.139
   ```

2. [ ] Copiar URL pública (ej: `https://abc123.ngrok.io`)

3. [ ] Usar esa URL en `ESP32_IP` del servidor Python

### Opción B: Python Local (Desarrollo)

1. [ ] Ejecutar servidor Python localmente:
   ```bash
   python server_exam_detection.py
   ```

2. [ ] Exponer con ngrok:
   ```bash
   ngrok http 5001
   ```

3. [ ] Usar URL de ngrok en `PYTHON_SERVER_URL` de Node.js

---

## 🧪 Pruebas

### 1. Probar Python Server (Local):
```bash
# Health check
curl http://localhost:5001/health

# Iniciar examen
curl -X POST http://localhost:5001/api/exam/start

# Ver estado
curl http://localhost:5001/api/exam/status

# Detener examen
curl -X POST http://localhost:5001/api/exam/stop
```

### 2. Probar Node.js Backend:
```bash
# Iniciar examen (proxy)
curl -X POST https://api-tresa.onrender.com/api/exam/start

# Ver alertas
curl https://api-tresa.onrender.com/api/exam-alerts
```

### 3. Probar Dashboard:
1. Abrir: `http://localhost:5173/exam-monitor`
2. Click "Iniciar Examen"
3. Acercar celular a cámara
4. Verificar que aparece alerta

---

## 📊 Resumen de URLs

```
Dashboard:        https://tu-dashboard.vercel.app/exam-monitor
Node.js Backend:  https://api-tresa.onrender.com
Python Server:    https://tu-python.onrender.com
ESP32-CAM:        http://192.168.0.139 (local)
                  https://abc123.ngrok.io (público temporal)
GCS Bucket:       gs://exam-monitoring-tresa
```

---

## 🎯 Próximos Pasos Inmediatos

1. **Subir código a GitHub**
2. **Desplegar Python en Render**
3. **Actualizar Node.js con endpoints**
4. **Configurar ngrok para ESP32-CAM**
5. **Probar flujo completo**

---

## ❓ Troubleshooting

### Python no carga YOLO:
- Verificar logs de Render
- Puede tardar ~30 segundos en primera carga
- Verificar que instance type tenga suficiente RAM

### ESP32-CAM no conecta:
- Verificar que ngrok esté corriendo
- Verificar URL en ESP32_IP
- Probar manualmente: `curl https://ngrok-url/capture`

### Alertas no llegan al dashboard:
- Verificar tabla `exam_alerts` en MySQL
- Verificar logs de Node.js
- Verificar CORS en Node.js

---

**Estado Actual: 40% Completado** 🚀
