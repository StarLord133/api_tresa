# Guía de Configuración: Google Sheets API

Sigue estos pasos para obtener las credenciales necesarias para conectar tu sistema con Google Sheets.

## Paso 1: Crear un Proyecto en Google Cloud
1. Ve a [Google Cloud Console](https://console.cloud.google.com/).
2. Inicia sesión con tu cuenta de Google.
3. En la parte superior izquierda, haz clic en el selector de proyectos y selecciona **"Nuevo Proyecto"**.
4. Dale un nombre (ej. `Sistema-Asistencia`) y haz clic en **"Crear"**.

## Paso 2: Habilitar la API de Google Sheets
1. En el menú lateral izquierdo, ve a **"API y servicios"** > **"Biblioteca"**.
2. En el buscador, escribe **"Google Sheets API"**.
3. Haz clic en el resultado y luego en el botón **"Habilitar"**.

## Paso 3: Crear una Cuenta de Servicio (Service Account)
1. Ve a **"API y servicios"** > **"Credenciales"**.
2. Haz clic en **"Crear credenciales"** (arriba) y selecciona **"Cuenta de servicio"**.
3. **Detalles de la cuenta de servicio**:
   - Nombre: `bot-asistencia` (o lo que prefieras).
   - Haz clic en **"Crear y continuar"**.
4. **Permisos** (Opcional): Puedes saltar este paso o darle rol de "Propietario" (Owner) para pruebas rápidas. Haz clic en **"Continuar"** y luego en **"Listo"**.

## Paso 4: Descargar la Clave JSON
1. En la lista de "Cuentas de servicio", haz clic en el email de la cuenta que acabas de crear (ej. `bot-asistencia@tu-proyecto.iam.gserviceaccount.com`).
2. Ve a la pestaña **"Claves"** (Keys).
3. Haz clic en **"Agregar clave"** > **"Crear clave nueva"**.
4. Selecciona **JSON** y haz clic en **"Crear"**.
5. Se descargará un archivo `.json` a tu computadora. **Guarda este archivo, es tu llave de acceso.**

## Paso 5: Compartir tu Google Sheet
1. Abre tu formulario de Google Forms y ve a la hoja de respuestas (Google Sheet).
2. Haz clic en el botón **"Compartir"** (Share) arriba a la derecha.
3. Abre el archivo JSON que descargaste y busca el campo `"client_email"`. Copia ese correo (ej. `bot-asistencia@tu-proyecto.iam.gserviceaccount.com`).
4. Pega ese correo en el cuadro de compartir del Google Sheet y dale permisos de **"Editor"** o **"Lector"** (Viewer es suficiente si solo vamos a leer).
5. Haz clic en **"Enviar"**.

## Paso 6: Obtener el ID de la Hoja (Spreadsheet ID)
1. Mira la URL de tu Google Sheet. Se ve algo así:
   `https://docs.google.com/spreadsheets/d/1aBcDeFgHiJkLmNoPqRsTuVwXyZ/edit#gid=0`
2. El ID es la parte larga entre `/d/` y `/edit`.
   - En este ejemplo, el ID es: `1aBcDeFgHiJkLmNoPqRsTuVwXyZ`

---

## ¿Qué necesito que me envíes?
1. **El contenido del archivo JSON** (puedes pegarlo aquí o subir el archivo a la carpeta del proyecto).
2. **El Spreadsheet ID** de tu hoja de respuestas.
