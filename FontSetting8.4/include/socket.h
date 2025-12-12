#ifndef SOCKET_H
#define SOCKET_H

#include <stdint.h>

#define MAX_STRING_LEN 256
#define MAX_SOCKET_PATH 108

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Socket client structure for communicating with face recognition server
 *
 * Provides a C interface to send binary protocol messages via TCP or Unix domain
 * socket and receive responses.
 */
typedef struct {
    char socket_path[MAX_SOCKET_PATH];
    char server_ip[64];
    int port;
    int use_tcp;  /* 1 for TCP, 0 for Unix domain socket */
} SocketClient;

/**
 * @brief Response from server
 */
typedef struct {
    int success;                    /* 1 if OK, 0 if ERROR */
    char message[MAX_STRING_LEN];   /* Response message */
} Response;

/**
 * @brief Create socket client for Unix domain socket
 * @param socket_path Path to Unix socket (default: /tmp/face_recognition.sock)
 * @return Pointer to SocketClient or NULL on error
 */
SocketClient* socket_client_create_unix(const char *socket_path);

/**
 * @brief Create socket client for TCP connection
 * @param server_ip Server IP address
 * @param port Server port number
 * @return Pointer to SocketClient or NULL on error
 */
SocketClient* socket_client_create_tcp(const char *server_ip, int port);

/**
 * @brief Destroy socket client and free resources
 * @param client Socket client to destroy
 */
void socket_client_destroy(SocketClient *client);

/**
 * @brief Turn camera on
 * @param client Socket client
 * @param response Response structure to fill
 * @return 0 on success, -1 on error
 */
int socket_client_camera_on(SocketClient *client, Response *response);

/**
 * @brief Turn camera off
 * @param client Socket client
 * @param response Response structure to fill
 * @return 0 on success, -1 on error
 */
int socket_client_camera_off(SocketClient *client, Response *response);

/**
 * @brief Capture person
 * @param client Socket client
 * @param initial Person initial (A-Z)
 * @param id Person ID (1-9999)
 * @param response Response structure to fill
 * @return 0 on success, -1 on error
 */
int socket_client_capture(SocketClient *client, const char *initial, uint64_t id, Response *response);

/**
 * @brief Start training recognition model
 * @param client Socket client
 * @param response Response structure to fill
 * @return 0 on success, -1 on error
 */
int socket_client_train(SocketClient *client, Response *response);

/**
 * @brief Delete person
 * @param client Socket client
 * @param name Person name to delete
 * @param response Response structure to fill
 * @return 0 on success, -1 on error
 */
int socket_client_delete_person(SocketClient *client, const char *name, Response *response);

/**
 * @brief Get server status
 * @param client Socket client
 * @param response Response structure to fill
 * @return 0 on success, -1 on error
 */
int socket_client_status(SocketClient *client, Response *response);

/**
 * @brief List registered persons
 * @param client Socket client
 * @param response Response structure to fill
 * @return 0 on success, -1 on error
 */
int socket_client_list_persons(SocketClient *client, Response *response);

/**
 * @brief Toggle face detection
 * @param client Socket client
 * @param enabled 1 to enable, 0 to disable
 * @param response Response structure to fill
 * @return 0 on success, -1 on error
 */
int socket_client_detect_faces(SocketClient *client, int enabled, Response *response);

/**
 * @brief Enable Face Anti-Spoofing
 * @param client Socket client
 * @param response Response structure to fill
 * @return 0 on success, -1 on error
 */
int socket_client_fas_on(SocketClient *client, Response *response);

/**
 * @brief Disable Face Anti-Spoofing
 * @param client Socket client
 * @param response Response structure to fill
 * @return 0 on success, -1 on error
 */
int socket_client_fas_off(SocketClient *client, Response *response);

/**
 * @brief Set configuration settings
 * @param client Socket client
 * @param max_ratio Maximum face aspect ratio
 * @param max_degree Maximum face degree
 * @param min_size Minimum face size
 * @param det_th Detection threshold
 * @param fas_th FAS threshold
 * @param response Response structure to fill
 * @return 0 on success, -1 on error
 */
int socket_client_set_settings(SocketClient *client, float max_ratio, float max_degree,
                               uint32_t min_size, float det_th, float fas_th, Response *response);

/**
 * @brief Start streaming recognition results
 * @param client Socket client
 * @return Socket file descriptor on success, -1 on error (caller must close socket)
 */
int socket_client_stream_recognition(SocketClient *client);

#ifdef __cplusplus
}
#endif

#endif /* SOCKET_H */
