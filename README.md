# Lightweight Key-Value Database

A lightweight, high-performance, in-memory key-value store designed from scratch for fast access, persistence, and concurrency control. Built with a focus on performance and scalability.

---

## Key Features

- **Efficient Key Management** using a custom Trie data structure.
- **Persistence Support** through trie serialization to disk.
- **Thread-Safe Operations** with mutex-based concurrency control.
- **Core Operations**: Insert, Search, Update, Delete.
- **Modular Design** for extensibility and future scalability.


## Prerequisites:
1) Linux Environment  (used linux specific "netinet/in.h" library )
2) C++ 20

## Command to connect to the server : telnet 127.0.0.1 8080