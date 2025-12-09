#ifndef SOCKET_SERVER_H
#define SOCKET_SERVER_H

#include <string>
#include <functional>
#include <thread>
#include <atomic>
#include <memory>
#include <map>
#include <vector>
#include <cstdint>

// Forward declaration
namespace Protocol {
    class Message;
}

/**
 * @brief Unix domain socket server for remote command control
 *
 * Provides a socket interface for controlling the application via commands:
 * - camera_on: Start camera
 * - camera_off: Stop camera
 * - capture: Capture and register new person
 * - registering: Train recognition model
 * - status: Get application status
 */
class SocketServer {
public:
    // Command callback types
    using CommandCallback = std::function<std::string(const std::string&)>;
    // Streaming callback - receives client_fd and is responsible for communication
    using StreamingCallback = std::function<void(int)>;

    SocketServer(const std::string& socket_path = "/tmp/face_recognition.sock");
    ~SocketServer();

    /**
     * @brief Start socket server
     * @return true if server started successfully
     */
    bool start();

    /**
     * @brief Stop socket server
     */
    void stop();

    /**
     * @brief Check if server is running
     */
    bool is_running() const { return running; }

    /**
     * @brief Register a command handler
     * @param command Command name (e.g., "camera_on")
     * @param callback Function to execute when command is received
     *                  Takes command arguments and returns response
     */
    void register_command(const std::string& command, CommandCallback callback);

    /**
     * @brief Register a streaming command handler
     * @param command Command name (e.g., "stream_recognition")
     * @param callback Function to handle streaming (receives client_fd)
     */
    void register_streaming_command(const std::string& command, StreamingCallback callback);

    /**
     * @brief Get server socket path
     */
    const std::string& get_socket_path() const { return socket_path; }

private:
    /**
     * @brief Main server loop (runs in separate thread)
     */
    void server_loop();

    /**
     * @brief Handle incoming client connection
     */
    void handle_client(int client_fd);

    /**
     * @brief Parse and execute command
     * @return Response message to send to client
     */
    std::string execute_command(const std::string& command_str);

    /**
     * @brief Handle binary protocol message
     * @return true if socket should remain open (for streaming), false to close
     */
    bool handle_binary_protocol(int client_fd, const char* initial_data, size_t initial_len);

    /**
     * @brief Handle specific binary message type
     * @return true if socket should remain open (for streaming), false to close
     */
    bool handle_binary_message(int client_fd, const Protocol::Message& request);

    /**
     * @brief Send binary protocol response
     */
    void send_binary_response(int client_fd, const Protocol::Message& response);

    std::string socket_path;
    int server_socket;
    std::atomic<bool> running;
    std::unique_ptr<std::thread> server_thread;
    std::map<std::string, CommandCallback> command_handlers;
    std::map<std::string, StreamingCallback> streaming_handlers;
};

#endif // SOCKET_SERVER_H
