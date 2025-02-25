# Penney's Ante Game

## Description
This project implements a UDP-based client application designed for a multiplayer game. The client communicates with a central game server and other clients over a UDP network, handling game state updates, player interactions, and synchronization mechanisms.
## Features

- **Game Registration**:
	-***The client sends a REGISTER request to the game server to join a match.***\n
	-***Upon successful registration, the server responds with necessary game information, including player ID and game session details.***
  
- **Game State Management**:  
	-***The client continuously listens for game updates from the server.***
	-***Handles game events such as player actions, state changes, and game progress updates.***
  
- **Message Exchange**:  
	-***Supports structured UDP message communication with predefined message types (GAME, WINNER, END).***
	-***Processes incoming packets efficiently to ensure real-time game performance.***
  
- **Buffering and Performance Optimization**:  
	-***Uses dedicated buffers for sending and receiving game-related messages.***
	-***Implements an efficient queue system for managing incoming and outgoing messages.***
  
- **Error Handling and Resilience**:  
	-***Detects and handles packet loss or corruption using retransmission mechanisms.***
	-***Ensures smooth operation by filtering invalid or out-of-sequence messages.***
  
