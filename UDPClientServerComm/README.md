# UDP Client Application

This project implements a simple UDP client application in C. It communicates with multiple UDP servers, listens for incoming messages, sends random messages to connected servers, and ensures that the servers remain active with periodic "keep-alive" messages. The client is capable of handling dynamic server connections and maintaining an updated list of active servers.

## Features

- **Listening for incoming messages**: The client continuously listens for UDP packets from servers. When a server sends a "HELLO" message, it stores the server's information (IP address, port, and token).

- **Random message generation**: The client periodically sends random messages to one of the connected servers. These messages are generated dynamically and include a random string of characters.

- **Keep-alive mechanism**: The client periodically sends "KEEP_ALIVE" messages to all connected servers to ensure that the servers remain active. If a server does not respond within a set time frame, it is removed from the list of connected servers.

- **Server monitoring**: The client actively monitors all connected servers, checking their status and removing any inactive servers from the list.

- **Buffer management**: When only one server is connected, the client clears the buffer every 5 seconds to maintain system performance.

## Workflow

1. **Initialization**: The client starts by setting up two communication channels — one for listening for incoming messages and another for sending messages to servers.

2. **Server Discovery**: When a server sends a "HELLO" message, the client records the server's details, such as its IP address, port number, and token.

3. **Sending Random Messages**: The client generates random messages and sends them to randomly chosen connected servers. These messages contain a unique token associated with the target server.

4. **Keep-Alive Messages**: The client sends periodic "KEEP_ALIVE" messages to each connected server. If a server fails to respond, it is removed from the active server list.

5. **Server Removal**: If a server does not respond to the "KEEP_ALIVE" message, it is removed from the list of active servers. The client sends a notification to the server about its disconnection.

