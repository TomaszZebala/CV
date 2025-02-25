# UDP Client Application

This project implements a simple UDP client application in C. It communicates with multiple UDP servers, listens for incoming messages, sends random messages to connected servers, and ensures that the servers remain active with periodic "keep-alive" messages. The client is capable of handling dynamic server connections and maintaining an updated list of active servers.

## Features

- **Listening for incoming messages**: The client continuously listens for UDP packets from servers. When a server sends a "HELLO" message, it stores the server's information (IP address, port, and token).

- **Random message generation**: The application randomly generates a message in the format <Token>-><Random Message> and sends it to a connected UDP server. The server processes the message and responds, typically with a confirmation or error message. The client measures the time it takes for the server to respond, logging the duration in microseconds along with the timestamp of the request.

- **Keep-alive mechanism**: The client periodically sends "KEEP_ALIVE" messages to all connected servers to ensure that the servers remain active. If a server does not respond within a set time frame, it is removed from the list of connected servers.

- **Server monitoring**: The client actively monitors all connected servers, checking their status and removing any inactive servers from the list.

- **Buffer management**: When only one server is connected, the client clears the buffer every 5 seconds to maintain system performance.



