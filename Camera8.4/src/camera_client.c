/**
 * @brief Binary protocol socket client for testing face recognition server
 *
 * Usage:
 *   ./socket_client 192.168.1.100 camera_on
 *   ./socket_client 192.168.1.100 camera_off
 *   ./socket_client 192.168.1.100 capture A 1
 *   ./socket_client 192.168.1.100 train
 *   ./socket_client 192.168.1.100 status
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

/* Protocol constants */
#define PROTOCOL_MAGIC 0x46524543  /* "FREC" */
#define PROTOCOL_VERSION 1
#define MAX_PAYLOAD_SIZE (1024 * 1024)  /* 1MB */
#define HEADER_SIZE 10  /* 4 + 2 + 4 bytes */
#define DEFAULT_PORT 9999
#define MAX_STRING_LEN 256
#define MAX_PERSONS 100

/* Message types */
typedef enum {
    /* Request messages (Client -> Server) */
    REQ_CAMERA_ON = 0x0001,
    REQ_CAMERA_OFF = 0x0002,
    REQ_CAPTURE = 0x0003,
    REQ_TRAIN = 0x0004,
    REQ_STATUS = 0x0005,
    REQ_STREAM_START = 0x0006,
    REQ_STREAM_STOP = 0x0007,
    REQ_DELETE_PERSON = 0x0008,
    REQ_LIST_PERSONS = 0x0009,
    REQ_GET_SETTINGS = 0x000A,
    REQ_SET_SETTINGS = 0x000B,
    REQ_DETECT_FACES = 0x000C,
    REQ_FAS_ON = 0x000D,
    REQ_FAS_OFF = 0x000E,

    /* Response messages (Server -> Client) */
    RESP_SUCCESS = 0x1001,
    RESP_ERROR = 0x1002,
    RESP_STATUS = 0x1003,
    RESP_PERSON_LIST = 0x1004,
    RESP_SETTINGS = 0x1005,

    /* Stream messages */
    STREAM_FACE_DETECTED = 0x2001,
    STREAM_NO_FACE = 0x2002,
    STREAM_MULTIPLE_FACES = 0x2003,

    /* Event messages */
    EVENT_TRAINING_STARTED = 0x3001,
    EVENT_TRAINING_PROGRESS = 0x3002,
    EVENT_TRAINING_COMPLETED = 0x3003,
    EVENT_TRAINING_FAILED = 0x3004,
    EVENT_CAMERA_ERROR = 0x3005,

    UNKNOWN = 0xFFFF
} MessageType;

/* Message header structure */
typedef struct __attribute__((packed)) {
    uint32_t magic;      /* Protocol magic number */
    uint16_t type;       /* Message type */
    uint32_t length;     /* Payload length */
} MessageHeader;

/* Simple buffer for message payload */
typedef struct {
    uint8_t *data;
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

/* Status response data */
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

/* Person list response data */
typedef struct {
    PersonInfo persons[MAX_PERSONS];
    uint32_t count;
} PersonListData;

/* Response data union */
typedef struct {
    MessageType type;
    union {
        char success_message[MAX_STRING_LEN];
        struct {
            uint32_t error_code;
            char error_message[MAX_STRING_LEN];
        } error;
        StatusData status;
        PersonListData person_list;
    } data;
} Response;

/* All functions below are unused - kept for reference only */
#if 0

/* Buffer functions */
static Buffer* buffer_create(uint32_t initial_capacity) {
    Buffer *buf = (Buffer*)malloc(sizeof(Buffer));
    if (!buf) return NULL;

    buf->data = (uint8_t*)malloc(initial_capacity);
    if (!buf->data) {
        free(buf);
        return NULL;
    }

    buf->size = 0;
    buf->capacity = initial_capacity;
    return buf;
}

static void buffer_destroy(Buffer *buf) {
    if (buf) {
        free(buf->data);
        free(buf);
    }
}

static int buffer_ensure_capacity(Buffer *buf, uint32_t required) {
    if (buf->capacity >= required) return 0;

    uint32_t new_capacity = buf->capacity * 2;
    if (new_capacity < required) new_capacity = required;

    uint8_t *new_data = (uint8_t*)realloc(buf->data, new_capacity);
    if (!new_data) return -1;

    buf->data = new_data;
    buf->capacity = new_capacity;
    return 0;
}

static int buffer_append(Buffer *buf, const void *data, uint32_t len) {
    if (buffer_ensure_capacity(buf, buf->size + len) < 0) return -1;
    memcpy(buf->data + buf->size, data, len);
    buf->size += len;
    return 0;
}

static int buffer_write_uint32(Buffer *buf, uint32_t value) {
    uint32_t net_value = htonl(value);
    return buffer_append(buf, &net_value, sizeof(net_value));
}

static int buffer_write_uint16(Buffer *buf, uint16_t value) {
    uint16_t net_value = htons(value);
    return buffer_append(buf, &net_value, sizeof(net_value));
}

static int buffer_write_string(Buffer *buf, const char *str) {
    uint32_t len = strlen(str);
    if (buffer_write_uint32(buf, len) < 0) return -1;
    return buffer_append(buf, str, len);
}

static int buffer_write_uint64(Buffer *buf, uint64_t value) {
    uint32_t high = htonl((uint32_t)(value >> 32));
    uint32_t low = htonl((uint32_t)(value & 0xFFFFFFFF));
    if (buffer_append(buf, &high, sizeof(high)) < 0) return -1;
    return buffer_append(buf, &low, sizeof(low));
}

/* Read functions */
static uint32_t read_uint32(const uint8_t *data, size_t *offset) {
    uint32_t value;
    memcpy(&value, data + *offset, sizeof(value));
    *offset += sizeof(value);
    return ntohl(value);
}

static uint16_t read_uint16(const uint8_t *data, size_t *offset) {
    uint16_t value;
    memcpy(&value, data + *offset, sizeof(value));
    *offset += sizeof(value);
    return ntohs(value);
}

static uint8_t read_uint8(const uint8_t *data, size_t *offset) {
    uint8_t value = data[*offset];
    *offset += 1;
    return value;
}

static float read_float(const uint8_t *data, size_t *offset) {
    uint32_t net_value;
    memcpy(&net_value, data + *offset, sizeof(net_value));
    *offset += sizeof(net_value);
    uint32_t host_value = ntohl(net_value);
    float result;
    memcpy(&result, &host_value, sizeof(float));
    return result;
}

static uint64_t read_uint64(const uint8_t *data, size_t *offset) {
    uint32_t high = read_uint32(data, offset);
    uint32_t low = read_uint32(data, offset);
    return ((uint64_t)high << 32) | low;
}

static void read_string(const uint8_t *data, size_t *offset, char *str, size_t max_len) {
    uint32_t len = read_uint32(data, offset);
    if (len >= max_len) len = max_len - 1;
    memcpy(str, data + *offset, len);
    str[len] = '\0';
    *offset += len;
}

/* Create message header */
static void create_header(MessageHeader *header, MessageType type, uint32_t payload_len) {
    header->magic = htonl(PROTOCOL_MAGIC);
    header->type = htons((uint16_t)type);
    header->length = htonl(payload_len);
}

/* Send request and receive response */
static int send_and_receive(const char *server_ip, int port,
                           const uint8_t *request_data, uint32_t request_size,
                           Response *response) {
    int sock = -1;
    struct sockaddr_in addr;
    uint8_t header_buf[HEADER_SIZE];
    uint8_t *payload_buf = NULL;
    ssize_t bytes_read;
    int result = -1;

    /* Create socket */
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        fprintf(stderr, "Failed to create socket\n");
        return -1;
    }

    /* Connect to server */
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);

    if (inet_pton(AF_INET, server_ip, &addr.sin_addr) <= 0) {
        fprintf(stderr, "Invalid server IP address: %s\n", server_ip);
        goto cleanup;
    }

    if (connect(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        fprintf(stderr, "Failed to connect to server at %s:%d. Make sure the application is running.\n",
                server_ip, port);
        goto cleanup;
    }

    /* Send request */
    if (write(sock, request_data, request_size) < 0) {
        fprintf(stderr, "Failed to send request\n");
        goto cleanup;
    }

    /* Read response header */
    bytes_read = read(sock, header_buf, HEADER_SIZE);
    if (bytes_read != HEADER_SIZE) {
        fprintf(stderr, "Failed to read response header\n");
        goto cleanup;
    }

    /* Parse header */
    size_t offset = 0;
    uint32_t magic = read_uint32(header_buf, &offset);
    uint16_t type = read_uint16(header_buf, &offset);
    uint32_t payload_length = read_uint32(header_buf, &offset);

    if (magic != PROTOCOL_MAGIC) {
        fprintf(stderr, "Invalid protocol magic\n");
        goto cleanup;
    }

    response->type = (MessageType)type;

    /* Read payload if present */
    if (payload_length > 0) {
        if (payload_length > MAX_PAYLOAD_SIZE) {
            fprintf(stderr, "Payload too large\n");
            goto cleanup;
        }

        payload_buf = (uint8_t*)malloc(payload_length);
        if (!payload_buf) {
            fprintf(stderr, "Failed to allocate payload buffer\n");
            goto cleanup;
        }

        bytes_read = read(sock, payload_buf, payload_length);
        if (bytes_read != (ssize_t)payload_length) {
            fprintf(stderr, "Failed to read response payload\n");
            goto cleanup;
        }

        /* Parse response based on type */
        offset = 0;
        switch (response->type) {
            case RESP_SUCCESS:
                read_string(payload_buf, &offset, response->data.success_message, MAX_STRING_LEN);
                break;

            case RESP_ERROR:
                response->data.error.error_code = read_uint32(payload_buf, &offset);
                read_string(payload_buf, &offset, response->data.error.error_message, MAX_STRING_LEN);
                break;

            case RESP_STATUS:
                response->data.status.camera_running = read_uint8(payload_buf, &offset);
                response->data.status.recognition_enabled = read_uint8(payload_buf, &offset);
                response->data.status.training_in_progress = read_uint8(payload_buf, &offset);
                response->data.status.people_count = read_uint32(payload_buf, &offset);
                response->data.status.total_faces = read_uint32(payload_buf, &offset);
                response->data.status.fps = read_float(payload_buf, &offset);

                /* Optional fields for backward compatibility */
                if (offset < payload_length) {
                    response->data.status.max_face_aspect_ratio = read_float(payload_buf, &offset);
                    response->data.status.max_face_degree = read_float(payload_buf, &offset);
                    response->data.status.min_face_size = read_uint32(payload_buf, &offset);
                    response->data.status.det_th = read_float(payload_buf, &offset);
                    response->data.status.fas_th = read_float(payload_buf, &offset);
                }

                if (offset < payload_length) {
                    response->data.status.detection_time_ms = read_float(payload_buf, &offset);
                }
                break;

            case RESP_PERSON_LIST:
                response->data.person_list.count = read_uint32(payload_buf, &offset);
                if (response->data.person_list.count > MAX_PERSONS) {
                    response->data.person_list.count = MAX_PERSONS;
                }

                for (uint32_t i = 0; i < response->data.person_list.count; i++) {
                    read_string(payload_buf, &offset, response->data.person_list.persons[i].name, MAX_STRING_LEN);
                    response->data.person_list.persons[i].id = read_uint64(payload_buf, &offset);
                    response->data.person_list.persons[i].image_count = read_uint32(payload_buf, &offset);
                    response->data.person_list.persons[i].created_timestamp = read_uint64(payload_buf, &offset);
                }
                break;

            default:
                fprintf(stderr, "Unknown response type: 0x%04X\n", type);
                goto cleanup;
        }
    }

    result = 0;

cleanup:
    if (payload_buf) free(payload_buf);
    if (sock >= 0) close(sock);
    return result;
}

/* Create simple request (no payload) */
static int create_simple_request(MessageType type, uint8_t **out_data, uint32_t *out_size) {
    MessageHeader header;
    create_header(&header, type, 0);

    *out_data = (uint8_t*)malloc(HEADER_SIZE);
    if (!*out_data) return -1;

    memcpy(*out_data, &header, HEADER_SIZE);
    *out_size = HEADER_SIZE;
    return 0;
}

/* Create capture request */
static int create_capture_request(const char *name, uint64_t id,
                                 uint8_t **out_data, uint32_t *out_size) {
    Buffer *buf = buffer_create(256);
    if (!buf) return -1;

    if (buffer_write_string(buf, name) < 0 ||
        buffer_write_uint64(buf, id) < 0) {
        buffer_destroy(buf);
        return -1;
    }

    MessageHeader header;
    create_header(&header, REQ_CAPTURE, buf->size);

    *out_size = HEADER_SIZE + buf->size;
    *out_data = (uint8_t*)malloc(*out_size);
    if (!*out_data) {
        buffer_destroy(buf);
        return -1;
    }

    memcpy(*out_data, &header, HEADER_SIZE);
    memcpy(*out_data + HEADER_SIZE, buf->data, buf->size);

    buffer_destroy(buf);
    return 0;
}

/* Create delete person request */
static int create_delete_request(const char *name, uint8_t **out_data, uint32_t *out_size) {
    Buffer *buf = buffer_create(256);
    if (!buf) return -1;

    if (buffer_write_string(buf, name) < 0) {
        buffer_destroy(buf);
        return -1;
    }

    MessageHeader header;
    create_header(&header, REQ_DELETE_PERSON, buf->size);

    *out_size = HEADER_SIZE + buf->size;
    *out_data = (uint8_t*)malloc(*out_size);
    if (!*out_data) {
        buffer_destroy(buf);
        return -1;
    }

    memcpy(*out_data, &header, HEADER_SIZE);
    memcpy(*out_data + HEADER_SIZE, buf->data, buf->size);

    buffer_destroy(buf);
    return 0;
}

/* Get message type name */
static const char* get_message_type_name(MessageType type) {
    switch (type) {
        case REQ_CAMERA_ON: return "REQ_CAMERA_ON";
        case REQ_CAMERA_OFF: return "REQ_CAMERA_OFF";
        case REQ_CAPTURE: return "REQ_CAPTURE";
        case REQ_TRAIN: return "REQ_TRAIN";
        case REQ_STATUS: return "REQ_STATUS";
        case REQ_DELETE_PERSON: return "REQ_DELETE_PERSON";
        case REQ_LIST_PERSONS: return "REQ_LIST_PERSONS";
        case RESP_SUCCESS: return "RESP_SUCCESS";
        case RESP_ERROR: return "RESP_ERROR";
        case RESP_STATUS: return "RESP_STATUS";
        case RESP_PERSON_LIST: return "RESP_PERSON_LIST";
        default: return "UNKNOWN";
    }
}
#endif