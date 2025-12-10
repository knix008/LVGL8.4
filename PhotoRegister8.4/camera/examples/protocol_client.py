#!/usr/bin/env python3
"""
Binary Protocol Client Example for Face Recognition Server

This example demonstrates how to communicate with the server using the binary protocol.
"""

import socket
import struct
import time
from enum import IntEnum
from typing import Tuple, Optional

# Protocol Constants
PROTOCOL_MAGIC = 0x46524543  # "FREC"
PROTOCOL_VERSION = 1
MAX_PAYLOAD_SIZE = 1024 * 1024  # 1MB
HEADER_SIZE = 10

class MessageType(IntEnum):
    """Message type enumeration"""
    # Request messages
    REQ_CAMERA_ON = 0x0001
    REQ_CAMERA_OFF = 0x0002
    REQ_CAPTURE = 0x0003
    REQ_TRAIN = 0x0004
    REQ_STATUS = 0x0005
    REQ_STREAM_START = 0x0006
    REQ_STREAM_STOP = 0x0007
    REQ_DELETE_PERSON = 0x0008
    REQ_LIST_PERSONS = 0x0009
    REQ_GET_SETTINGS = 0x000A
    REQ_SET_SETTINGS = 0x000B
    
    # Response messages
    RESP_SUCCESS = 0x1001
    RESP_ERROR = 0x1002
    RESP_STATUS = 0x1003
    RESP_PERSON_LIST = 0x1004
    RESP_SETTINGS = 0x1005
    
    # Stream messages
    STREAM_FACE_DETECTED = 0x2001
    STREAM_NO_FACE = 0x2002
    STREAM_MULTIPLE_FACES = 0x2003
    
    # Event messages
    EVENT_TRAINING_STARTED = 0x3001
    EVENT_TRAINING_PROGRESS = 0x3002
    EVENT_TRAINING_COMPLETED = 0x3003
    EVENT_TRAINING_FAILED = 0x3004
    EVENT_CAMERA_ERROR = 0x3005

class ProtocolMessage:
    """Binary protocol message"""
    
    def __init__(self, msg_type: MessageType, payload: bytes = b''):
        self.msg_type = msg_type
        self.payload = payload
    
    def serialize(self) -> bytes:
        """Serialize message to bytes"""
        # Pack header: magic (4), type (2), length (4)
        header = struct.pack('!IHI', PROTOCOL_MAGIC, self.msg_type, len(self.payload))
        return header + self.payload
    
    @staticmethod
    def deserialize(data: bytes) -> 'ProtocolMessage':
        """Deserialize message from bytes"""
        if len(data) < HEADER_SIZE:
            raise ValueError("Message too short")
        
        # Unpack header
        magic, msg_type, length = struct.unpack('!IHI', data[:HEADER_SIZE])
        
        if magic != PROTOCOL_MAGIC:
            raise ValueError(f"Invalid magic number: 0x{magic:08X}")
        
        if length > MAX_PAYLOAD_SIZE:
            raise ValueError(f"Payload too large: {length}")
        
        # Extract payload
        payload = data[HEADER_SIZE:HEADER_SIZE + length]
        
        if len(payload) != length:
            raise ValueError("Incomplete payload")
        
        return ProtocolMessage(MessageType(msg_type), payload)
    
    def write_string(self, s: str) -> bytes:
        """Encode string for payload"""
        encoded = s.encode('utf-8')
        return struct.pack('!I', len(encoded)) + encoded
    
    @staticmethod
    def read_string(data: bytes, offset: int) -> Tuple[str, int]:
        """Decode string from payload"""
        length = struct.unpack('!I', data[offset:offset+4])[0]
        offset += 4
        string = data[offset:offset+length].decode('utf-8')
        offset += length
        return string, offset

class ProtocolClient:
    """Client for binary protocol communication"""
    
    def __init__(self, socket_path: str = "/tmp/face_recognition.sock"):
        self.socket_path = socket_path
        self.sock: Optional[socket.socket] = None
    
    def connect(self) -> bool:
        """Connect to server"""
        try:
            self.sock = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
            self.sock.connect(self.socket_path)
            print(f"Connected to {self.socket_path}")
            return True
        except Exception as e:
            print(f"Connection failed: {e}")
            return False
    
    def disconnect(self):
        """Disconnect from server"""
        if self.sock:
            self.sock.close()
            self.sock = None
    
    def send_message(self, msg: ProtocolMessage) -> bool:
        """Send message to server"""
        if not self.sock:
            print("Not connected")
            return False
        
        try:
            data = msg.serialize()
            self.sock.sendall(data)
            print(f"Sent {msg.msg_type.name} ({len(data)} bytes)")
            return True
        except Exception as e:
            print(f"Send failed: {e}")
            return False
    
    def receive_message(self) -> ProtocolMessage:
        """Receive message from server"""
        if not self.sock:
            raise RuntimeError("Not connected")
        
        # Read header
        header_data = self._recv_exact(HEADER_SIZE)
        
        # Parse header to get payload length
        magic, msg_type, payload_length = struct.unpack('!IHI', header_data)
        
        # Read payload
        payload = b''
        if payload_length > 0:
            payload = self._recv_exact(payload_length)
        
        full_data = header_data + payload
        msg = ProtocolMessage.deserialize(full_data)
        print(f"Received {msg.msg_type.name} ({len(full_data)} bytes)")
        
        return msg
    
    def _recv_exact(self, n: int) -> bytes:
        """Receive exactly n bytes"""
        data = b''
        while len(data) < n:
            chunk = self.sock.recv(n - len(data))
            if not chunk:
                raise RuntimeError("Connection closed")
            data += chunk
        return data

# High-level API functions

def camera_on(client: ProtocolClient):
    """Turn camera on"""
    msg = ProtocolMessage(MessageType.REQ_CAMERA_ON)
    client.send_message(msg)
    response = client.receive_message()
    
    if response.msg_type == MessageType.RESP_SUCCESS:
        message, _ = ProtocolMessage.read_string(response.payload, 0)
        print(f"✓ Success: {message}")
    elif response.msg_type == MessageType.RESP_ERROR:
        error_code = struct.unpack('!I', response.payload[0:4])[0]
        error_msg, _ = ProtocolMessage.read_string(response.payload, 4)
        print(f"✗ Error {error_code}: {error_msg}")

def camera_off(client: ProtocolClient):
    """Turn camera off"""
    msg = ProtocolMessage(MessageType.REQ_CAMERA_OFF)
    client.send_message(msg)
    response = client.receive_message()
    
    if response.msg_type == MessageType.RESP_SUCCESS:
        message, _ = ProtocolMessage.read_string(response.payload, 0)
        print(f"✓ Success: {message}")

def capture_person(client: ProtocolClient, name: str, person_id: int):
    """Capture person"""
    # Build payload: name (string) + id (uint32)
    msg = ProtocolMessage(MessageType.REQ_CAPTURE)
    payload = msg.write_string(name) + struct.pack('!I', person_id)
    msg.payload = payload
    
    client.send_message(msg)
    response = client.receive_message()
    
    if response.msg_type == MessageType.RESP_SUCCESS:
        message, _ = ProtocolMessage.read_string(response.payload, 0)
        print(f"✓ Success: {message}")
    elif response.msg_type == MessageType.RESP_ERROR:
        error_code = struct.unpack('!I', response.payload[0:4])[0]
        error_msg, _ = ProtocolMessage.read_string(response.payload, 4)
        print(f"✗ Error {error_code}: {error_msg}")

def get_status(client: ProtocolClient):
    """Get server status"""
    msg = ProtocolMessage(MessageType.REQ_STATUS)
    client.send_message(msg)
    response = client.receive_message()
    
    if response.msg_type == MessageType.RESP_STATUS:
        offset = 0
        camera_running = response.payload[offset] != 0; offset += 1
        recognition_enabled = response.payload[offset] != 0; offset += 1
        training_in_progress = response.payload[offset] != 0; offset += 1
        people_count = struct.unpack('!I', response.payload[offset:offset+4])[0]; offset += 4
        total_faces = struct.unpack('!I', response.payload[offset:offset+4])[0]; offset += 4
        fps = struct.unpack('!f', response.payload[offset:offset+4])[0]; offset += 4
        
        print("\n=== Server Status ===")
        print(f"Camera Running: {camera_running}")
        print(f"Recognition Enabled: {recognition_enabled}")
        print(f"Training In Progress: {training_in_progress}")
        print(f"People Count: {people_count}")
        print(f"Total Faces: {total_faces}")
        print(f"FPS: {fps:.2f}")

def list_persons(client: ProtocolClient):
    """List all registered persons"""
    msg = ProtocolMessage(MessageType.REQ_LIST_PERSONS)
    client.send_message(msg)
    response = client.receive_message()
    
    if response.msg_type == MessageType.RESP_PERSON_LIST:
        offset = 0
        count = struct.unpack('!I', response.payload[offset:offset+4])[0]; offset += 4
        
        print(f"\n=== Registered Persons ({count}) ===")
        for i in range(count):
            name, offset = ProtocolMessage.read_string(response.payload, offset)
            person_id = struct.unpack('!I', response.payload[offset:offset+4])[0]; offset += 4
            image_count = struct.unpack('!I', response.payload[offset:offset+4])[0]; offset += 4
            ts_high = struct.unpack('!I', response.payload[offset:offset+4])[0]; offset += 4
            ts_low = struct.unpack('!I', response.payload[offset:offset+4])[0]; offset += 4
            timestamp = (ts_high << 32) | ts_low
            
            print(f"  {i+1}. {name} (ID: {person_id}, Images: {image_count}, Created: {timestamp})")

def train_model(client: ProtocolClient):
    """Train recognition model"""
    msg = ProtocolMessage(MessageType.REQ_TRAIN)
    client.send_message(msg)
    response = client.receive_message()
    
    if response.msg_type == MessageType.RESP_SUCCESS:
        message, _ = ProtocolMessage.read_string(response.payload, 0)
        print(f"✓ Training started: {message}")
    elif response.msg_type == MessageType.RESP_ERROR:
        error_code = struct.unpack('!I', response.payload[0:4])[0]
        error_msg, _ = ProtocolMessage.read_string(response.payload, 4)
        print(f"✗ Error {error_code}: {error_msg}")

def stream_recognition(client: ProtocolClient, duration: int = 10):
    """Stream recognition results"""
    msg = ProtocolMessage(MessageType.REQ_STREAM_START)
    client.send_message(msg)
    
    initial = client.receive_message()
    if initial.msg_type == MessageType.RESP_SUCCESS:
        print("\n=== Recognition Stream (press Ctrl+C to stop) ===")
        
        start_time = time.time()
        try:
            while time.time() - start_time < duration:
                stream_msg = client.receive_message()
                
                if stream_msg.msg_type == MessageType.STREAM_FACE_DETECTED:
                    offset = 0
                    name, offset = ProtocolMessage.read_string(stream_msg.payload, offset)
                    confidence = struct.unpack('!f', stream_msg.payload[offset:offset+4])[0]; offset += 4
                    ts_high = struct.unpack('!I', stream_msg.payload[offset:offset+4])[0]; offset += 4
                    ts_low = struct.unpack('!I', stream_msg.payload[offset:offset+4])[0]; offset += 4
                    timestamp = (ts_high << 32) | ts_low
                    
                    print(f"  Face: {name} ({confidence*100:.1f}% confidence) @ {timestamp}")
                    
                elif stream_msg.msg_type == MessageType.STREAM_NO_FACE:
                    offset = 0
                    ts_high = struct.unpack('!I', stream_msg.payload[offset:offset+4])[0]; offset += 4
                    ts_low = struct.unpack('!I', stream_msg.payload[offset:offset+4])[0]; offset += 4
                    timestamp = (ts_high << 32) | ts_low
                    print(f"  No face @ {timestamp}")
                    
        except KeyboardInterrupt:
            print("\nStream stopped by user")

def main():
    """Main example function"""
    print("=== Binary Protocol Python Client ===\n")
    
    client = ProtocolClient()
    
    if not client.connect():
        return
    
    try:
        # Example workflow
        print("\n1. Getting status...")
        get_status(client)
        
        print("\n2. Turning camera on...")
        camera_on(client)
        
        print("\n3. Capturing person 'Alice' with ID 1...")
        time.sleep(1)
        capture_person(client, "Alice", 1)
        
        print("\n4. Listing persons...")
        list_persons(client)
        
        print("\n5. Training model...")
        train_model(client)
        
        print("\n6. Getting updated status...")
        time.sleep(2)
        get_status(client)
        
        # Uncomment to test streaming
        # print("\n7. Streaming recognition (10 seconds)...")
        # stream_recognition(client, 10)
        
    finally:
        client.disconnect()
        print("\nDisconnected")

if __name__ == "__main__":
    main()
