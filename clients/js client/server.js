const WebSocket = require('ws');
const net = require('net');

const WS_PORT = 8080;
const TCP_HOST = '127.0.0.1';
const TCP_PORT = 2000;

// WebSocket Server
const wss = new WebSocket.Server({ port: WS_PORT });

console.log(`[WS SERVER] Escuchando en puerto ${WS_PORT}`);
console.log('[INFO] Bridge WebSocket <-> TCP iniciado');
console.log('[INFO] Abre el navegador en: http://localhost:3000');

wss.on('connection', (ws) => {
    console.log('[WS] Cliente conectado desde navegador');
    
    // TCP Client
    const tcpClient = new net.Socket();
    
    tcpClient.connect(TCP_PORT, TCP_HOST, () => {
        console.log('[TCP] Conectado al servidor C');
    });
    
    // WS -> TCP: Forward messages from browser to C server
    ws.on('message', (data) => {
        const message = data.toString();
        console.log(`[WS -> TCP] ${message.substring(0, 50)}`);
        tcpClient.write(message);
    });
    
    // TCP -> WS: Forward messages from C server to browser
    tcpClient.on('data', (data) => {
        const message = data.toString();
        console.log(`[TCP -> WS] ${message.substring(0, 50)}`);
        
        if (ws.readyState === WebSocket.OPEN) {
            ws.send(message);
        }
    });
    
    // Handle disconnections
    ws.on('close', () => {
        console.log('[WS] Cliente desconectado');
        tcpClient.end();
    });
    
    tcpClient.on('close', () => {
        console.log('[TCP] Conexión cerrada');
        ws.close();
    });
    
    tcpClient.on('error', (err) => {
        console.error('[TCP] Error:', err.message);
        ws.close();
    });
});
