import serial
import threading
from flask import Flask, render_template
from flask_socketio import SocketIO

app = Flask(__name__)
socketio = SocketIO(app, cors_allowed_origins="*")

# --- Configuration ---
# Change 'COM3' to your actual port (e.g., '/dev/ttyUSB0' on Linux/Mac)
SERIAL_PORT = 'COM3' 
BAUD_RATE = 115200

try:
    ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)
except Exception as e:
    print(f"Error opening serial port: {e}")
    ser = None

def read_from_serial():
    """Reads serial data in a background thread."""
    while True:
        if ser and ser.in_available > 0:
            try:
                line = ser.readline().decode('utf-8').strip()
                if line:
                    # Emit the data to the 'serial_data' event in the Web UI
                    socketio.emit('serial_data', {'value': line})
            except:
                pass

@app.route('/')
def index():
    return render_template('index.html')

if __name__ == '_main_':
    # Start the serial reading thread
    thread = threading.Thread(target=read_from_serial)
    thread.daemon = True
    thread.start()
    
    # Run the web server
    socketio.run(app, debug=True, port=5000)