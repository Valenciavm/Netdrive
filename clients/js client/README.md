# Cliente de Telemetría - JavaScript

Réplica exacta de la interfaz de telemetría desarrollada en Python usando CustomTkinter, ahora implementada en JavaScript con HTML/CSS.

## Características

- **Interfaz idéntica** al cliente Python
- **Diseño responsivo** con CSS Grid
- **Tema oscuro** consistente
- **Imagen del carro** centrada
- **4 métricas de telemetría**:
  - Velocidad (km/h)
  - Batería (%)
  - Temperatura (°C)
  - Dirección (NORTE/SUR/ESTE/OESTE)

## Estructura

```
javascript_client/
├── index.html          # Interfaz principal
├── package.json        # Configuración del proyecto
├── README.md          # Documentación
└── images/
    └── auto.png       # Imagen del carro
```

## Instalación y Uso

### Opción 1: Servidor HTTP simple
```bash
cd clients/javascript_client
npx http-server -p 3000 -o
```

### Opción 2: Live Server (desarrollo)
```bash
cd clients/javascript_client
npm install
npm run dev
```

### Opción 3: Abrir directamente
Abrir `index.html` directamente en el navegador.

## Tecnologías

- **HTML5** - Estructura semántica
- **CSS3** - Estilos y layout con Grid
- **JavaScript** - Interactividad (preparado para futuras funcionalidades)

## Comparación con Python

| Característica | Python (CustomTkinter) | JavaScript (HTML/CSS) |
|----------------|------------------------|----------------------|
| Layout | `grid()` y `pack()` | CSS Grid |
| Colores | `fg_color` | `background-color` |
| Bordes redondos | `corner_radius` | `border-radius` |
| Fuentes | `CTkFont` | `font-family` |
| Imágenes | `ImageTk.PhotoImage` | `<img>` tag |
| Responsive | `weight` y `sticky` | `grid-template-columns` |

## Próximas Funcionalidades

- [ ] Actualización en tiempo real de datos
- [ ] WebSocket para comunicación con servidor
- [ ] Animaciones CSS
- [ ] Modo claro/oscuro
- [ ] Responsive design para móviles

