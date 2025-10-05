# NetDrive - Sistema de Telemetría para Vehículo Autónomo

## Descripción del Proyecto

NetDrive es un sistema completo de telemetría y control para un vehículo autónomo. El sistema permite a múltiples usuarios conectarse simultáneamente para visualizar datos de telemetría en tiempo real y, en el caso de administradores, enviar comandos de control al vehículo.

## Arquitectura del Sistema

El sistema está compuesto por tres componentes principales:

### 1. Servidor (C)
- Servidor TCP multihilo implementado en C
- Gestiona múltiples conexiones de clientes concurrentes
- Sistema de autenticación y roles (Admin/Observer)
- Envía datos de telemetría cada 10 segundos a todos los clientes autenticados
- Procesa comandos de control del vehículo

### 2. Clientes Python
- **Cliente Observador**: Visualiza datos de telemetría en tiempo real
- **Cliente Administrador**: Envía comandos y gestiona usuarios
- Interfaz gráfica usando CustomTkinter
- Autenticación automática al servidor

### 3. Cliente JavaScript
- Cliente web unificado con interfaz de login
- Interfaz adaptativa según el rol (Admin/Observer)
- Comunicación mediante WebSocket-TCP bridge
- Visualización de telemetría y panel de control para administradores

## Protocolo PTT (Proprietary Telemetry Transfer)

El protocolo PTT es un protocolo de capa de aplicación basado en texto diseñado específicamente para este sistema.

### Estructura de Mensajes

Todos los mensajes siguen este formato:
```
PTT + ACTION(12 bytes) + DATA(150 bytes) + END
```

- **PTT**: Header de 3 bytes que identifica el inicio del mensaje
- **ACTION**: 12 bytes con padding de espacios que indica el tipo de mensaje
- **DATA**: 150 bytes con padding de espacios que contiene los datos
- **END**: Footer de 3 bytes que marca el fin del mensaje

Total: 168 bytes por mensaje

### Tipos de Acciones

- **LOGIN**: Autenticación de usuario (formato: `username;password`)
- **OK**: Respuesta exitosa del servidor
- **ERROR**: Respuesta de error del servidor
- **DENIED**: Permiso denegado
- **DATA**: Datos de telemetría (formato: `speed=X;dir=Y;battery=Z;temp=W`)
- **COMMAND**: Comando de control del vehículo
- **LIST**: Solicitud de lista de usuarios conectados (solo Admin)

### Flujo de Comunicación

1. **Conexión**: Cliente se conecta al servidor TCP en el puerto 2000
2. **Autenticación**: Cliente envía mensaje LOGIN con credenciales
3. **Respuesta**: Servidor valida y responde con OK o ERROR
4. **Telemetría**: Servidor envía datos cada 10 segundos a clientes autenticados
5. **Comandos**: Clientes Admin pueden enviar comandos COMMAND
6. **Desconexión**: Cliente cierra conexión y servidor limpia sesión

## Sistema de Autenticación

### Usuarios Predefinidos

El sistema incluye los siguientes usuarios por defecto:

| Usuario   | Contraseña    | Rol       |
|-----------|---------------|-----------|
| admin     | admin123      | ADMIN     |
| observer  | observer123   | OBSERVER  |
| user1     | pass1         | OBSERVER  |
| root      | root          | ADMIN     |

### Roles y Permisos

**ADMIN (Administrador)**
- Visualizar datos de telemetría en tiempo real
- Enviar comandos de control al vehículo
- Ver lista de usuarios conectados
- Acceder a panel de administración

**OBSERVER (Observador)**
- Visualizar datos de telemetría en tiempo real
- Sin permisos para enviar comandos
- Sin acceso a panel de administración

## Datos de Telemetría

El vehículo transmite los siguientes datos cada 10 segundos:

- **Velocidad**: 0-120 km/h
- **Dirección**: 0-359 grados (convertido a puntos cardinales)
- **Batería**: 0-100%
- **Temperatura**: Temperatura interna en grados Celsius

## Comandos de Control

Los administradores pueden enviar los siguientes comandos:

- **SPEED UP**: Incrementa la velocidad en 10 km/h (máximo 120 km/h)
- **SLOW DOWN**: Reduce la velocidad en 10 km/h (mínimo 0 km/h)
- **TURN LEFT**: Gira 45 grados a la izquierda
- **TURN RIGHT**: Gira 45 grados a la derecha

### Efectos de los Comandos

- La batería disminuye 1% con cada comando ejecutado
- La temperatura aumenta cuando el vehículo está en movimiento
- La temperatura disminuye cuando el vehículo está detenido

## Requisitos del Sistema

### Servidor (C)
- Compilador GCC
- Biblioteca pthread (para multithreading)
- Sistema operativo compatible con Berkeley Sockets API

### Clientes Python
- Python 3.7 o superior
- Bibliotecas requeridas:
  - `customtkinter`
  - `Pillow` (PIL)

### Cliente JavaScript
- Node.js 14 o superior
- npm o yarn
- Navegador web moderno (Chrome, Firefox, Edge)

## Instalación y Configuración

### 1. Servidor C

```bash
# Navegar al directorio del servidor
cd server

# Compilar el servidor
gcc -o server main.c car.c auth.c protocol.c -pthread

# Ejecutar el servidor
./server
```

El servidor se iniciará en el puerto 2000 por defecto.

### 2. Cliente Python - Observador

```bash
# Navegar al directorio del cliente Python
cd clients/python_client

# Crear entorno virtual (opcional pero recomendado)
python -m venv venv

# Activar entorno virtual
# En Windows:
venv\Scripts\activate.ps1
# En Linux/Mac:
source venv/bin/activate

# Instalar dependencias
pip install -r requirements.txt

# Ejecutar cliente observador
python client.py
```

El cliente se autenticará automáticamente como `observer/observer123`.

### 3. Cliente Python - Administrador

```bash
# En el mismo directorio clients/python_client
# Con el entorno virtual activado

# Ejecutar cliente administrador
python admin_client.py
```

Credenciales por defecto: `admin/admin123`

### 4. Cliente JavaScript (Web)

```bash
# Navegar al directorio del cliente JavaScript
cd clients/js_client

# Instalar dependencias
npm install

# Terminal 1: Iniciar el WebSocket-TCP bridge
npm run server

# Terminal 2: Iniciar el servidor HTTP
npx http-server -p 3000

# Abrir navegador
# http://localhost:3000
```

## Uso del Sistema

### Cliente Observador (Python)

1. Ejecutar `python client.py`
2. La interfaz gráfica se abrirá automáticamente
3. Los datos de telemetría se actualizarán en tiempo real
4. No requiere login manual (se autentica automáticamente)

### Cliente Administrador (Python)

1. Ejecutar `python admin_client.py`
2. Ingresar credenciales de administrador
3. Usar botones de control para enviar comandos
4. Hacer clic en "Actualizar Lista" para ver usuarios conectados

### Cliente Web (JavaScript)

1. Abrir `http://localhost:3000` en el navegador
2. Ingresar credenciales (admin o observer)
3. La interfaz se adaptará según el rol:
   - **Observer**: Solo visualización de telemetría
   - **Admin**: Telemetría + panel de control + lista de usuarios

## Estructura del Proyecto

```
Netdrive/
├── server/                    # Servidor en C
│   ├── main.c                 # Lógica principal y manejo de clientes
│   ├── car.c                  # Lógica del vehículo y telemetría
│   ├── car.h
│   ├── auth.c                 # Sistema de autenticación
│   ├── auth.h
│   ├── protocol.c             # Implementación del protocolo PTT
│   └── protocol.h
│
├── clients/
│   ├── python_client/         # Clientes en Python
│   │   ├── client.py          # Cliente observador
│   │   ├── admin_client.py    # Cliente administrador
│   │   ├── telemetry_client.py # GUI para telemetría
│   │   ├── car.py             # Modelo del carro
│   │   └── images/
│   │       └── auto.png
│   │
│   └── js_client/             # Cliente web en JavaScript
│       ├── index.html         # Interfaz unificada (login + dashboard)
│       ├── server.js          # WebSocket-TCP bridge
│       ├── package.json
│       └── images/
│           └── auto.png
│
└── README.md                  # Este archivo
```

## Características Técnicas

### Concurrencia
- El servidor utiliza hilos (threads) para manejar múltiples clientes simultáneamente
- Cada cliente se ejecuta en su propio hilo
- Un hilo dedicado envía telemetría cada 10 segundos

### Sincronización
- Uso de mutexes para proteger estructuras de datos compartidas
- `clients_lock`: Protege el array de file descriptors de clientes
- `car_lock`: Protege el estado del vehículo
- `sessions_lock`: Protege las sesiones de usuarios

### Gestión de Sesiones
- Cada cliente autenticado recibe un token único
- Las sesiones se mantienen por file descriptor
- Las sesiones se limpian al desconectarse el cliente

### Manejo de Errores
- El servidor valida todos los mensajes recibidos
- Los clientes manejan desconexiones inesperadas
- Logs detallados para debugging

## Logs del Sistema

El servidor genera logs en consola para monitorear la actividad:

```
[SERVIDOR] Escuchando en el puerto 2000...
[SERVIDOR] Nuevo cliente conectado (fd=4)
[HILO] Cliente conectado (fd=4) desde 127.0.0.1:12345
[AUTH] Login exitoso: admin (role=2) fd=4
[TELEMETRIA] Enviada a admin (fd=4)
[COMMAND] Ejecutado 'SPEED UP' por admin fd=4
[CAR] Velocidad incrementada a 10 km/h
```

## Solución de Problemas

### El servidor no compila
- Verificar que GCC esté instalado: `gcc --version`
- Verificar que pthread esté disponible
- En Windows, usar MinGW o WSL

### Los clientes Python no se conectan
- Verificar que el servidor esté ejecutándose
- Verificar la IP y puerto (127.0.0.1:2000 por defecto)
- Verificar que no haya firewall bloqueando el puerto

### El cliente web no funciona
- Verificar que el bridge WebSocket esté ejecutándose (`npm run server`)
- Verificar que el servidor HTTP esté ejecutándose
- Verificar que el servidor C esté ejecutándose
- Usar Ctrl+Shift+R para limpiar caché del navegador

### Los comandos no funcionan
- Verificar que el usuario sea ADMIN
- Revisar logs del servidor para ver si el comando se recibe
- Verificar que el cliente esté autenticado

## Desarrollo y Contribución

### Agregar Nuevos Usuarios

Editar `server/auth.c`, sección `users_db`:

```c
User users_db[] = {
    {"nuevo_usuario", "contraseña", ROLE_ADMIN},
    // ... usuarios existentes
};
int users_db_count = 5; // Actualizar el contador
```

### Agregar Nuevos Comandos

1. Editar `server/car.c`, función `updateCarTelemetry()`
2. Agregar nuevo caso en el if-else para el comando
3. Actualizar clientes para incluir el botón del nuevo comando

### Modificar Intervalo de Telemetría

Editar `server/main.c`, función `send_telemetry()`:

```c
sleep(10); // Cambiar 10 por el número de segundos deseado
```

