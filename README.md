# HTTP-Proxy

<p align="center">
  <img src="https://github.com/Sergiuas/HTTP-Proxy/blob/main/proxydock.png?raw=true" alt="alt text" width="400" height="400">
</p>


## Table of Contents

<details>

1. [Version Table](#version-table)
    
2. [Introduction](#introduction)
   2.1 [Project Overview](#project-overview)
   2.2 [Objective](#objective)

4. [Functional Requirements](#functional-requirements)

5. [Non-Functional Requirements](#non-functional-requirements)

6. [Technical Requirements](#technical-requirements)

</details>

## 1. Version Table 

| Date       | Version  | Notes                                                   |
| :----------|:---------|:--------------------------------------------------------|
| 11/7/2023  | 1.0      | - Created repository                                    |
|            |          |  - Added first version of product requirements document |


### 2.1 Project Overview
**proxydock** is a software application designed to act as an intermediary between clients and servers over the HTTP protocol. It facilitates the forwarding of HTTP requests and responses, enabling users to access web content securely and anonymously.

### 2.2 Objective
- Intercept and inspect HTTP traffic for security and monitoring purposes.
- Enable content filtering and access control for users on a network.
- Editing/adding headers
- Possibility of decompressing gzip/base64

##  3.  Functional Requirements


## 4. Non-Functional Requirements


## 5. Architecture

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

## 6. Technical Requirements

