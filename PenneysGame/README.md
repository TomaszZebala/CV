# UDP Client Application

## Description
This project implements a simple UDP client application in C that communicates with multiple UDP servers. The client listens for incoming messages, sends random messages to connected servers, and ensures that the servers remain active through periodic "keep-alive" messages. The client is capable of handling dynamic server connections and maintains an updated list of active servers.

## Features

- **Listening for Incoming Messages**:  
  The client continuously listens for UDP packets from servers. When a server sends a "HELLO" message, the client stores the server's information (IP address, port, and token) for future communication.

- **Random Message Generation**:  
  The application randomly generates a message and sends it to a connected UDP server. The server processes the message and responds with a confirmation or error message. The client measures the response time, logs the duration in microseconds, and records the timestamp of the request.

- **Keep-alive Mechanism**:  
  Periodically, the client sends "KEEP_ALIVE" messages to all connected servers to ensure that the servers remain active. If a server does not respond within a set time frame, it is removed from the list of connected servers.

- **Server Monitoring**:  
  The client actively monitors all connected servers by checking their status. Inactive servers that do not respond are removed from the list.

- **Buffer Management**:  
  When only one server is connected, the client clears the message buffer every 5 seconds to maintain system performance and avoid potential memory overuse.

