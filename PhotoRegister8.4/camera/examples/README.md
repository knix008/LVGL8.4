# Protocol Examples

This directory contains example code demonstrating how to use the binary communication protocol for the Face Recognition server.

## Overview

The binary protocol provides a type-safe, efficient way to communicate with the server. It supports:

- **Request/Response** - Simple command execution
- **Streaming** - Continuous data feeds (face recognition results)
- **Events** - Asynchronous notifications (training progress)

## Files

### protocol_example.cpp

C++ example demonstrating all protocol features:

```bash
# Build the example
g++ -std=c++17 -I../include protocol_example.cpp ../src/protocol.cpp -o protocol_example

# Run specific example
./protocol_example camera    # Camera control
./protocol_example capture   # Capture person
./protocol_example status    # Get status
./protocol_example list      # List persons
./protocol_example train     # Train model
./protocol_example stream    # Recognition streaming
./protocol_example settings  # Update settings

# Run all examples
./protocol_example
```

### protocol_client.py

Python example with the same functionality:

```bash
# Make executable (if not already)
chmod +x protocol_client.py

# Run the example
./protocol_client.py
```

## Protocol Features Demonstrated

### 1. Camera Control

```cpp
// C++ Example
CameraControlMessage camera_on(true);
client.send_message(camera_on);
Message response = client.receive_message();
```

```python
# Python Example
msg = ProtocolMessage(MessageType.REQ_CAMERA_ON)
client.send_message(msg)
response = client.receive_message()
```

### 2. Person Capture

```cpp
// C++ Example
CaptureMessage capture("Alice", 1);
client.send_message(capture);
Message response = client.receive_message();
```

```python
# Python Example
capture_person(client, "Alice", 1)
```

### 3. Status Query

```cpp
// C++ Example
StatusRequestMessage status_req;
client.send_message(status_req);
Message response = client.receive_message();

if (response.header.get_type() == MessageType::RESP_STATUS) {
    StatusResponse status = StatusResponse::from_message(response);
    std::cout << "People: " << status.people_count << std::endl;
}
```

```python
# Python Example
get_status(client)
```

### 4. List Persons

```cpp
// C++ Example
ListPersonsMessage list_req;
client.send_message(list_req);
Message response = client.receive_message();

PersonListResponse person_list = PersonListResponse::from_message(response);
for (const auto& person : person_list.persons) {
    std::cout << person.name << std::endl;
}
```

### 5. Model Training

```cpp
// C++ Example
TrainMessage train;
client.send_message(train);
Message response = client.receive_message();
```

### 6. Recognition Streaming

```cpp
// C++ Example
StreamControlMessage stream_start(true);
client.send_message(stream_start);

// Receive continuous updates
while (streaming) {
    Message msg = client.receive_message();
    if (msg.header.get_type() == MessageType::STREAM_FACE_DETECTED) {
        FaceDetectionMessage face = FaceDetectionMessage::from_message(msg);
        std::cout << "Face: " << face.person_name 
                  << " (" << face.confidence << ")" << std::endl;
    }
}
```

## Message Types

### Request Messages (Client → Server)

| Message | Code | Description |
|---------|------|-------------|
| REQ_CAMERA_ON | 0x0001 | Start camera |
| REQ_CAMERA_OFF | 0x0002 | Stop camera |
| REQ_CAPTURE | 0x0003 | Capture and register person |
| REQ_TRAIN | 0x0004 | Train recognition model |
| REQ_STATUS | 0x0005 | Query status |
| REQ_STREAM_START | 0x0006 | Begin streaming |
| REQ_STREAM_STOP | 0x0007 | Stop streaming |
| REQ_DELETE_PERSON | 0x0008 | Delete person |
| REQ_LIST_PERSONS | 0x0009 | List all persons |
| REQ_SET_SETTINGS | 0x000B | Update settings |

### Response Messages (Server → Client)

| Message | Code | Description |
|---------|------|-------------|
| RESP_SUCCESS | 0x1001 | Operation succeeded |
| RESP_ERROR | 0x1002 | Operation failed |
| RESP_STATUS | 0x1003 | Status information |
| RESP_PERSON_LIST | 0x1004 | Person list |
| RESP_SETTINGS | 0x1005 | Current settings |

### Stream Messages (Server → Client)

| Message | Code | Description |
|---------|------|-------------|
| STREAM_FACE_DETECTED | 0x2001 | Face recognized |
| STREAM_NO_FACE | 0x2002 | No face detected |

### Event Messages (Server → Client)

| Message | Code | Description |
|---------|------|-------------|
| EVENT_TRAINING_PROGRESS | 0x3002 | Training progress update |
| EVENT_TRAINING_COMPLETED | 0x3003 | Training completed |

## Error Codes

| Code | Name | Description |
|------|------|-------------|
| 0 | SUCCESS | No error |
| 1 | UNKNOWN_ERROR | Unspecified error |
| 10 | CAMERA_NOT_RUNNING | Camera must be started |
| 11 | CAMERA_ALREADY_RUNNING | Camera already active |
| 20 | CAPTURE_FAILED | Failed to capture frame |
| 21 | NO_FACE_DETECTED | No face in frame |
| 30 | TRAINING_IN_PROGRESS | Training already running |
| 40 | PERSON_NOT_FOUND | Person doesn't exist |
| 50 | INVALID_PARAMETERS | Invalid request parameters |

## Protocol Structure

Every message has a 10-byte header:

```
+--------+--------+----------+----------+
| Magic  | MsgType| Length   | Payload  |
| 4 bytes| 2 bytes| 4 bytes  | N bytes  |
+--------+--------+----------+----------+
```

- **Magic**: 0x46524543 ("FREC")
- **MsgType**: Message type code
- **Length**: Payload size in bytes
- **Payload**: Message-specific data

All multi-byte values use network byte order (big-endian).

## Integration Tips

### C++ Integration

1. Include the protocol header:
   ```cpp
   #include "protocol.h"
   ```

2. Create and send messages:
   ```cpp
   CaptureMessage msg("Alice", 1);
   std::vector<uint8_t> data = msg.serialize();
   write(socket_fd, data.data(), data.size());
   ```

3. Receive and parse:
   ```cpp
   std::vector<uint8_t> buffer(1024);
   read(socket_fd, buffer.data(), buffer.size());
   Message msg = Message::deserialize(buffer);
   ```

### Python Integration

1. Use the ProtocolClient class for easy communication
2. All serialization/deserialization is handled automatically
3. Network byte order conversion is built-in

### Other Languages

The protocol is language-agnostic. Key requirements:

1. Support for binary data (byte arrays)
2. Network byte order (big-endian) conversion
3. Socket communication (Unix domain or TCP)

## Advantages Over Text Protocol

| Feature | Binary Protocol | Text Protocol |
|---------|----------------|---------------|
| Type Safety | ✓ Strong typing | ✗ String parsing |
| Performance | ✓ Fast serialization | ✗ Slower parsing |
| Validation | ✓ Compile-time checks | ✗ Runtime parsing |
| Extensibility | ✓ Version support | ~ Limited |
| Size Efficiency | ✓ Compact binary | ✗ Verbose text |
| Error Detection | ✓ CRC/checksum ready | ~ Manual |

## Further Documentation

- [PROTOCOL.md](../PROTOCOL.md) - Complete protocol specification
- [SOCKET_INTERFACE.md](../SOCKET_INTERFACE.md) - Legacy text protocol
- [ARCHITECTURE.md](../ARCHITECTURE.md) - System architecture

## Requirements

### C++ Example

- C++17 compiler
- POSIX socket support (Unix/Linux)
- Protocol library (protocol.h/protocol.cpp)

### Python Example

- Python 3.6+
- No external dependencies (uses standard library)
- Unix domain socket support

## Testing

Before running examples:

1. **Start the server:**
   ```bash
   cd ..
   make
   ./gtk_webcam
   ```

2. **Verify socket exists:**
   ```bash
   ls -la /tmp/face_recognition.sock
   ```

3. **Run examples:**
   ```bash
   ./protocol_example
   ./protocol_client.py
   ```

## Troubleshooting

**"Connection failed"**
- Ensure server is running
- Check socket path: `/tmp/face_recognition.sock`
- Verify socket permissions

**"Invalid magic number"**
- Server may be using text protocol
- Check protocol version compatibility

**"Message too short"**
- Network transmission error
- Check socket buffer sizes

## License

Same as the main project.
