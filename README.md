# HTTP-Proxy

<p align="center">
  <img src="https://github.com/Sergiuas/HTTP-Proxy/blob/main/proxydock.png?raw=true" alt="alt text" width="400" height="400">
</p>

## Project Documentation

You can find the project documentation here: [file](./proxydock_srd.pdf).

## Overview

**proxydock** is a C/C++ and Qt-based software application designed to act as an intermediary between clients and servers over the HTTP protocol. It facilitates the forwarding of HTTP requests and responses, enabling users to access web content securely and anonymously. This software application acts as a reliable gateway; proxydock ensures communication between clients and servers while maintaining the utmost privacy and confidentiality for end-users, also providing a GUI for easier use.

## Features
- Proxy Server Configuration
- Request Routing 
- Caching (based on connection and file access time)
- Protocol Support (HTTP/1.0, HTTP/1.1, HTTP/2)
- Content Filtering
- Traffic Inspection and Manipulation

## Installation

Before installing, ensure you have the following dependencies:

- [GCC](https://gcc.gnu.org/) (GNU Compiler Collection)
- [Make](https://www.gnu.org/software/make/) (GNU Make)

### Clone the Repository

```bash
git clone https://github.com/Sergiuas/HTTP-Proxy.git
cd HTTP-Proxy/src/
```

### Build and Run
```bash
make
make run
```
Build and run the client side app using a qt and c++ ide such as Qt Creator.


### Good sites for testing
http://www.testingmcafeesites.com/index.html (and the links you find here)
http://testphp.vulnweb.com/login.php
http://httpforever.com/
http://http-textarea.badssl.com/

### Project flow

*Client interface.*
![Screenshot 1](./screenshots/q3.jpeg)


*Blacklist initialisation.*
![Screenshot 2](./screenshots/q1.png)


*Log.*
![Screenshot 3](./screenshots/q2.png)



## Arhitecture

```mermaid
sequenceDiagram
Client->>Proxy: Send HTTP request
Proxy->>HTTP Server: 
Note over Proxy: Read HTTP header
Note over Proxy: Detect Host header
Note over Proxy: Send request to HTTP Server
HTTP Server->> Proxy: Send response
Note over Proxy: Read response
Proxy->>Client: Send response
```

##
_Project completed within the PSO course by Amzuloiu Sergiu and Racianu Gabriel under the guidance of Vaman Adina._
