from flask import Flask, request, jsonify, send_from_directory
from google.cloud import speech
import os
import json
import tempfile
import time
import requests
from dotenv import load_dotenv
import wave

load_dotenv()

app = Flask(__name__)

# Configurar credenciales desde variable de entorno
if "GOOGLE_CLOUD_CREDENTIALS" in os.environ:
    with tempfile.NamedTemporaryFile(mode='w', delete=False, suffix='.json') as temp:
        temp.write(os.environ["GOOGLE_CLOUD_CREDENTIALS"])
        temp_path = temp.name
    os.environ["GOOGLE_APPLICATION_CREDENTIALS"] = temp_path
    print(f"Credenciales cargadas desde variable de entorno a {temp_path}")
else:
    print("ADVERTENCIA: No se encontró la variable GOOGLE_CLOUD_CREDENTIALS")

speech_client = speech.SpeechClient()

# Directorio para guardar audios
AUDIO_DIR = "recordings"
if not os.path.exists(AUDIO_DIR):
    os.makedirs(AUDIO_DIR)

# URL del backend Node.js (ajusta si es necesario, e.g., si corres local o en render)
NODE_BACKEND_URL = "https://api-tresa.onrender.com/api/recordings" 
# Si estás probando localmente, usa:
# NODE_BACKEND_URL = "http://localhost:3000/api/recordings"

@app.route('/upload_stream', methods=['POST'])
def upload_stream():
    try:
        # Generar nombre de archivo único
        filename = f"rec_{int(time.time())}.wav"
        filepath = os.path.join(AUDIO_DIR, filename)
        
        print(f"Recibiendo stream de audio... Guardando en {filepath}")

        # Guardar el stream directamente a un archivo
        # El ESP32 enviará RAW PCM (16-bit, 16kHz, Mono)
        # Necesitamos guardarlo y luego convertirlo o envolverlo en WAV
        
        # Opción A: Guardar raw y luego convertir
        # Opción B: Escribir cabecera WAV y luego los datos (más eficiente si sabemos el tamaño, pero en stream no siempre)
        # Opción C: Guardar raw y usar speech config para raw
        
        # Vamos a guardar los datos raw tal cual llegan
        with open(filepath, 'wb') as f:
            # Flask stream processing
            chunk_size = 4096
            while True:
                chunk = request.stream.read(chunk_size)
                if len(chunk) == 0:
                    break
                f.write(chunk)
        
        print("Stream finalizado. Verificando tamaño...")
        
        file_size = os.path.getsize(filepath)
        print(f"Tamaño del archivo: {file_size} bytes")

        # Filtrar grabaciones muy cortas (ruido o falsos contactos)
        # 16kHz * 2 bytes * 0.5s = ~16000 bytes. 
        # Usaremos 4096 bytes como mínimo seguro.
        if file_size < 4096:
            print("Archivo muy pequeño (posible ruido), ignorando.")
            os.remove(filepath)
            return jsonify({"status": "ignored", "message": "Audio too short"}), 200

        transcription = ""
        try:
            # Transcribir con Google Speech
            with open(filepath, 'rb') as audio_file:
                content = audio_file.read()

            audio = speech.RecognitionAudio(content=content)
            config = speech.RecognitionConfig(
                encoding=speech.RecognitionConfig.AudioEncoding.LINEAR16,
                sample_rate_hertz=16000,
                language_code="es-MX",
            )

            response = speech_client.recognize(config=config, audio=audio)

            for result in response.results:
                transcription += result.alternatives[0].transcript
            
            print(f"Transcripción: {transcription}")

        except Exception as trans_error:
            print(f"⚠️ Error en transcripción: {trans_error}")
            transcription = "[Error de Transcripción - Audio Guardado]"

        # Notificar al servidor Node.js
        # La URL del archivo será accesible desde este servidor Python
        # Asumimos que este servidor es accesible públicamente o localmente
        # Para Render, necesitamos la URL pública de este servicio
        # Por ahora usaremos una URL relativa o IP local si es prueba
        
        # IMPORTANTE: Si esto corre en Render, necesitas la URL de TU servicio Python en Render
        # Ejemplo: https://mi-python-service.onrender.com/audio/rec_123.wav
        
        # Para simplificar, enviaremos el nombre del archivo y el frontend construirá la URL
        # O mejor, enviamos la ruta relativa
        file_url = f"/audio/{filename}" 

        try:
            requests.post(NODE_BACKEND_URL, json={
                "archivo_url": file_url,
                "transcripcion": transcription
            })
            print("Metadatos enviados a Node.js")
        except Exception as e:
            print(f"Error enviando a Node: {e}")

        return jsonify({
            "status": "success",
            "transcription": transcription,
            "file": filename
        }), 200

    except Exception as e:
        print(f"Error: {str(e)}")
        return jsonify({"status": "error", "message": str(e)}), 500

# Endpoint para servir los archivos de audio
@app.route('/audio/<path:filename>')
def serve_audio(filename):
    return send_from_directory(AUDIO_DIR, filename)

@app.route('/health', methods=['GET'])
def health():
    return jsonify({"status": "Server running"}), 200

if __name__ == '__main__':
    print("Servidor Python iniciado en puerto 5001")
    app.run(host='0.0.0.0', port=5001, debug=True)