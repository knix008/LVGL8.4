#include "protocol.h"
#include <iostream>
#include <iomanip>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <arpa/inet.h>

using namespace Protocol;

/**
 * @brief Example utility class for binary protocol communication
 */
class ProtocolClient {
public:
    ProtocolClient(const std::string& socket_path)
        : socket_path_(socket_path), socket_fd_(-1) {}

    ~ProtocolClient() {
        disconnect();
    }

    bool connect() {
        socket_fd_ = socket(AF_UNIX, SOCK_STREAM, 0);
        if (socket_fd_ < 0) {
            std::cerr << "Failed to create socket" << std::endl;
            return false;
        }

        struct sockaddr_un addr;
        memset(&addr, 0, sizeof(addr));
        addr.sun_family = AF_UNIX;
        strncpy(addr.sun_path, socket_path_.c_str(), sizeof(addr.sun_path) - 1);

        if (::connect(socket_fd_, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
            std::cerr << "Failed to connect to server" << std::endl;
            close(socket_fd_);
            socket_fd_ = -1;
            return false;
        }

        std::cout << "Connected to server: " << socket_path_ << std::endl;
        return true;
    }

    void disconnect() {
        if (socket_fd_ >= 0) {
            close(socket_fd_);
            socket_fd_ = -1;
        }
    }

    bool send_message(const Message& msg) {
        if (socket_fd_ < 0) {
            std::cerr << "Not connected" << std::endl;
            return false;
        }

        std::vector<uint8_t> data = msg.serialize();
        
        std::cout << "Sending " << get_message_type_name(msg.header.get_type()) 
                  << " (" << data.size() << " bytes)" << std::endl;

        ssize_t sent = write(socket_fd_, data.data(), data.size());
        if (sent < 0 || static_cast<size_t>(sent) != data.size()) {
            std::cerr << "Failed to send message" << std::endl;
            return false;
        }

        return true;
    }

    Message receive_message() {
        if (socket_fd_ < 0) {
            throw std::runtime_error("Not connected");
        }

        // Read header first
        uint8_t header_buf[HEADER_SIZE];
        ssize_t bytes_read = read(socket_fd_, header_buf, HEADER_SIZE);
        
        if (bytes_read != HEADER_SIZE) {
            throw std::runtime_error("Failed to read message header");
        }

        // Parse header to get payload length
        std::vector<uint8_t> header_data(header_buf, header_buf + HEADER_SIZE);
        
        // Extract length (bytes 6-9 in network order)
        uint32_t length_net;
        std::memcpy(&length_net, header_buf + 6, sizeof(length_net));
        uint32_t payload_length = ntohl(length_net);

        // Read payload
        std::vector<uint8_t> full_data(header_data);
        if (payload_length > 0) {
            std::vector<uint8_t> payload_buf(payload_length);
            bytes_read = read(socket_fd_, payload_buf.data(), payload_length);
            
            if (bytes_read != static_cast<ssize_t>(payload_length)) {
                throw std::runtime_error("Failed to read message payload");
            }
            
            full_data.insert(full_data.end(), payload_buf.begin(), payload_buf.end());
        }

        Message msg = Message::deserialize(full_data);
        std::cout << "Received " << get_message_type_name(msg.header.get_type()) 
                  << " (" << full_data.size() << " bytes)" << std::endl;

        return msg;
    }

private:
    std::string socket_path_;
    int socket_fd_;
};

// Example usage functions

void example_camera_control() {
    std::cout << "\n=== Example: Camera Control ===" << std::endl;
    
    ProtocolClient client("/tmp/face_recognition.sock");
    if (!client.connect()) return;

    // Turn camera on
    CameraControlMessage camera_on(true);
    client.send_message(camera_on);
    
    Message response = client.receive_message();
    if (response.header.get_type() == MessageType::RESP_SUCCESS) {
        SuccessResponse success = SuccessResponse::from_message(response);
        std::cout << "Success: " << success.message << std::endl;
    } else if (response.header.get_type() == MessageType::RESP_ERROR) {
        ErrorResponse error = ErrorResponse::from_message(response);
        std::cout << "Error " << error.error_code << ": " 
                  << error.error_message << std::endl;
    }
}

void example_capture_person() {
    std::cout << "\n=== Example: Capture Person ===" << std::endl;
    
    ProtocolClient client("/tmp/face_recognition.sock");
    if (!client.connect()) return;

    // Capture person "Alice" with ID 1
    CaptureMessage capture("Alice", 1);
    client.send_message(capture);
    
    Message response = client.receive_message();
    if (response.header.get_type() == MessageType::RESP_SUCCESS) {
        SuccessResponse success = SuccessResponse::from_message(response);
        std::cout << "Success: " << success.message << std::endl;
    } else if (response.header.get_type() == MessageType::RESP_ERROR) {
        ErrorResponse error = ErrorResponse::from_message(response);
        std::cout << "Error " << error.error_code << ": " 
                  << error.error_message << std::endl;
    }
}

void example_get_status() {
    std::cout << "\n=== Example: Get Status ===" << std::endl;
    
    ProtocolClient client("/tmp/face_recognition.sock");
    if (!client.connect()) return;

    // Request status
    StatusRequestMessage status_req;
    client.send_message(status_req);
    
    Message response = client.receive_message();
    if (response.header.get_type() == MessageType::RESP_STATUS) {
        StatusResponse status = StatusResponse::from_message(response);
        std::cout << "Status:" << std::endl;
        std::cout << "  Camera Running: " << (status.camera_running ? "Yes" : "No") << std::endl;
        std::cout << "  Recognition Enabled: " << (status.recognition_enabled ? "Yes" : "No") << std::endl;
        std::cout << "  Training In Progress: " << (status.training_in_progress ? "Yes" : "No") << std::endl;
        std::cout << "  People Count: " << status.people_count << std::endl;
        std::cout << "  Total Faces: " << status.total_faces << std::endl;
        std::cout << "  FPS: " << status.fps << std::endl;
    }
}

void example_list_persons() {
    std::cout << "\n=== Example: List Persons ===" << std::endl;
    
    ProtocolClient client("/tmp/face_recognition.sock");
    if (!client.connect()) return;

    // Request person list
    ListPersonsMessage list_req;
    client.send_message(list_req);
    
    Message response = client.receive_message();
    if (response.header.get_type() == MessageType::RESP_PERSON_LIST) {
        PersonListResponse person_list = PersonListResponse::from_message(response);
        std::cout << "Registered Persons (" << person_list.persons.size() << "):" << std::endl;
        
        for (const auto& person : person_list.persons) {
            std::cout << "  - " << person.name 
                      << " (ID: " << person.id 
                      << ", Images: " << person.image_count 
                      << ", Created: " << person.created_timestamp << ")" 
                      << std::endl;
        }
    }
}

void example_train_model() {
    std::cout << "\n=== Example: Train Model ===" << std::endl;
    
    ProtocolClient client("/tmp/face_recognition.sock");
    if (!client.connect()) return;

    // Start training
    TrainMessage train;
    client.send_message(train);
    
    Message response = client.receive_message();
    if (response.header.get_type() == MessageType::RESP_SUCCESS) {
        SuccessResponse success = SuccessResponse::from_message(response);
        std::cout << "Training started: " << success.message << std::endl;
    } else if (response.header.get_type() == MessageType::RESP_ERROR) {
        ErrorResponse error = ErrorResponse::from_message(response);
        std::cout << "Error " << error.error_code << ": " 
                  << error.error_message << std::endl;
    }
}

void example_streaming() {
    std::cout << "\n=== Example: Recognition Streaming ===" << std::endl;
    
    ProtocolClient client("/tmp/face_recognition.sock");
    if (!client.connect()) return;

    // Start streaming
    StreamControlMessage stream_start(true);
    client.send_message(stream_start);
    
    Message initial_response = client.receive_message();
    if (initial_response.header.get_type() == MessageType::RESP_SUCCESS) {
        std::cout << "Stream started, receiving updates..." << std::endl;
        
        // Receive stream messages (press Ctrl+C to stop)
        for (int i = 0; i < 10; ++i) {  // Receive 10 updates for demo
            try {
                Message stream_msg = client.receive_message();
                
                if (stream_msg.header.get_type() == MessageType::STREAM_FACE_DETECTED) {
                    FaceDetectionMessage face = FaceDetectionMessage::from_message(stream_msg);
                    std::cout << "  Face detected: " << face.person_name 
                              << " (confidence: " << std::fixed << std::setprecision(2) 
                              << (face.confidence * 100) << "%)" << std::endl;
                              
                } else if (stream_msg.header.get_type() == MessageType::STREAM_NO_FACE) {
                    NoFaceMessage no_face = NoFaceMessage::from_message(stream_msg);
                    std::cout << "  No face detected at " << no_face.timestamp_ms << std::endl;
                }
                
            } catch (const std::exception& e) {
                std::cerr << "Stream error: " << e.what() << std::endl;
                break;
            }
        }
    }
}

void example_update_settings() {
    std::cout << "\n=== Example: Update Settings ===" << std::endl;
    
    ProtocolClient client("/tmp/face_recognition.sock");
    if (!client.connect()) return;

    // Update settings: threshold=0.75, interval=100ms, auto_train=true
    SettingsMessage settings(0.75f, 100, true);
    client.send_message(settings);
    
    Message response = client.receive_message();
    if (response.header.get_type() == MessageType::RESP_SUCCESS) {
        SuccessResponse success = SuccessResponse::from_message(response);
        std::cout << "Settings updated: " << success.message << std::endl;
    }
}

int main(int argc, char* argv[]) {
    std::cout << "=== Binary Protocol Examples ===" << std::endl;
    std::cout << "Protocol Version: " << PROTOCOL_VERSION << std::endl;
    std::cout << "Protocol Magic: 0x" << std::hex << PROTOCOL_MAGIC << std::dec << std::endl;

    if (argc > 1) {
        std::string example = argv[1];
        
        if (example == "camera") {
            example_camera_control();
        } else if (example == "capture") {
            example_capture_person();
        } else if (example == "status") {
            example_get_status();
        } else if (example == "list") {
            example_list_persons();
        } else if (example == "train") {
            example_train_model();
        } else if (example == "stream") {
            example_streaming();
        } else if (example == "settings") {
            example_update_settings();
        } else {
            std::cout << "\nUsage: " << argv[0] << " <example>" << std::endl;
            std::cout << "Examples: camera, capture, status, list, train, stream, settings" << std::endl;
        }
    } else {
        std::cout << "\nRunning all examples..." << std::endl;
        
        // Run examples (note: server must be running)
        example_camera_control();
        example_get_status();
        example_capture_person();
        example_list_persons();
        example_train_model();
        example_update_settings();
        // example_streaming();  // Comment out to avoid blocking
    }

    return 0;
}
