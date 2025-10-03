import socket
import threading
import telemetry_client as tc
import time
import car as car_module


# ====== Configuración del cliente ======
SERVER_IP = "127.0.0.1"  # Cambiar por IP del servidor si no es local
SERVER_PORT = 2000

client = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
client.connect((SERVER_IP, SERVER_PORT))
print("Conectado al servidor.")

def parse_data(data):
    data = data[4:]  # quitar "PTT"
    print("[RAW DATA] ", data)
    try:
        data = data[5:]
        print("[RAW DATA NO LABEL]" + data)
        parts = data.replace("END", "").split(";")

        if len(parts) != 4:
            raise ValueError("Cantidad incorrecta de campos: " + str(parts))

        speed, direction, battery, temp = parts  #componenetes del mensaje

        print("[DEBUG LEN] ", len(speed))
        speed = speed.strip()         # elimina espacios y caracteres invisibles
        speed = speed.replace("\x00", "")  # elimina bytes nulos si los hay
        speed = speed[6:]       # "speed="
        direction = direction[4:]  # "dir="
        battery = battery[8:]   # "battery="
        temp = temp[5:]         # "temp="

        print("[PARSED SPEED]", speed)
        print("[PARSED DIRECTION]", direction)
        print("[PARSED BATTERY]", battery)
        print("[PARSED TEMP]", temp)
        
        return speed, temp, direction, battery

    except Exception as e:
        print("Error parsing data:", e)
    return None


# ====== Hilo de recepción de datos ======
def receive_thread():
    buffer = ""
    while True:
        try:
            chunk = client.recv(1024)
            if not chunk:
                print("Servidor desconectado.")
                break

            buffer += chunk.decode()
            
            print("====================")

            # Procesar todos los mensajes completos que contengan "END"
            i = 0
            while "END" in buffer:
                msg, buffer = buffer.split("END", 1)  # separar el primero
                print("\n[TELEMETRÍA] ", i , ": " + msg + "END")
                parsed = parse_data(msg + "END")
                if parsed:
                    speed, temp, dir, battery = parsed
                    print("[OK]", speed, temp, dir, battery)
                    #update_telemetry_data(speed, temp, dir, battery)
                    car_module.car.updateState(speed, dir, battery, temp)
                    tc.root.after(0, lambda: tc.update_telemetry(speed, temp, dir, battery))
                i += 1

        except Exception as e:
            print("Error recibiendo datos:", e)
            break
        
def send_thread():
    try:
        while True:
            entry = input("Comando> ")   # Entrada del usuario
            if entry.lower() == "exit":
                print("Saliendo del cliente...")
                break
            client.sendall(entry.encode())  # Envía comando al servidor
    finally:
        client.close()
            

def update_telemetry_data(speed, dir, battery, temp):
    while True:
        time.sleep(5)
        #tc.update_telemetry(0, 0, "NONE", 0)
        #tc.root.after(0, lambda: tc.update_telemetry(0, 0, "NONE", 0))
        car_module.car.updateState(speed, dir, battery, temp)  
        

# ====== Lanzar hilos ======
threading.Thread(target=receive_thread, daemon=True).start()
threading.Thread(target=send_thread, daemon=True).start()

# ====== Hilo principal de la interfaz ======
tc.root.mainloop()
