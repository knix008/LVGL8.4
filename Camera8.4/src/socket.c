#include "../include/socket.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>

/* Protocol constants (matching protocol.h) */
#define PROTOCOL_MAGIC 0x46524543  /* "FREC" */
#define MAX_PAYLOAD_SIZE (1024 * 1024)
#define HEADER_SIZE 10
#define MAX_STRING_LEN 256
#define MAX_PERSONS 100

/* Message types */
typedef enum {
    REQ_CAMERA_ON = 0x0001,
    REQ_CAMERA_OFF = 0x0002,
    REQ_CAPTURE = 0x0003,
    REQ_TRAIN = 0x0004,
    REQ_STATUS = 0x0005,
    REQ_STREAM_START = 0x0006,
    REQ_STREAM_STOP = 0x0007,
    REQ_DELETE_PERSON = 0x0008,
    REQ_LIST_PERSONS = 0x0009,
    REQ_DETECT_FACES = 0x000C,
    REQ_FAS_ON = 0x000D,
    REQ_FAS_OFF = 0x000E,
    REQ_SET_SETTINGS = 0x000B,

    RESP_SUCCESS = 0x1001,
    RESP_ERROR = 0x1002,
    RESP_STATUS = 0x1003,
    RESP_PERSON_LIST = 0x1004
} MessageType;

#define MAX_BUFFER_SIZE 4096

/* Simple buffer structure with fixed size */
typedef struct {
    uint8_t data[MAX_BUFFER_SIZE];
    uint32_t size;
    uint32_t capacity;
} Buffer;

/* Person information */
typedef struct {
    char name[MAX_STRING_LEN];
    uint64_t id;
    uint32_t image_count;
    uint64_t created_timestamp;
} PersonInfo;

/* Response data structures */
typedef struct {
    int camera_running;
    int recognition_enabled;
    int training_in_progress;
    uint32_t people_count;
    uint32_t total_faces;
    float fps;
    float max_face_aspect_ratio;
    float max_face_degree;
    uint32_t min_face_size;
    float det_th;
    float fas_th;
    float detection_time_ms;
} StatusData;

typedef struct {
    PersonInfo persons[MAX_PERSONS];
    uint32_t count;
} PersonListData;

/* Buffer helper functions */
static void buffer_init(Buffer *buf) {
    if (!buf) return;
    buf->size = 0;
    buf->capacity = MAX_BUFFER_SIZE;
    memset(buf->data, 0, MAX_BUFFER_SIZE);
}

static int buffer_ensure_capacity(Buffer *buf, uint32_t required) {
    if (!buf) return -1;
    if (buf->capacity == 0) return -1;  /* Invalid buffer */
    if (required > buf->capacity) return -1;  /* Fixed size buffer - overflow */
    if (required > MAX_BUFFER_SIZE) return -1;  /* Sanity check */
    return 0;
}

static int buffer_append(Buffer *buf, const void *data, uint32_t len) {
    if (!buf || !data) return -1;
    if (len == 0) return 0;  /* Nothing to append */
    
    /* Check for overflow in addition */
    if (buf->size > UINT32_MAX - len) return -1;
    
    /* Check capacity before appending */
    if (buffer_ensure_capacity(buf, buf->size + len) < 0) return -1;
    
    /* Ensure we don't write beyond buffer bounds */
    if (buf->size + len > MAX_BUFFER_SIZE) return -1;
    
    memcpy(buf->data + buf->size, data, len);
    buf->size += len;
    return 0;
}

static int buffer_write_uint32(Buffer *buf, uint32_t value) {
    if (!buf) return -1;
    uint32_t net_value = htonl(value);
    return buffer_append(buf, &net_value, sizeof(net_value));
}

static int buffer_write_uint64(Buffer *buf, uint64_t value) {
    if (!buf) return -1;
    uint32_t high = htonl((uint32_t)(value >> 32));
    uint32_t low = htonl((uint32_t)(value & 0xFFFFFFFF));
    if (buffer_append(buf, &high, sizeof(high)) < 0) return -1;
    return buffer_append(buf, &low, sizeof(low));
}

static int buffer_write_string(Buffer *buf, const char *str) {
    if (!buf || !str) return -1;
    
    uint32_t len = strlen(str);
    /* Prevent string length overflow */
    if (len > MAX_STRING_LEN) return -1;
    if (len > MAX_BUFFER_SIZE - sizeof(uint32_t)) return -1;
    
    if (buffer_write_uint32(buf, len) < 0) return -1;
    if (len > 0 && buffer_append(buf, str, len) < 0) return -1;
    return 0;
}

static int buffer_write_float(Buffer *buf, float value) {
    if (!buf) return -1;
    uint32_t int_value;
    memcpy(&int_value, &value, sizeof(float));
    uint32_t net_value = htonl(int_value);
    return buffer_append(buf, &net_value, sizeof(net_value));
}

static int buffer_write_uint8(Buffer *buf, uint8_t value) {
    if (!buf) return -1;
    return buffer_append(buf, &value, 1);
}

/* Read helper functions with boundary checks */
static int read_uint32_safe(const uint8_t *data, size_t data_len, size_t *offset, uint32_t *out_value) {
    if (!data || !offset || !out_value) return -1;
    if (data_len == 0) return -1;
    if (*offset >= data_len) return -1;
    if (*offset + sizeof(uint32_t) > data_len) return -1;
    
    uint32_t value;
    memcpy(&value, data + *offset, sizeof(value));
    *offset += sizeof(value);
    *out_value = ntohl(value);
    return 0;
}

static int read_uint8_safe(const uint8_t *data, size_t data_len, size_t *offset, uint8_t *out_value) {
    if (!data || !offset || !out_value) return -1;
    if (data_len == 0) return -1;
    if (*offset >= data_len) return -1;
    
    *out_value = data[*offset];
    *offset += 1;
    return 0;
}

static int read_float_safe(const uint8_t *data, size_t data_len, size_t *offset, float *out_value) {
    if (!data || !offset || !out_value) return -1;
    if (data_len == 0) return -1;
    if (*offset >= data_len) return -1;
    if (*offset + sizeof(uint32_t) > data_len) return -1;
    
    uint32_t net_value;
    memcpy(&net_value, data + *offset, sizeof(net_value));
    *offset += sizeof(net_value);
    uint32_t host_value = ntohl(net_value);
    memcpy(out_value, &host_value, sizeof(float));
    return 0;
}

static int read_uint64_safe(const uint8_t *data, size_t data_len, size_t *offset, uint64_t *out_value) {
    if (!data || !offset || !out_value) return -1;
    if (data_len == 0) return -1;
    
    uint32_t high, low;
    if (read_uint32_safe(data, data_len, offset, &high) < 0) return -1;
    if (read_uint32_safe(data, data_len, offset, &low) < 0) return -1;
    *out_value = ((uint64_t)high << 32) | low;
    return 0;
}

static int read_string_safe(const uint8_t *data, size_t data_len, size_t *offset, char *str, size_t max_len) {
    if (!data || !offset || !str) return -1;
    if (max_len == 0) return -1;
    if (data_len == 0) return -1;
    
    uint32_t len;
    if (read_uint32_safe(data, data_len, offset, &len) < 0) return -1;
    
    /* Prevent excessive string lengths */
    if (len >= max_len) len = max_len - 1;
    if (len > MAX_STRING_LEN) return -1;
    
    /* Check buffer bounds */
    if (*offset + len > data_len) return -1;
    
    if (len > 0) {
        memcpy(str, data + *offset, len);
        *offset += len;
    }
    str[len] = '\0';
    return 0;
}

/* Create message header */
static void create_header(uint8_t *header, MessageType type, uint32_t payload_len) {
    if (!header) return;
    if (payload_len > MAX_PAYLOAD_SIZE) return;
    
    uint32_t magic = htonl(PROTOCOL_MAGIC);
    uint16_t msg_type = htons((uint16_t)type);
    uint32_t length = htonl(payload_len);

    memcpy(header, &magic, 4);
    memcpy(header + 4, &msg_type, 2);
    memcpy(header + 6, &length, 4);
}

/* Create request message */
static int create_request(MessageType type, Buffer *payload, uint8_t *out_data, uint32_t max_size, uint32_t *out_size) {
    if (!out_data || !out_size) return -1;
    if (max_size < HEADER_SIZE) return -1;
    
    uint32_t payload_size = payload ? payload->size : 0;
    
    /* Validate payload size */
    if (payload) {
        if (payload->size > MAX_PAYLOAD_SIZE) return -1;
        if (payload->size > payload->capacity) return -1;
    }
    
    /* Check for overflow */
    if (payload_size > UINT32_MAX - HEADER_SIZE) return -1;
    
    *out_size = HEADER_SIZE + payload_size;
    
    if (*out_size > max_size) return -1;
    if (*out_size > MAX_BUFFER_SIZE) return -1;

    create_header(out_data, type, payload_size);
    if (payload && payload_size > 0) {
        memcpy(out_data + HEADER_SIZE, payload->data, payload_size);
    }

    return 0;
}

/* Socket helper - connect to server */
static int socket_connect(SocketClient *client) {
    int sock = -1;

    if (client->use_tcp) {
        /* TCP socket */
        sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock < 0) return -1;

        struct sockaddr_in addr;
        memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_port = htons(client->port);

        if (inet_pton(AF_INET, client->server_ip, &addr.sin_addr) <= 0) {
            close(sock);
            return -1;
        }

        if (connect(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
            close(sock);
            return -1;
        }
    } else {
        /* Unix domain socket */
        sock = socket(AF_UNIX, SOCK_STREAM, 0);
        if (sock < 0) return -1;

        struct sockaddr_un addr;
        memset(&addr, 0, sizeof(addr));
        addr.sun_family = AF_UNIX;
        // Use memcpy with explicit length to avoid strncpy warning
        size_t path_len = strlen(client->socket_path);
        if (path_len >= sizeof(addr.sun_path)) {
            path_len = sizeof(addr.sun_path) - 1;
        }
        memcpy(addr.sun_path, client->socket_path, path_len);
        addr.sun_path[path_len] = '\0';

        if (connect(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
            close(sock);
            return -1;
        }
    }

    return sock;
}

/* Execute binary protocol request */
static int execute_binary(SocketClient *client, const uint8_t *request_data,
                         uint32_t request_size, Response *response) {
    int sock = -1;
    uint8_t payload_buf[MAX_PAYLOAD_SIZE];
    int result = -1;

    response->success = 0;
    response->message[0] = '\0';

    sock = socket_connect(client);
    if (sock < 0) {
        snprintf(response->message, MAX_STRING_LEN, "Failed to connect to server");
        return -1;
    }

    /* Send request */
    if (write(sock, request_data, request_size) < 0) {
        snprintf(response->message, MAX_STRING_LEN, "Failed to send request");
        goto cleanup;
    }

    /* Read response header */
    uint8_t header_buf[HEADER_SIZE];
    ssize_t bytes_read = read(sock, header_buf, HEADER_SIZE);
    if (bytes_read != HEADER_SIZE) {
        snprintf(response->message, MAX_STRING_LEN, "Failed to read response header");
        goto cleanup;
    }

    /* Parse header */
    if (bytes_read < HEADER_SIZE) {
        snprintf(response->message, MAX_STRING_LEN, "Incomplete header received");
        goto cleanup;
    }
    
    size_t offset = 0;
    uint32_t magic;
    if (read_uint32_safe(header_buf, HEADER_SIZE, &offset, &magic) < 0) {
        snprintf(response->message, MAX_STRING_LEN, "Failed to parse header magic");
        goto cleanup;
    }
    
    offset = 4;
    uint16_t type_net;
    if (offset + 2 > HEADER_SIZE) {
        snprintf(response->message, MAX_STRING_LEN, "Header buffer overflow");
        goto cleanup;
    }
    memcpy(&type_net, header_buf + 4, 2);
    uint16_t resp_type = ntohs(type_net);
    
    offset = 6;
    uint32_t payload_length;
    if (read_uint32_safe(header_buf, HEADER_SIZE, &offset, &payload_length) < 0) {
        snprintf(response->message, MAX_STRING_LEN, "Failed to parse payload length");
        goto cleanup;
    }

    if (magic != PROTOCOL_MAGIC) {
        snprintf(response->message, MAX_STRING_LEN, "Invalid protocol magic");
        goto cleanup;
    }

    /* Read payload if present */
    if (payload_length > 0) {
        if (payload_length > MAX_PAYLOAD_SIZE) {
            snprintf(response->message, MAX_STRING_LEN, "Payload too large");
            goto cleanup;
        }

        bytes_read = read(sock, payload_buf, payload_length);
        if (bytes_read != (ssize_t)payload_length) {
            snprintf(response->message, MAX_STRING_LEN, "Failed to read response payload");
            goto cleanup;
        }

        /* Parse response based on type */
        offset = 0;
        if (resp_type == RESP_SUCCESS) {
            response->success = 1;
            if (read_string_safe(payload_buf, payload_length, &offset, response->message, MAX_STRING_LEN) < 0) {
                snprintf(response->message, MAX_STRING_LEN, "Invalid success message");
                goto cleanup;
            }
            result = 0;

        } else if (resp_type == RESP_ERROR) {
            response->success = 0;
            uint32_t error_code;
            if (read_uint32_safe(payload_buf, payload_length, &offset, &error_code) < 0) goto cleanup;
            char error_msg[MAX_STRING_LEN];
            if (read_string_safe(payload_buf, payload_length, &offset, error_msg, MAX_STRING_LEN) < 0) {
                snprintf(response->message, MAX_STRING_LEN, "Invalid error message");
                goto cleanup;
            }
            // Use separate buffer for formatting to avoid truncation warning
            char temp[MAX_STRING_LEN];
            int written = snprintf(temp, sizeof(temp), "Error %u: ", error_code);
            if (written > 0 && written < (int)sizeof(temp)) {
                size_t remaining = sizeof(temp) - written;
                size_t msg_len = strlen(error_msg);
                if (msg_len > remaining - 1) {
                    msg_len = remaining - 1;
                }
                memcpy(temp + written, error_msg, msg_len);
                temp[written + msg_len] = '\0';
            }
            strncpy(response->message, temp, MAX_STRING_LEN - 1);
            response->message[MAX_STRING_LEN - 1] = '\0';
            result = 0;

        } else if (resp_type == RESP_STATUS) {
            response->success = 1;
            StatusData status;
            memset(&status, 0, sizeof(status));  // Initialize all fields
            
            uint8_t temp_u8;
            uint32_t temp_u32;
            float temp_float;
            
            if (read_uint8_safe(payload_buf, payload_length, &offset, &temp_u8) < 0) goto cleanup;
            status.camera_running = temp_u8;
            if (read_uint8_safe(payload_buf, payload_length, &offset, &temp_u8) < 0) goto cleanup;
            status.recognition_enabled = temp_u8;
            if (read_uint8_safe(payload_buf, payload_length, &offset, &temp_u8) < 0) goto cleanup;
            status.training_in_progress = temp_u8;
            if (read_uint32_safe(payload_buf, payload_length, &offset, &temp_u32) < 0) goto cleanup;
            status.people_count = temp_u32;
            if (read_uint32_safe(payload_buf, payload_length, &offset, &temp_u32) < 0) goto cleanup;
            status.total_faces = temp_u32;
            if (read_float_safe(payload_buf, payload_length, &offset, &temp_float) < 0) goto cleanup;
            status.fps = temp_float;

            if (offset < payload_length) {
                if (read_float_safe(payload_buf, payload_length, &offset, &temp_float) == 0)
                    status.max_face_aspect_ratio = temp_float;
                if (read_float_safe(payload_buf, payload_length, &offset, &temp_float) == 0)
                    status.max_face_degree = temp_float;
                if (read_uint32_safe(payload_buf, payload_length, &offset, &temp_u32) == 0)
                    status.min_face_size = temp_u32;
                if (read_float_safe(payload_buf, payload_length, &offset, &temp_float) == 0)
                    status.det_th = temp_float;
                if (read_float_safe(payload_buf, payload_length, &offset, &temp_float) == 0)
                    status.fas_th = temp_float;
            }

            if (offset < payload_length) {
                if (read_float_safe(payload_buf, payload_length, &offset, &temp_float) == 0)
                    status.detection_time_ms = temp_float;
            }

            snprintf(response->message, MAX_STRING_LEN,
                    "camera_running:%s,recognition_enabled:%s,people_count:%u,total_faces:%u,fps:%.2f,detection_time_ms:%.2f",
                    status.camera_running ? "true" : "false",
                    status.recognition_enabled ? "true" : "false",
                    status.people_count,
                    status.total_faces,
                    status.fps,
                    status.detection_time_ms);
            result = 0;

        } else if (resp_type == RESP_PERSON_LIST) {
            response->success = 1;
            uint32_t count;
            if (read_uint32_safe(payload_buf, payload_length, &offset, &count) < 0) goto cleanup;
            if (count > MAX_PERSONS) count = MAX_PERSONS;

            char temp[MAX_STRING_LEN * 2];
            snprintf(response->message, MAX_STRING_LEN, "count:%u", count);

            for (uint32_t i = 0; i < count; i++) {
                PersonInfo person;
                if (read_string_safe(payload_buf, payload_length, &offset, person.name, MAX_STRING_LEN) < 0) break;
                if (read_uint64_safe(payload_buf, payload_length, &offset, &person.id) < 0) break;
                if (read_uint32_safe(payload_buf, payload_length, &offset, &person.image_count) < 0) break;
                if (read_uint64_safe(payload_buf, payload_length, &offset, &person.created_timestamp) < 0) break;

                int len = snprintf(temp, sizeof(temp), ",person:%s:%llu:%u:%llu",
                        person.name,
                        (unsigned long long)person.id,
                        person.image_count,
                        (unsigned long long)person.created_timestamp);
                if (len > 0 && len < (int)sizeof(temp)) {
                    size_t current_len = strlen(response->message);
                    size_t remaining = MAX_STRING_LEN - current_len - 1;
                    if (remaining > 0) {
                        size_t copy_len = (size_t)len < remaining ? (size_t)len : remaining;
                        memcpy(response->message + current_len, temp, copy_len);
                        response->message[current_len + copy_len] = '\0';
                    }
                }
            }
            result = 0;

        } else {
            snprintf(response->message, MAX_STRING_LEN, "Unexpected response type");
        }
    }

cleanup:
    if (sock >= 0) close(sock);
    return result;
}

/* Public API functions */

static SocketClient unix_client;  /* Static client storage */
static SocketClient tcp_client;   /* Static client storage */

SocketClient* socket_client_create_unix(const char *socket_path) {
    SocketClient *client = &unix_client;
    memset(client, 0, sizeof(SocketClient));

    strncpy(client->socket_path, socket_path ? socket_path : "/tmp/face_recognition.sock",
            sizeof(client->socket_path) - 1);
    client->socket_path[sizeof(client->socket_path) - 1] = '\0';
    client->server_ip[0] = '\0';
    client->port = 0;
    client->use_tcp = 0;

    return client;
}

SocketClient* socket_client_create_tcp(const char *server_ip, int port) {
    SocketClient *client = &tcp_client;
    memset(client, 0, sizeof(SocketClient));

    client->socket_path[0] = '\0';
    strncpy(client->server_ip, server_ip, sizeof(client->server_ip) - 1);
    client->server_ip[sizeof(client->server_ip) - 1] = '\0';
    client->port = port;
    client->use_tcp = 1;

    return client;
}

void socket_client_destroy(SocketClient *client) {
    /* No-op: using static storage */
    (void)client;
}

int socket_client_camera_on(SocketClient *client, Response *response) {
    uint8_t request[HEADER_SIZE];
    uint32_t size = 0;

    if (create_request(REQ_CAMERA_ON, NULL, request, sizeof(request), &size) < 0) {
        snprintf(response->message, MAX_STRING_LEN, "Failed to create request");
        response->success = 0;
        return -1;
    }

    return execute_binary(client, request, size, response);
}

int socket_client_camera_off(SocketClient *client, Response *response) {
    uint8_t request[HEADER_SIZE];
    uint32_t size = 0;

    if (create_request(REQ_CAMERA_OFF, NULL, request, sizeof(request), &size) < 0) {
        snprintf(response->message, MAX_STRING_LEN, "Failed to create request");
        response->success = 0;
        return -1;
    }

    return execute_binary(client, request, size, response);
}

int socket_client_capture(SocketClient *client, const char *initial, uint64_t id, Response *response) {
    Buffer buf;
    buffer_init(&buf);

    if (buffer_write_string(&buf, initial) < 0 ||
        buffer_write_uint64(&buf, id) < 0) {
        snprintf(response->message, MAX_STRING_LEN, "Failed to build request");
        response->success = 0;
        return -1;
    }

    uint8_t request[MAX_BUFFER_SIZE];
    uint32_t size = 0;
    if (create_request(REQ_CAPTURE, &buf, request, sizeof(request), &size) < 0) {
        snprintf(response->message, MAX_STRING_LEN, "Failed to create request");
        response->success = 0;
        return -1;
    }

    return execute_binary(client, request, size, response);
}

int socket_client_train(SocketClient *client, Response *response) {
    uint8_t request[HEADER_SIZE];
    uint32_t size = 0;

    if (create_request(REQ_TRAIN, NULL, request, sizeof(request), &size) < 0) {
        snprintf(response->message, MAX_STRING_LEN, "Failed to create request");
        response->success = 0;
        return -1;
    }

    return execute_binary(client, request, size, response);
}

int socket_client_delete_person(SocketClient *client, const char *name, Response *response) {
    Buffer buf;
    buffer_init(&buf);

    if (buffer_write_string(&buf, name) < 0) {
        snprintf(response->message, MAX_STRING_LEN, "Failed to build request");
        response->success = 0;
        return -1;
    }

    uint8_t request[MAX_BUFFER_SIZE];
    uint32_t size = 0;
    if (create_request(REQ_DELETE_PERSON, &buf, request, sizeof(request), &size) < 0) {
        snprintf(response->message, MAX_STRING_LEN, "Failed to create request");
        response->success = 0;
        return -1;
    }

    return execute_binary(client, request, size, response);
}

int socket_client_status(SocketClient *client, Response *response) {
    uint8_t request[HEADER_SIZE];
    uint32_t size = 0;

    if (create_request(REQ_STATUS, NULL, request, sizeof(request), &size) < 0) {
        snprintf(response->message, MAX_STRING_LEN, "Failed to create request");
        response->success = 0;
        return -1;
    }

    return execute_binary(client, request, size, response);
}

int socket_client_list_persons(SocketClient *client, Response *response) {
    uint8_t request[HEADER_SIZE];
    uint32_t size = 0;

    if (create_request(REQ_LIST_PERSONS, NULL, request, sizeof(request), &size) < 0) {
        snprintf(response->message, MAX_STRING_LEN, "Failed to create request");
        response->success = 0;
        return -1;
    }

    return execute_binary(client, request, size, response);
}

int socket_client_detect_faces(SocketClient *client, int enabled, Response *response) {
    Buffer buf;
    buffer_init(&buf);

    if (buffer_write_uint8(&buf, enabled ? 1 : 0) < 0) {
        snprintf(response->message, MAX_STRING_LEN, "Failed to build request");
        response->success = 0;
        return -1;
    }

    uint8_t request[MAX_BUFFER_SIZE];
    uint32_t size = 0;
    if (create_request(REQ_DETECT_FACES, &buf, request, sizeof(request), &size) < 0) {
        snprintf(response->message, MAX_STRING_LEN, "Failed to create request");
        response->success = 0;
        return -1;
    }

    return execute_binary(client, request, size, response);
}

int socket_client_fas_on(SocketClient *client, Response *response) {
    uint8_t request[HEADER_SIZE];
    uint32_t size = 0;

    if (create_request(REQ_FAS_ON, NULL, request, sizeof(request), &size) < 0) {
        snprintf(response->message, MAX_STRING_LEN, "Failed to create request");
        response->success = 0;
        return -1;
    }

    return execute_binary(client, request, size, response);
}

int socket_client_fas_off(SocketClient *client, Response *response) {
    uint8_t request[HEADER_SIZE];
    uint32_t size = 0;

    if (create_request(REQ_FAS_OFF, NULL, request, sizeof(request), &size) < 0) {
        snprintf(response->message, MAX_STRING_LEN, "Failed to create request");
        response->success = 0;
        return -1;
    }

    return execute_binary(client, request, size, response);
}

int socket_client_set_settings(SocketClient *client, float max_ratio, float max_degree,
                               uint32_t min_size, float det_th, float fas_th, Response *response) {
    Buffer buf;
    buffer_init(&buf);

    if (buffer_write_float(&buf, max_ratio) < 0 ||
        buffer_write_float(&buf, max_degree) < 0 ||
        buffer_write_uint32(&buf, min_size) < 0 ||
        buffer_write_float(&buf, det_th) < 0 ||
        buffer_write_float(&buf, fas_th) < 0) {
        snprintf(response->message, MAX_STRING_LEN, "Failed to build request");
        response->success = 0;
        return -1;
    }

    uint8_t request[MAX_BUFFER_SIZE];
    uint32_t size = 0;
    if (create_request(REQ_SET_SETTINGS, &buf, request, sizeof(request), &size) < 0) {
        snprintf(response->message, MAX_STRING_LEN, "Failed to create request");
        response->success = 0;
        return -1;
    }

    return execute_binary(client, request, size, response);
}

int socket_client_stream_recognition(SocketClient *client) {
    int sock = socket_connect(client);
    if (sock < 0) {
        fprintf(stderr, "Failed to connect to server\n");
        return -1;
    }

    /* Create stream start message */
    uint8_t request[HEADER_SIZE];
    uint32_t size = 0;
    if (create_request(REQ_STREAM_START, NULL, request, sizeof(request), &size) < 0) {
        close(sock);
        fprintf(stderr, "Failed to create stream start message\n");
        return -1;
    }

    if (write(sock, request, size) < 0) {
        close(sock);
        fprintf(stderr, "Failed to send stream start message\n");
        return -1;
    }

    /* Return socket for streaming (caller must close it) */
    return sock;
}
