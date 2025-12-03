#!/usr/bin/env python3
"""
Servidor de Detección de Examen - API REST
Para desplegar en Render.com
"""

from flask import Flask, request, jsonify
from flask_cors import CORS
import cv2
import numpy as np
from ultralytics import YOLO
from datetime import datetime
import os
import time
import requests
import threading
from google.cloud import storage
import tempfile
import base64

app = Flask(__name__)
CORS(app)

# CONFIGURACIÓN
ESP32_IP = os.getenv("ESP32_IP", "192.168.0.139")

# Determinar URL base correcta
if ESP32_IP.startswith("http://") or ESP32_IP.startswith("https://"):
    BASE_ESP32_URL = ESP32_IP
else:
    BASE_ESP32_URL = f"http://{ESP32_IP}"

CAPTURE_URL = f"{BASE_ESP32_URL}/capture"
CONFIDENCE = float(os.getenv("CONFIDENCE", "0.45"))
NODE_BACKEND = os.getenv("NODE_BACKEND_URL", "https://api-tresa.onrender.com")

# Google Cloud Storage
GCS_BUCKET = os.getenv("GCS_BUCKET", "exam-monitoring-tresa")

PROHIBITED = {
    67: "Celular/Teléfono",
    73: "Libro",
}

# Estado global
exam_state = {
    "active": False,
    "start_time": None,
    "incident_count": 0,
    "monitoring_thread": None,
    "stop_monitoring": False
}

# Cargar YOLO al iniciar
print("📦 Cargando YOLO...")
model = YOLO('yolov8n.pt')
print("✓ YOLO cargado")

# Configurar Google Cloud Storage
storage_client = None
bucket = None

try:
    if "GOOGLE_CLOUD_CREDENTIALS" in os.environ:
        with tempfile.NamedTemporaryFile(mode='w', delete=False, suffix='.json') as temp:
            temp.write(os.environ["GOOGLE_CLOUD_CREDENTIALS"])
            temp_path = temp.name
        os.environ["GOOGLE_APPLICATION_CREDENTIALS"] = temp_path
        storage_client = storage.Client()
        bucket = storage_client.bucket(GCS_BUCKET)
        print(f"✓ Google Cloud Storage conectado: {GCS_BUCKET}")
except Exception as e:
    print(f"⚠️ GCS no disponible: {e}")


def capture_frame():
    """Capturar frame desde ESP32-CAM"""
    try:
        r = requests.get(CAPTURE_URL, timeout=30)
        if r.status_code != 200:
            return None
        
        img_array = np.frombuffer(r.content, dtype=np.uint8)
        frame = cv2.imdecode(img_array, cv2.IMREAD_COLOR)
        return frame
    except Exception as e:
        print(f"Error capturando frame: {e}")
        return None


def trigger_led(state):
    """Controlar LED de ESP32-CAM"""
    try:
        requests.get(f"{BASE_ESP32_URL}/led?state={state}", timeout=2)
    except:
        pass


def upload_to_gcs(image_data, filename):
    """Subir imagen a Google Cloud Storage"""
    if not bucket:
        return None
    
    try:
        blob = bucket.blob(f"exam_incidents/{filename}")
        blob.upload_from_string(image_data, content_type='image/jpeg')
        blob.make_public()
        return blob.public_url
    except Exception as e:
        print(f"Error subiendo a GCS: {e}")
        return None


def notify_backend(incident_data):
    """Notificar al backend Node.js"""
    try:
        response = requests.post(
            f"{NODE_BACKEND}/api/exam-alerts",
            json=incident_data,
            timeout=5
        )
        print(f"✓ Alerta enviada al backend: {response.status_code}")
    except Exception as e:
        print(f"Error notificando backend: {e}")


def monitoring_loop():
    """Loop de monitoreo continuo"""
    print("🎬 Iniciando monitoreo...")
    
    while not exam_state["stop_monitoring"]:
        if not exam_state["active"]:
            time.sleep(1)
            continue
        
        # Capturar frame
        frame = capture_frame()
        if frame is None:
            time.sleep(0.5)
            continue
        
        # Detectar con YOLO
        results = model(frame, verbose=False)
        
        detections = []
        prohibited_items = []
        
        for result in results:
            for box in result.boxes:
                conf = float(box.conf[0])
                if conf < CONFIDENCE:
                    continue
                
                class_id = int(box.cls[0])
                class_name = model.names[class_id]
                
                detection = {
                    'class_id': class_id,
                    'name': class_name,
                    'confidence': conf
                }
                
                detections.append(detection)
                
                if class_id in PROHIBITED:
                    prohibited_items.append(detection)
        
        # Si hay objetos prohibidos, generar alerta
        if prohibited_items and exam_state["active"]:
            handle_incident(frame, prohibited_items)
        
        # Pequeña pausa para no saturar
        time.sleep(0.5)
    
    print("🛑 Monitoreo detenido")


def handle_incident(frame, items):
    """Manejar incidente detectado"""
    exam_state["incident_count"] += 1
    ts = datetime.now()
    
    print(f"\n🚨 INCIDENTE #{exam_state['incident_count']}")
    
    # Activar LED
    trigger_led(1)
    time.sleep(2)
    trigger_led(0)
    
    # Guardar imagen
    filename = f"incident_{exam_state['incident_count']}_{ts.strftime('%Y%m%d_%H%M%S')}.jpg"
    
    # Codificar imagen
    _, img_encoded = cv2.imencode('.jpg', frame)
    img_bytes = img_encoded.tobytes()
    
    # Subir a GCS
    image_url = upload_to_gcs(img_bytes, filename)
    
    # Si no hay GCS, usar base64
    if not image_url:
        image_base64 = base64.b64encode(img_bytes).decode('utf-8')
        image_url = f"data:image/jpeg;base64,{image_base64}"
    
    # Preparar datos del incidente
    incident_data = {
        'timestamp': ts.isoformat(),
        'incident_number': exam_state['incident_count'],
        'image_url': image_url,
        'detections': [
            {
                'object': PROHIBITED.get(item['class_id'], item['name']),
                'confidence': item['confidence']
            }
            for item in items
        ],
        'severity': 'high' if len(items) > 1 else 'medium'
    }
    
    # Notificar al backend
    notify_backend(incident_data)
    
    print(f"✓ Incidente procesado: {filename}")


# ==================== ENDPOINTS API ====================

@app.route('/health', methods=['GET'])
def health():
    """Health check"""
    return jsonify({
        "status": "ok",
        "yolo_loaded": model is not None,
        "gcs_available": bucket is not None,
        "exam_active": exam_state["active"]
    })


@app.route('/api/exam/start', methods=['POST'])
def start_exam():
    """Iniciar modo examen"""
    if exam_state["active"]:
        return jsonify({"error": "Exam already active"}), 400
    
    # Verificar conexión con ESP32
    try:
        r = requests.get(f"{BASE_ESP32_URL}/", timeout=30)
        if r.status_code != 200:
            return jsonify({"error": "ESP32-CAM not reachable"}), 503
    except:
        return jsonify({"error": "ESP32-CAM not reachable"}), 503
    
    # Iniciar examen
    exam_state["active"] = True
    exam_state["start_time"] = datetime.now()
    exam_state["incident_count"] = 0
    exam_state["stop_monitoring"] = False
    
    # Iniciar thread de monitoreo
    exam_state["monitoring_thread"] = threading.Thread(target=monitoring_loop, daemon=True)
    exam_state["monitoring_thread"].start()
    
    print(f"\n🟢 EXAMEN INICIADO: {exam_state['start_time']}")
    
    return jsonify({
        "status": "started",
        "start_time": exam_state["start_time"].isoformat(),
        "esp32_ip": ESP32_IP
    })


@app.route('/api/exam/stop', methods=['POST'])
def stop_exam():
    """Detener modo examen"""
    if not exam_state["active"]:
        return jsonify({"error": "No active exam"}), 400
    
    # Detener monitoreo
    exam_state["stop_monitoring"] = True
    exam_state["active"] = False
    
    end_time = datetime.now()
    duration = end_time - exam_state["start_time"]
    
    print(f"\n🔴 EXAMEN FINALIZADO")
    print(f"Duración: {duration}")
    print(f"Incidentes: {exam_state['incident_count']}")
    
    result = {
        "status": "stopped",
        "start_time": exam_state["start_time"].isoformat(),
        "end_time": end_time.isoformat(),
        "duration_seconds": duration.total_seconds(),
        "incident_count": exam_state["incident_count"]
    }
    
    return jsonify(result)


@app.route('/api/exam/status', methods=['GET'])
def exam_status():
    """Obtener estado actual del examen"""
    if not exam_state["active"]:
        return jsonify({
            "active": False,
            "incident_count": 0
        })
    
    elapsed = datetime.now() - exam_state["start_time"]
    
    return jsonify({
        "active": True,
        "start_time": exam_state["start_time"].isoformat(),
        "elapsed_seconds": elapsed.total_seconds(),
        "incident_count": exam_state["incident_count"]
    })


@app.route('/api/exam/snapshot', methods=['GET'])
def get_snapshot():
    """Obtener snapshot actual de la cámara"""
    frame = capture_frame()
    if frame is None:
        return jsonify({"error": "Failed to capture frame"}), 500
    
    _, img_encoded = cv2.imencode('.jpg', frame)
    img_base64 = base64.b64encode(img_encoded.tobytes()).decode('utf-8')
    
    return jsonify({
        "image": f"data:image/jpeg;base64,{img_base64}",
        "timestamp": datetime.now().isoformat()
    })


@app.route('/api/config', methods=['GET'])
def get_config():
    """Obtener configuración actual"""
    return jsonify({
        "esp32_ip": ESP32_IP,
        "confidence_threshold": CONFIDENCE,
        "prohibited_objects": PROHIBITED,
        "gcs_bucket": GCS_BUCKET if bucket else None
    })


@app.route('/api/config', methods=['POST'])
def update_config():
    """Actualizar configuración"""
    global CONFIDENCE, ESP32_IP
    
    data = request.json
    
    if 'confidence' in data:
        CONFIDENCE = float(data['confidence'])
    
    if 'esp32_ip' in data:
        ESP32_IP = data['esp32_ip']
    
    return jsonify({
        "status": "updated",
        "confidence": CONFIDENCE,
        "esp32_ip": ESP32_IP
    })


if __name__ == '__main__':
    port = int(os.getenv('PORT', 5001))
    print(f"\n{'='*60}")
    print(f"  🎓 Servidor de Detección de Examen")
    print(f"  Puerto: {port}")
    print(f"  ESP32-CAM: {ESP32_IP}")
    print(f"  GCS: {'✓' if bucket else '✗'}")
    print(f"{'='*60}\n")
    
    app.run(host='0.0.0.0', port=port, debug=False)
