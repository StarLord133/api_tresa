from flask import Flask, request, jsonify
from google.cloud import speech
import os
import json
import tempfile
from dotenv import load_dotenv

load_dotenv()

app = Flask(__name__)

# Configurar credenciales desde variable de entorno
if "GOOGLE_CLOUD_CREDENTIALS" in os.environ:
    # Crear archivo temporal con las credenciales
    with tempfile.NamedTemporaryFile(mode='w', delete=False, suffix='.json') as temp:
        temp.write(os.environ["GOOGLE_CLOUD_CREDENTIALS"])
        temp_path = temp.name
    
    os.environ["GOOGLE_APPLICATION_CREDENTIALS"] = temp_path
    print(f"Credenciales cargadas desde variable de entorno a {temp_path}")
else:
    print("ADVERTENCIA: No se encontró la variable GOOGLE_CLOUD_CREDENTIALS")

# Cliente de Google Speech
speech_client = speech.SpeechClient()

@app.route('/upload_audio', methods=['POST'])
def upload_audio():
    try:
        # Recibir audio del ESP32
        audio_data = request.data
        
        print(f"Recibido audio: {len(audio_data)} bytes")
        
        # Configurar reconocimiento
        audio = speech.RecognitionAudio(content=audio_data)
        config = speech.RecognitionConfig(
            encoding=speech.RecognitionConfig.AudioEncoding.LINEAR16,
            sample_rate_hertz=16000,
            language_code="es-MX",
        )
        
        # Transcribir
        print("Transcribiendo...")
        response = speech_client.recognize(config=config, audio=audio)
        
        # Extraer texto
        transcription = ""
        for result in response.results:
            transcription += result.alternatives[0].transcript
        
        print(f"Transcripción: {transcription}")
        
        return jsonify({
            "status": "success",
            "transcription": transcription
        }), 200
        
    except Exception as e:
        print(f"Error: {str(e)}")
        return jsonify({"status": "error", "message": str(e)}), 500

@app.route('/health', methods=['GET'])
def health():
    return jsonify({"status": "Server running "}), 200

if __name__ == '__main__':
    print(" Servidor iniciado en http://0.0.0.0:5001")
    app.run(host='0.0.0.0', port=5001, debug=True)