# 🎓 Sistema Modo Examen - ESP32-CAM + YOLO

## 📋 Descripción
Sistema de vigilancia anti-trampa para exámenes que detecta objetos prohibidos en tiempo real usando YOLOv8 y ESP32-CAM.

## ✅ Dependencias Instaladas
- ✓ opencv-python
- ✓ ultralytics (YOLOv8)
- ✓ numpy
- ✓ requests

## 🚀 Cómo Usar

### 1. Asegúrate que la ESP32-CAM esté encendida
- Debe estar conectada a WiFi "Roborregos"
- IP configurada: `192.168.0.139`
- Verifica que puedas acceder a: `http://192.168.0.139`

### 2. Ejecuta el script
```bash
python modo_examen_yolo.py
```

### 3. Controles
Una vez que se abra la ventana:
- **S** = Iniciar examen (comienza a detectar y guardar incidentes)
- **E** = Finalizar examen (muestra estadísticas)
- **Q** = Salir del programa

## 🎯 Objetos Detectados

### Prohibidos (generan alerta):
- 📱 **Celulares/Teléfonos** (class_id: 67)
- 📚 **Libros** (class_id: 73)

### Permitidos (solo se muestran):
- 👤 **Personas** (se cuenta cuántas hay)
- Otros objetos detectados por YOLO

## 🔴 Sistema de Alertas

Cuando se detecta un objeto prohibido:
1. ✅ Se muestra alerta visual en pantalla (parpadeo rojo)
2. ✅ Se enciende el LED de la ESP32-CAM por 2 segundos
3. ✅ Se guarda imagen con anotaciones en `exam_logs/images/`
4. ✅ Se imprime alerta en consola con detalles

## 📁 Estructura de Archivos Generados

```
exam_logs/
  └── images/
      ├── incident_1_143052.jpg
      ├── incident_2_143125.jpg
      └── ...
```

## 🎨 Interfaz Visual

La ventana muestra:
- **Panel superior**: Estado del examen, tiempo transcurrido
- **Estadísticas**: Número de incidentes y personas detectadas
- **Detecciones**: Cajas delimitadoras con etiquetas
  - Verde = Objetos permitidos
  - Rojo parpadeante = Objetos prohibidos
- **FPS**: Frames por segundo en esquina inferior

## ⚙️ Configuración

Puedes modificar estas variables en `modo_examen_yolo.py`:

```python
ESP32_IP = "192.168.0.139"  # IP de tu ESP32-CAM
CONFIDENCE = 0.45           # Umbral de confianza (0-1)

PROHIBITED = {
    67: "Celular/Teléfono",
    73: "Libro",
    # Agrega más IDs según necesites
}
```

## 📊 IDs de Clases YOLO Comunes

```
0: person
67: cell phone
73: book
63: laptop
64: mouse
65: remote
66: keyboard
```

Para ver todas las clases: https://github.com/ultralytics/ultralytics/blob/main/ultralytics/cfg/datasets/coco.yaml

## 🔧 Troubleshooting

### "No conecta a ESP32-CAM"
- Verifica que la ESP32 esté encendida
- Confirma la IP con Serial Monitor
- Prueba hacer ping: `ping 192.168.0.139`

### "/capture no funciona"
- Asegúrate que el firmware de ESP32 tenga el endpoint `/capture`
- Prueba manualmente: `http://192.168.0.139/capture`

### "LED no disponible"
- El LED es opcional, el sistema funciona sin él
- Verifica que el endpoint `/led` exista en el firmware

### "YOLO muy lento"
- Reduce la resolución de la ESP32-CAM
- Usa modelo más pequeño: `yolov8n.pt` (nano)
- Aumenta el umbral de confianza

### "Falsos positivos"
- Aumenta `CONFIDENCE` (ej: 0.6 o 0.7)
- Verifica iluminación de la cámara
- Ajusta posición de la cámara

## 📈 Mejoras Futuras

- [ ] Enviar alertas a servidor Node.js
- [ ] Guardar en Google Cloud Storage
- [ ] Detección de múltiples personas
- [ ] Análisis de mirada (face landmarks)
- [ ] Dashboard web en tiempo real
- [ ] Grabación de video completo
- [ ] Reconocimiento facial del estudiante

## 🎓 Uso en Exámenes Reales

### Antes del examen:
1. Posiciona la ESP32-CAM frente al estudiante
2. Verifica buena iluminación
3. Ejecuta el script y prueba detección
4. Presiona **S** para iniciar

### Durante el examen:
- El sistema monitorea automáticamente
- Las alertas se muestran en tiempo real
- Las imágenes se guardan automáticamente

### Después del examen:
- Presiona **E** para finalizar
- Revisa las imágenes en `exam_logs/images/`
- Verifica estadísticas en consola

## ⚠️ Consideraciones Legales

- ✅ Informar al estudiante sobre el monitoreo
- ✅ Obtener consentimiento previo
- ✅ Cumplir con leyes de privacidad locales
- ✅ Borrar imágenes después de revisión
- ✅ Permitir apelación de falsas alarmas

## 📞 Soporte

Si tienes problemas:
1. Verifica que todas las dependencias estén instaladas
2. Confirma que la ESP32-CAM funcione correctamente
3. Revisa los logs en consola para errores específicos
