#!/usr/bin/env python3
"""
Sistema Modo Examen - ESP32-CAM con YOLO
"""

import cv2
import numpy as np
from ultralytics import YOLO
from datetime import datetime
import os
import time
import requests

# CONFIGURACIÓN
ESP32_IP = "192.168.0.139"
CAPTURE_URL = f"http://{ESP32_IP}/capture"
CONFIDENCE = 0.45

PROHIBITED = {
    67: "Celular/Teléfono",
    73: "Libro",
}

SAVE_PATH = "exam_logs/"
IMAGES_PATH = f"{SAVE_PATH}images/"

class ExamMonitor:
    def __init__(self):
        print("=" * 60)
        print("  🎓 SISTEMA MODO EXAMEN")
        print("=" * 60)
        
        os.makedirs(IMAGES_PATH, exist_ok=True)
        
        self.exam_started = False
        self.exam_start_time = None
        self.incident_count = 0
        self.person_count = 0
        self.led_available = False
        
        # YOLO
        print("\n📦 Cargando YOLO...")
        self.model = YOLO('yolov8n.pt')
        print("✓ YOLO cargado")
        
        # Verificar ESP32
        print(f"\n🔍 Verificando ESP32-CAM...")
        try:
            r = requests.get(f"http://{ESP32_IP}/", timeout=5)
            print(f"✓ ESP32-CAM OK (HTTP {r.status_code})")
        except:
            print(f"✗ No conecta")
            exit(1)
        
        # Probar captura
        print(f"\n📸 Probando /capture...")
        frame = self.capture_frame()
        if frame is None:
            print("✗ /capture no funciona")
            exit(1)
        
        print(f"✓ Frame OK ({frame.shape[1]}x{frame.shape[0]})")
        
        # Probar LED
        print("\n🔴 Probando LED...")
        try:
            r = requests.get(f"http://{ESP32_IP}/led?state=0", timeout=2)
            if r.status_code == 200:
                print("✓ LED disponible (GPIO 4)")
                self.led_available = True
            else:
                print("⚠️ LED no responde")
        except:
            print("⚠️ LED no disponible")

        print("\n" + "=" * 60)
        print("  ✅ SISTEMA LISTO")
        print("\n  📋 DETECTA:")
        print("     📱 Teléfonos/Celulares")
        print("     📚 Libros")
        print("\n  🎮 CONTROLES:")
        print("     S = Iniciar examen")
        print("     E = Finalizar examen")
        print("     Q = Salir")
        print("=" * 60 + "\n")
    
    def capture_frame(self):
        """Capturar frame desde /capture"""
        try:
            r = requests.get(CAPTURE_URL, timeout=2)
            if r.status_code != 200:
                return None
            
            img_array = np.frombuffer(r.content, dtype=np.uint8)
            frame = cv2.imdecode(img_array, cv2.IMREAD_COLOR)
            return frame
        except:
            return None
    
    def trigger_led(self, state):
        """Controlar LED"""
        if not self.led_available:
            return
        try:
            requests.get(f"http://{ESP32_IP}/led?state={state}", timeout=1)
            if state == 1:
                print("🔴 LED ENCENDIDO")
            else:
                print("⚪ LED APAGADO")
        except:
            pass
    
    def start_exam(self):
        self.exam_started = True
        self.exam_start_time = datetime.now()
        print("\n" + "🟢" * 30)
        print("  ✅ EXAMEN INICIADO")
        print(f"  📅 {self.exam_start_time.strftime('%H:%M:%S')}")
        print("🟢" * 30 + "\n")
    
    def end_exam(self):
        if not self.exam_started:
            return
        duration = datetime.now() - self.exam_start_time
        print("\n" + "🔴" * 30)
        print("  🏁 EXAMEN FINALIZADO")
        print(f"  ⏱️  Duración: {duration}")
        print(f"  📊 Incidentes totales: {self.incident_count}")
        print("🔴" * 30 + "\n")
        self.exam_started = False
    
    def handle_incident(self, frame, items):
        self.incident_count += 1
        ts = datetime.now()
        
        # ACTIVAR LED
        print("\n🚨 ¡OBJETO PROHIBIDO DETECTADO!")
        self.trigger_led(1)
        time.sleep(2)
        self.trigger_led(0)
        
        # Guardar
        filename = f"incident_{self.incident_count}_{ts.strftime('%H%M%S')}.jpg"
        cv2.imwrite(os.path.join(IMAGES_PATH, filename), frame)
        
        # Alerta
        print("\n" + "🚨" * 30)
        print(f"  ⚠️  INCIDENTE #{self.incident_count}")
        print(f"  ⏰ {ts.strftime('%H:%M:%S')}")
        print(f"  📍 Objetos detectados:")
        for item in items:
            name = PROHIBITED.get(item['class_id'], item['name'])
            print(f"     - {name.upper()} (confianza: {item['conf']:.0%})")
        print(f"  💾 Imagen guardada: {filename}")
        print("🚨" * 30 + "\n")
    
    def draw_ui(self, frame, detections):
        h, w = frame.shape[:2]
        
        # Panel superior
        overlay = frame.copy()
        cv2.rectangle(overlay, (0, 0), (w, 100), (0, 0, 0), -1)
        cv2.addWeighted(overlay, 0.7, frame, 0.3, 0, frame)
        
        # Título
        color = (0, 255, 0) if self.exam_started else (100, 100, 100)
        cv2.putText(frame, "MODO EXAMEN - VIGILANCIA", (15, 30),
                cv2.FONT_HERSHEY_SIMPLEX, 0.8, color, 2)
        
        # Estado
        if self.exam_started:
            elapsed = str(datetime.now() - self.exam_start_time).split('.')[0]
            cv2.putText(frame, f"Tiempo: {elapsed}", (15, 60),
                    cv2.FONT_HERSHEY_SIMPLEX, 0.6, (255, 255, 255), 2)
        else:
            cv2.putText(frame, ">>> HAZ CLIC AQUI Y PRESIONA 'S' <<<", (15, 60),
                    cv2.FONT_HERSHEY_SIMPLEX, 0.6, (255, 255, 0), 2)
        
        # LED
        led_txt = "LED: ACTIVO" if self.led_available else "LED: INACTIVO"
        led_col = (0, 255, 0) if self.led_available else (0, 0, 255)
        cv2.putText(frame, led_txt, (15, 85),
                cv2.FONT_HERSHEY_SIMPLEX, 0.5, led_col, 2)
        
        # Stats
        cv2.putText(frame, f"Incidentes: {self.incident_count}", (w-200, 30),
                cv2.FONT_HERSHEY_SIMPLEX, 0.6, (255, 255, 255), 2)
        cv2.putText(frame, f"Personas: {self.person_count}", (w-200, 60),
                cv2.FONT_HERSHEY_SIMPLEX, 0.6, (255, 255, 255), 2)
        
        # Detecciones
        for det in detections:
            x1, y1, x2, y2 = det['bbox']
            
            if det['prohibited']:
                color = (0, 0, 255)
                thickness = 3
                name = PROHIBITED.get(det['class_id'], det['name'])
                label = f"PROHIBIDO: {name.upper()} {det['conf']:.0%}"
                # Parpadeo
                if int(time.time() * 2) % 2 == 0:
                    thickness = 5
            else:
                color = (0, 255, 0)
                thickness = 2
                label = f"{det['name']} {det['conf']:.0%}"
            
            cv2.rectangle(frame, (x1, y1), (x2, y2), color, thickness)
            
            (tw, th), _ = cv2.getTextSize(label, cv2.FONT_HERSHEY_SIMPLEX, 0.5, 2)
            cv2.rectangle(frame, (x1, y1-th-8), (x1+tw+8, y1), color, -1)
            cv2.putText(frame, label, (x1+4, y1-4),
                    cv2.FONT_HERSHEY_SIMPLEX, 0.5, (255, 255, 255), 2)
        
        # Alerta grande
        proh = sum(1 for d in detections if d['prohibited'])
        if proh > 0 and self.exam_started:
            warn = f"⚠️ {proh} OBJETO(S) PROHIBIDO(S) DETECTADO(S) ⚠️"
            (tw, th), _ = cv2.getTextSize(warn, cv2.FONT_HERSHEY_SIMPLEX, 0.8, 3)
            tx = (w - tw) // 2
            
            if int(time.time() * 3) % 2 == 0:
                cv2.rectangle(frame, (tx-10, h-70), (tx+tw+10, h-20), (0, 0, 255), -1)
                cv2.putText(frame, warn, (tx, h-35),
                        cv2.FONT_HERSHEY_SIMPLEX, 0.8, (255, 255, 255), 3)
        
        return frame
    
    def run(self):
        fps = 0
        fps_count = 0
        fps_time = time.time()
        
        print("🎬 Iniciando vigilancia...")
        print("\n⚠️  IMPORTANTE: HAZ CLIC EN LA VENTANA 'Modo Examen'")
        print("   para poder usar los controles (S, E, Q)\n")
        
        try:
            while True:
                # Capturar
                frame = self.capture_frame()
                if frame is None:
                    time.sleep(0.05)
                    continue
                
                # YOLO
                results = self.model(frame, verbose=False)
                
                detections = []
                self.person_count = 0
                
                for result in results:
                    for box in result.boxes:
                        conf = float(box.conf[0])
                        if conf < CONFIDENCE:
                            continue
                        
                        class_id = int(box.cls[0])
                        class_name = self.model.names[class_id]
                        x1, y1, x2, y2 = map(int, box.xyxy[0])
                        
                        if class_name == 'person':
                            self.person_count += 1
                        
                        detections.append({
                            'class_id': class_id,
                            'name': class_name,
                            'conf': conf,
                            'bbox': (x1, y1, x2, y2),
                            'prohibited': class_id in PROHIBITED
                        })
                
                # Revisar incidentes
                prohibited = [d for d in detections if d['prohibited']]
                if prohibited and self.exam_started:
                    self.handle_incident(frame, prohibited)
                
                # Dibujar
                frame = self.draw_ui(frame, detections)
                
                # FPS
                fps_count += 1
                if time.time() - fps_time >= 1:
                    fps = fps_count
                    fps_count = 0
                    fps_time = time.time()
                
                cv2.putText(frame, f"FPS: {fps}", (frame.shape[1]-80, frame.shape[0]-10),
                        cv2.FONT_HERSHEY_SIMPLEX, 0.5, (0, 255, 0), 2)
                
                # Mostrar
                cv2.imshow("Modo Examen - Vigilancia Activa", frame)
                
                # Controles
                key = cv2.waitKey(1) & 0xFF
                
                if key == ord('s') or key == ord('S'):
                    if not self.exam_started:
                        self.start_exam()
                
                elif key == ord('e') or key == ord('E'):
                    if self.exam_started:
                        self.end_exam()
                        break
                
                elif key == ord('q') or key == ord('Q'):
                    break
        
        except KeyboardInterrupt:
            print("\n\n⚠️ Interrumpido por usuario")
        
        finally:
            if self.exam_started:
                self.end_exam()
            cv2.destroyAllWindows()
            print("\n✓ Sistema cerrado correctamente\n")


if __name__ == "__main__":
    print("\n" + "=" * 60)
    print("  🎓 Sistema Modo Examen")
    print("  📹 Vigilancia con YOLO + ESP32-CAM")
    print("=" * 60)
    
    try:
        monitor = ExamMonitor()
        monitor.run()
    except Exception as e:
        print(f"\n✗ Error fatal: {e}")
        import traceback
        traceback.print_exc()