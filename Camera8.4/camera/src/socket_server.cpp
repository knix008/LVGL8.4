#include "socket_server.h"
#include "protocol.h"
#include "logger.h"
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <cstring>
#include <cerrno>
#include <filesystem>
#include <sstream>
#include <algorithm>
#include <arpa/inet.h>
#include <vector>

SocketServer::SocketServer(const std::string& socket_path)
    : socket_path(socket_path), server_socket(-1), running(false) {}

SocketServer::~SocketServer() {
    stop();
    // Detach thread if it's still running to avoid terminate() exception
    if (server_thread && server_thread->joinable()) {
        server_thread->detach();
    }
}

bool SocketServer::start() {
    if (running) {
        LOG_WARN("Socket server already running");
        return false;
    }

    // Remove existing socket file if it exists
    if (std::filesystem::exists(socket_path)) {
        try {
            std::filesystem::remove(socket_path);
            LOG_INFO("Removed existing socket file: " << socket_path);
        } catch (const std::exception& e) {
            LOG_ERROR("Failed to remove socket file: " << e.what());
            return false;
        }
    }

    // Create Unix domain socket
    server_socket = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_socket < 0) {
        LOG_ERROR("Failed to create socket");
        return false;
    }

    // Set socket address
    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, socket_path.c_str(), sizeof(addr.sun_path) - 1);

    // Bind socket
    if (bind(server_socket, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        LOG_ERROR("Failed to bind socket: " << strerror(errno));
        close(server_socket);
        server_socket = -1;
        return false;
    }

    // Listen for connections
    if (listen(server_socket, 5) < 0) {
        LOG_ERROR("Failed to listen on socket");
        close(server_socket);
        server_socket = -1;
        return false;
    }

    running = true;
    server_thread = std::make_unique<std::thread>(&SocketServer::server_loop, this);
    LOG_INFO("Socket server started on: " << socket_path);
    return true;
}

void SocketServer::stop() {
    if (!running) {
        return;
    }

    running = false;
    LOG_INFO("Socket server stopping...");

    // Close socket first to unblock accept() call
    if (server_socket >= 0) {
        close(server_socket);
        server_socket = -1;
    }

    // Try to wait for server thread to finish with a timeout
    if (server_thread && server_thread->joinable()) {
        // Give the thread 1 second to finish gracefully
        // In most cases, closing the socket will wake up accept() immediately
        for (int i = 0; i < 10; ++i) {
            if (!server_thread->joinable()) {
                break;  // Thread finished
            }
            usleep(100000);  // 100ms
        }

        if (server_thread->joinable()) {
            // Thread is still running, but we can't wait forever
            // The destructor will attempt to join again
            LOG_WARN("Socket server thread did not finish promptly - continuing anyway");
        } else {
            server_thread->join();
        }
    }

    // Clean up socket file immediately
    try {
        if (std::filesystem::exists(socket_path)) {
            std::filesystem::remove(socket_path);
            LOG_INFO("Socket file removed: " << socket_path);
        }
    } catch (const std::exception& e) {
        LOG_WARN("Failed to clean up socket file: " << e.what());
    }

    LOG_INFO("Socket server stopped");
}

void SocketServer::register_command(const std::string& command, CommandCallback callback) {
    command_handlers[command] = callback;
    LOG_INFO("Registered command: " << command);
}

void SocketServer::register_streaming_command(const std::string& command, StreamingCallback callback) {
    streaming_handlers[command] = callback;
    LOG_INFO("Registered streaming command: " << command);
}

void SocketServer::server_loop() {
    LOG_INFO("Socket server loop started");

    while (running) {
        struct sockaddr_un addr;
        socklen_t addr_len = sizeof(addr);

        // Set socket to non-blocking mode with timeout
        // This allows us to check running flag periodically
        int client_fd = accept(server_socket, (struct sockaddr*)&addr, &addr_len);
        if (client_fd < 0) {
            if (running) {
                LOG_ERROR("Accept failed: " << strerror(errno));
            } else {
                LOG_INFO("Accept failed due to shutdown (exiting loop)");
                break;  // Exit loop explicitly when shutting down
            }
            continue;
        }

        // Handle client in separate thread to support concurrent connections
        std::thread([this, client_fd]() {
            handle_client(client_fd);
        }).detach();
    }

    LOG_INFO("Socket server loop ended");
}

void SocketServer::handle_client(int client_fd) {
    try {
        // Read initial data from client
        char buffer[1024] = {0};
        ssize_t bytes_read = read(client_fd, buffer, sizeof(buffer) - 1);

        if (bytes_read < 0) {
            LOG_ERROR("Failed to read from client");
            close(client_fd);
            return;
        }

        if (bytes_read == 0) {
            close(client_fd);
            return;
        }

        // Check if this is a binary protocol message by checking magic number
        if (bytes_read >= 4) {
            uint32_t magic;
            std::memcpy(&magic, buffer, sizeof(magic));
            magic = ntohl(magic);
            
            if (magic == Protocol::PROTOCOL_MAGIC) {
                LOG_INFO("Detected binary protocol message");
                bool keep_open = handle_binary_protocol(client_fd, buffer, bytes_read);
                if (!keep_open) {
                    close(client_fd);
                }
                return;
            }
        }

        // Legacy text protocol handling
        std::string command_str(buffer, bytes_read);
        LOG_INFO("Received command: " << command_str);

        // Parse command name
        std::istringstream iss(command_str);
        std::string command_name;
        std::getline(iss, command_name, ':');

        // Trim whitespace
        command_name.erase(0, command_name.find_first_not_of(" \t\n\r"));
        command_name.erase(command_name.find_last_not_of(" \t\n\r") + 1);

        // Convert to lowercase
        std::transform(command_name.begin(), command_name.end(), command_name.begin(), ::tolower);

        // Check if it's a streaming command
        auto streaming_it = streaming_handlers.find(command_name);
        if (streaming_it != streaming_handlers.end()) {
            LOG_INFO("Handling streaming command: " << command_name);
            streaming_it->second(client_fd);
            close(client_fd);
            return;
        }

        // Regular command handling
        std::string response = execute_command(command_str);

        // Send response back to client
        if (write(client_fd, response.c_str(), response.length()) < 0) {
            LOG_ERROR("Failed to write response to client");
        }

        close(client_fd);

    } catch (const std::exception& e) {
        LOG_ERROR("Exception handling client: " << e.what());
        close(client_fd);
    }
}

std::string SocketServer::execute_command(const std::string& command_str) {
    // Parse command: "command_name:arg1:arg2:..."
    std::istringstream iss(command_str);
    std::string command;
    std::getline(iss, command, ':');

    // Trim whitespace
    command.erase(0, command.find_first_not_of(" \t\n\r"));
    command.erase(command.find_last_not_of(" \t\n\r") + 1);

    // Convert to lowercase for comparison
    std::transform(command.begin(), command.end(), command.begin(), ::tolower);

    // Look up command handler
    auto it = command_handlers.find(command);
    if (it == command_handlers.end()) {
        LOG_WARN("Unknown command: " << command);
        return "ERROR:Unknown command: " + command;
    }

    try {
        // Get remaining arguments
        std::string args = command_str.substr(command.length());
        if (!args.empty() && args[0] == ':') {
            args = args.substr(1);  // Remove leading colon
        }

        // Execute command handler
        std::string response = it->second(args);
        LOG_INFO("Command executed successfully: " << command);
        return response;

    } catch (const std::exception& e) {
        LOG_ERROR("Error executing command " << command << ": " << e.what());
        return "ERROR:" + std::string(e.what());
    }
}

bool SocketServer::handle_binary_protocol(int client_fd, const char* initial_data, size_t initial_len) {
    try {
        // Read complete message - we may need more data than initial read
        std::vector<uint8_t> buffer(initial_data, initial_data + initial_len);
        
        // Check if we have the complete header
        if (buffer.size() < Protocol::HEADER_SIZE) {
            LOG_ERROR("Incomplete message header");
            Protocol::ErrorResponse error(static_cast<uint32_t>(Protocol::ErrorCode::INVALID_MESSAGE), "Incomplete header");
            send_binary_response(client_fd, error);
            return false;
        }
        
        // Parse header to get payload length
        size_t offset = 4; // Skip magic (already verified)
        uint16_t type_net;
        std::memcpy(&type_net, buffer.data() + offset, sizeof(type_net));
        offset += sizeof(type_net);
        
        uint32_t length_net;
        std::memcpy(&length_net, buffer.data() + offset, sizeof(length_net));
        uint32_t payload_length = ntohl(length_net);
        
        // Read remaining payload if needed
        size_t total_size = Protocol::HEADER_SIZE + payload_length;
        if (buffer.size() < total_size) {
            buffer.resize(total_size);
            size_t remaining = total_size - initial_len;
            ssize_t bytes_read = read(client_fd, buffer.data() + initial_len, remaining);
            if (bytes_read < 0 || static_cast<size_t>(bytes_read) < remaining) {
                LOG_ERROR("Failed to read complete message payload");
                Protocol::ErrorResponse error(static_cast<uint32_t>(Protocol::ErrorCode::INVALID_MESSAGE), "Incomplete payload");
                send_binary_response(client_fd, error);
                return false;
            }
        }
        
        // Deserialize message
        Protocol::Message request = Protocol::Message::deserialize(buffer);
        LOG_INFO("Binary protocol message type: " << Protocol::get_message_type_name(static_cast<Protocol::MessageType>(request.header.type)));
        
        // Handle different message types
        return handle_binary_message(client_fd, request);
        
    } catch (const std::exception& e) {
        LOG_ERROR("Error handling binary protocol: " << e.what());
        Protocol::ErrorResponse error(static_cast<uint32_t>(Protocol::ErrorCode::UNKNOWN_ERROR), e.what());
        send_binary_response(client_fd, error);
        return false;
    }
}

bool SocketServer::handle_binary_message(int client_fd, const Protocol::Message& request) {
    using namespace Protocol;
    
    try {
        switch (static_cast<MessageType>(request.header.type)) {
            case MessageType::REQ_CAMERA_ON: {
                auto cmd = CameraControlMessage::from_message(request);
                std::string result = execute_command(cmd.turn_on ? "camera_on" : "camera_off");
                
                if (result.find("OK:") == 0) {
                    SuccessResponse response(result.substr(3)); // Skip "OK:"
                    send_binary_response(client_fd, response);
                } else if (result.find("ERROR") == 0) {
                    ErrorResponse error(static_cast<uint32_t>(ErrorCode::CAMERA_DEVICE_ERROR), result.substr(6)); // Skip "ERROR:"
                    send_binary_response(client_fd, error);
                } else {
                    SuccessResponse response(result);
                    send_binary_response(client_fd, response);
                }
                return false;
            }
            
            case MessageType::REQ_CAMERA_OFF: {
                std::string result = execute_command("camera_off");
                
                if (result.find("OK:") == 0) {
                    SuccessResponse response(result.substr(3));
                    send_binary_response(client_fd, response);
                } else if (result.find("ERROR") == 0) {
                    ErrorResponse error(static_cast<uint32_t>(ErrorCode::CAMERA_DEVICE_ERROR), result.substr(6));
                    send_binary_response(client_fd, error);
                } else {
                    SuccessResponse response(result);
                    send_binary_response(client_fd, response);
                }
                return false;
            }
            
            case MessageType::REQ_CAPTURE: {
                auto cmd = CaptureMessage::from_message(request);
                // Capture command expects arguments: "initial:id"
                std::string args = cmd.person_initial + ":" + std::to_string(cmd.person_id);
                std::string result = execute_command("capture:" + args);
                
                if (result.find("OK:") == 0) {
                    SuccessResponse response(result.substr(3));
                    send_binary_response(client_fd, response);
                } else if (result.find("ERROR") == 0) {
                    ErrorResponse error(static_cast<uint32_t>(ErrorCode::CAPTURE_FAILED), result.substr(6));
                    send_binary_response(client_fd, error);
                } else {
                    SuccessResponse response(result);
                    send_binary_response(client_fd, response);
                }
                return false;
            }
            
            case MessageType::REQ_TRAIN: {
                std::string result = execute_command("registering");
                
                if (result.find("OK:") == 0) {
                    SuccessResponse response(result.substr(3));
                    send_binary_response(client_fd, response);
                } else if (result.find("ERROR") == 0) {
                    ErrorResponse error(static_cast<uint32_t>(ErrorCode::TRAINING_FAILED), result.substr(6));
                    send_binary_response(client_fd, error);
                } else {
                    SuccessResponse response(result);
                    send_binary_response(client_fd, response);
                }
                return false;
            }
            
            case MessageType::REQ_STATUS: {
                std::string result = execute_command("status");
                
                if (result.find("OK:") == 0) {
                    // Parse status response: "OK:camera_running:false,recognition_enabled:false,..."
                    std::string data = result.substr(3);
                    std::istringstream iss(data);
                    std::string camera_on_str, recognizing_str, training_str, people_str, faces_str;
                    
                    // Parse key:value pairs
                    std::getline(iss, camera_on_str, ',');
                    std::getline(iss, recognizing_str, ',');
                    std::getline(iss, training_str, ',');
                    std::getline(iss, people_str, ',');
                    std::getline(iss, faces_str);
                    
                    // Extract values after colon
                    bool camera_on = (camera_on_str.find("true") != std::string::npos);
                    bool recognizing = (recognizing_str.find("true") != std::string::npos);
                    bool training = (training_str.find("true") != std::string::npos);
                    
                    uint32_t people_count = 0;
                    uint32_t faces_count = 0;
                    size_t pos;
                    if ((pos = people_str.find(':')) != std::string::npos) {
                        try { people_count = std::stoul(people_str.substr(pos + 1)); } catch(...) {}
                    }
                    if ((pos = faces_str.find(':')) != std::string::npos) {
                        try { faces_count = std::stoul(faces_str.substr(pos + 1)); } catch(...) {}
                    }
                    
                    StatusResponse response(camera_on, recognizing, training, people_count, faces_count, 0.0f);
                    send_binary_response(client_fd, response);
                } else if (result.find("ERROR") == 0) {
                    ErrorResponse error(static_cast<uint32_t>(ErrorCode::UNKNOWN_ERROR), result.substr(6));
                    send_binary_response(client_fd, error);
                } else {
                    SuccessResponse response(result);
                    send_binary_response(client_fd, response);
                }
                return false;
            }
            
            case MessageType::REQ_LIST_PERSONS: {
                std::string result = execute_command("list");
                
                if (result.find("OK:") == 0) {
                    // Parse person list: "OK:count,person1,person2,..."
                    std::string data = result.substr(3);
                    std::istringstream iss(data);
                    std::string count_str;
                    std::getline(iss, count_str, ',');
                    
                    std::vector<PersonInfo> persons;
                    std::string person;
                    uint32_t id = 1;
                    while (std::getline(iss, person, ',')) {
                        if (!person.empty()) {
                            PersonInfo info;
                            info.name = person;
                            info.id = id++;
                            info.image_count = 0; // Unknown from text protocol
                            info.created_timestamp = 0; // Unknown from text protocol
                            persons.push_back(info);
                        }
                    }
                    
                    PersonListResponse response(persons);
                    send_binary_response(client_fd, response);
                } else if (result.find("ERROR") == 0) {
                    ErrorResponse error(static_cast<uint32_t>(ErrorCode::DATABASE_ERROR), result.substr(6));
                    send_binary_response(client_fd, error);
                } else {
                    SuccessResponse response(result);
                    send_binary_response(client_fd, response);
                }
                return false;
            }
            
            case MessageType::REQ_STREAM_START: {
                // Streaming command - hand off to streaming handler (don't close socket)
                LOG_INFO("Starting recognition stream");
                auto streaming_it = streaming_handlers.find("stream_recognition");
                if (streaming_it != streaming_handlers.end()) {
                    // Send success response first
                    SuccessResponse response("Stream started");
                    send_binary_response(client_fd, response);
                    // Call streaming handler (it will handle the socket)
                    streaming_it->second(client_fd);
                } else {
                    ErrorResponse error(static_cast<uint32_t>(ErrorCode::UNKNOWN_ERROR), "Streaming not available");
                    send_binary_response(client_fd, error);
                }
                return true; // Keep socket open - streaming handler manages it
            }
            
            case MessageType::REQ_STREAM_STOP: {
                // Client is requesting to stop stream (socket will be closed)
                SuccessResponse response("Stream stop acknowledged");
                send_binary_response(client_fd, response);
                return false;
            }
            
            default:
                LOG_WARN("Unsupported binary message type: " << request.header.type);
                ErrorResponse error(static_cast<uint32_t>(ErrorCode::UNKNOWN_ERROR), "Message type not supported");
                send_binary_response(client_fd, error);
                return false;
        }
        
    } catch (const std::exception& e) {
        LOG_ERROR("Error handling binary message: " << e.what());
        ErrorResponse error(static_cast<uint32_t>(ErrorCode::UNKNOWN_ERROR), e.what());
        send_binary_response(client_fd, error);
        return false;
    }
}

void SocketServer::send_binary_response(int client_fd, const Protocol::Message& response) {
    try {
        std::vector<uint8_t> data = response.serialize();
        
        if (write(client_fd, data.data(), data.size()) < 0) {
            LOG_ERROR("Failed to write binary response to client");
        } else {
            LOG_INFO("Sent binary response, type: " << Protocol::get_message_type_name(static_cast<Protocol::MessageType>(response.header.type)));
        }
        
    } catch (const std::exception& e) {
        LOG_ERROR("Error sending binary response: " << e.what());
    }
}

