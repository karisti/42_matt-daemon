# 42_matt-daemon

# Servidor Demonizado

## 1. Memoria de la aplicación

### 1.1. Descripción técnica

"Servidor Demonizado" es una aplicación en C++ que despliega un demonio (daemon) de sistema en Linux para gestionar conexiones TCP de clientes de manera eficiente mediante un bucle de eventos (`epoll`). El demonio se encarga de:

- Realizar las comprobaciones iniciales (ejecución como usuario root, bloqueo con archivo de lock en `/var/lock/matt_daemon.lock`).
- Desdoblarse en segundo plano usando dos llamadas a `fork()` para independizarse del terminal.
- Crear un servidor de sockets IPv4 en el puerto por defecto **4242**.
- Gestionar múltiples clientes de forma no bloqueante mediante **epoll**.
- Registrar eventos y errores en un fichero de log (`/var/log/matt_daemon.log`) con timestamp.

### 1.2. Entidades implicadas

- **MD::Daemon**: Clase principal que orquesta la creación del daemon, configuración de señales, bloqueo, ciclo de vida y limpieza.
- **MD::Server**: Implementa la lógica de red: creación de socket, bind, listen, bucle de eventos con `epoll`, aceptación de clientes y recepción de mensajes.
- **MD::Client**: Representa una conexión cliente: encapsula el `socket`, la dirección remota y un buffer de mensajes recibidos.
- **MD::Tintin_reporter**: Logger singleton que crea y gestiona el fichero de log, redirigiendo `stdout` y `stderr` al descriptor del log.
- **Funciones utilitarias** (en `utils.cpp`): split de cadenas, generación de pings aleatorios, gestión de señales y comprobación de cadenas numéricas.

### 1.3. Intercambios desarrollados

- **Handshake**: Al conectar, el cliente establece el socket TCP con el servidor.
- **Recepción de comandos**: El servidor lee datos no bloqueantes, elimina saltos de línea y analiza textos como "quit".
- **Cierre de conexión**: Si el cliente envía "quit" o se desconecta abruptamente, el servidor cierra el socket y actualiza el log.
- **Terminación global**: Si algún cliente envía "quit", se activa la bandera global `g_stopRequested` y finaliza el bucle del servidor.

### 1.4. Infraestructura de comunicaciones

- **Red**: TCP/IPv4.
- **Epoll**: Mecanismo de multiplexación de I/O en Linux para gestión escalable de múltiples sockets.
- **Puertos**: Escucha en cualquier IP (`0.0.0.0`) y puerto configurable (por defecto 4242).

### 1.5. Procesos y hilos

- **Procesos**:

  - Proceso padre inicia y finaliza tras el primer `fork()`.
  - Proceso demonio (proceso hijo tras doble fork) es el encargado de la lógica de servidor.

- **Hilos**:

  - No usa hilos; toda la concurrencia se gestiona con `epoll` en un único hilo de ejecución.

### 1.6. Protocolos y formatos

- **TCP**: Transporte de datos.
- **Protocolos internos**: Texto plano con terminación en newline; comandos sencillos (p.ej. `quit`).
- **Logs**: Formato `[dd/mm/YYYY-HH:MM:SS] [ LEVEL ] - Reporter: mensaje`.

### 1.7. Clases y procedimientos implementados

- **MD::Daemon**: `initialChecks()`, `daemonize()`, `create()`, `run()`, `stop()`, `remove()`, `configSignals()`, `signalHandler()`.
- **MD::Server**: `createNetwork()`, `loop()`, `clientConnected()`, `receiveMessage()`, `clientDisconnected()`, `terminateServer()`, `saveIp()`, `throwError()`.
- **MD::Client**: `startListeningSocket()`, getters y buffer management.
- **MD::Tintin_reporter**: `getInstance()`, `create()`, `createLogFile()`, `openLogFile()`, `log()`, `getCurrentTimestamp()`.
- **utils**: `splitString()`, `pingGenerator()`, `isNumber()`, `printStrVector()`, manejo de señales (`catchSignal()`).

### 1.8. Librerías usadas

- **Estándar C/C++**: `<iostream>`, `<vector>`, `<string>`, `<ctime>`, `<cstring>`, `<cstdlib>`, `<unistd.h>`, `<fcntl.h>`.
- **Sistema**: `<sys/socket.h>`, `<netinet/in.h>`, `<arpa/inet.h>`, `<sys/epoll.h>`, `<netdb.h>`, `<sys/file.h>`, `<signal.h>`.

---

## 2. Manual de usuario

### 2.1. Requisitos previos

- **Sistema operativo**: Linux (kernel con soporte epoll).
- **Permisos**: Usuario **root** para ejecutar el demonio.
- **Puerto**: 4242 (o el configurado) libre.
- **Fichero Ejecutable**: `/bin/MattDaemon`
- **Docker y Docker Compose**: En caso de no disponer de un sistema Linux nativo, se incluye un fichero `docker-compose.yml` para desplegar la aplicación en un contenedor:

  1. Instalar [Docker](https://docs.docker.com/get-docker/) y [Docker Compose](https://docs.docker.com/compose/install/).
  2. Levantar el contenedor que copiara el fichero ejecutable dentro del Docker

     ```bash
     docker-compose up -d
     ```

  3. Acceder al contenedor

     ```bash
     docker-compose exec debian bash
     ```

  4. Acceder al directorio del proyecto dentro del contenedor
     ```bash
     cd
     ```

### 2.2. Instalación y despliegue

1. Como se proporciona el fichero ejecutable solamente ejecutandolo ya arranca el servidor:

   ```bash
   ./MattDaemon
   ```

2. Para **conectar un cliente** al servidor mirar apartado **2.4**

#### 2.2.1 Instalación Adicional

En caso necesario desde la raiz del repositorio se puede copiar el proyecto y compilarlo desde el contenedor
   ```bash
   docker cp . debian:/root/ServidorDemonizado/
   ```

Después se puede ejecutar
   ```bash
   make && ./bin/MattDaemon
   ```

### 2.3. Ficheros generados

- **/var/log/matt_daemon.log**: Registro de actividad y errores.
- **/var/lock/matt_daemon.lock**: Archivo de lock que impide ejecuciones concurrentes.

### 2.4. Uso de la aplicación

#### 2.4.1. Ver logs

Para monitorizar la actividad:

```bash
tail -f /var/log/matt_daemon.log
```

#### 2.4.2. Conectar clientes

Desde otra terminal `Linux`, usar **telnet** o **netcat**:

```bash
telnet localhost 4242
# o
nc localhost 4242
```

Desde otra terminal Powershell `Windows`:

   ```bash
   Test-NetConnection -Computername localhost -Port 4242
   ```
Esta solamente captara la conexion y desconexion del cliente.

#### 2.4.3. Comandos disponibles

- **quit**: Cierra la conexión actual y, si se envía desde cualquier cliente, detiene el servidor demonizado.
- **cualquier otro texto**: El servidor lo registra en el log pero no responde.

### 2.5. Interfaz de usuario

No existe interfaz gráfica. La interacción se realiza mediante clientes de línea de comandos que establecen una conexión TCP con el demonio. Los elementos de "interfaz" son:

- **Consola del servidor**: No interactiva (todo está en background). Se revisa vía log.
- **Cliente de terminal**:

  - **Línea de comandos**: Donde escribir los comandos.
  - **Indicador de conexión**: Se muestra mensaje de bienvenida genérico de telnet/netcat.

---

_Fin del documento._


--------------------------------------

## What is a daemon?
A daemon is a background process that runs continuously and performs specific tasks or waits for events to occur. Daemons are typically used in Unix-like operating systems to handle system-level tasks, such as managing hardware devices, handling network requests, or performing scheduled jobs.



## Commands
- **See processes**: `ps aux | grep MattDaemon`



## Resources
- https://www.youtube.com/watch?v=65DarzNIFR0


## TODO:
- [X] BUG: El cliente siempre pone que es el numero 5 al conectarse: (New client connecting: 5)
- [X] Max clients: Si ya se ha alcanzado el maximo de clientes, no se acepta la conexion y se envia un mensaje al cliente.
- [X] Asegurar que no pete con mensajes "raros" o "especiales".
- [X] LOCK_UN
- [X] Manejo señales: señal ctrl z, ...
- [X] Al intentar ejecutar un segundo Daemon: an error message indicating a creation/file opening on matt_daemon.lock attempt must pop.
- [X] Pensar error management
- [X] Refactor code, error management, config file (puerto donde se escucha, ...) ...

- [ ] Testeo god mode By Gonzalo.
- [ ] Revisar corrección.
- [ ] Poner readme bonito. Comentarios en el código, documentación, etc.


| Comando    | Señal     | Número | Qué hace                                                  |
| ---------- | --------- | ------ | --------------------------------------------------------- |
| `kill -15` | `SIGTERM` | 15     | Termina el proceso de forma **ordenada**                  |
| `kill -9`  | `SIGKILL` | 9      | Termina el proceso **inmediatamente** (no puede ser capturada por  que es una señal privilegiada del sistema que haria el proceso incontrolable) |
| `kill -2`  | `SIGINT`  | 2      | Como `Ctrl+C`, interrumpe el proceso                      |
| `kill -3`  | `SIGQUIT`  | 2      | Como `Ctrl+\`, interrumpe el proceso                      |
| `kill -19` | `SIGSTOP` | 19     | Pausa (suspende) el proceso (como `Ctrl+Z`) (no puede ser capturada por  que es una señal privilegiada del sistema que haria el proceso incontrolable)              |
| `kill -20` | `SIGTSTP` | 20      | Suspender con posibilidad de manejo `Ctrl+Z` |
