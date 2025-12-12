#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Binary message protocol for server-client communication
 *
 * Protocol Structure:
 * +--------+--------+----------+----------+
 * | Magic  | MsgType| Length   | Payload  |
 * | 4 bytes| 2 bytes| 4 bytes  | N bytes  |
 * +--------+--------+----------+----------+
 *
 * - Magic: Protocol identifier (0x46524543 = "FREC")
 * - MsgType: Message type (see MessageType enum)
 * - Length: Payload length in bytes
 * - Payload: Message-specific data
 */

/* Protocol constants */
#define PROTOCOL_MAGIC 0x46524543  /* "FREC" (Face RECognition) */
#define PROTOCOL_VERSION 1
#define MAX_PAYLOAD_SIZE (1024 * 1024)  /* 1MB */
#define HEADER_SIZE 10  /* 4 + 2 + 4 bytes */
#define MAX_STRING_LEN 256
#define MAX_PERSONS 100

/**
 * @brief Message types for communication
 */
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

    /* Stream messages (Server -> Client) */
    STREAM_FACE_DETECTED = 0x2001,
    STREAM_NO_FACE = 0x2002,
    STREAM_MULTIPLE_FACES = 0x2003,

    /* Event messages (Server -> Client) */
    EVENT_TRAINING_STARTED = 0x3001,
    EVENT_TRAINING_PROGRESS = 0x3002,
    EVENT_TRAINING_COMPLETED = 0x3003,
    EVENT_TRAINING_FAILED = 0x3004,
    EVENT_CAMERA_ERROR = 0x3005,

    /* Unknown message */
    UNKNOWN = 0xFFFF
} MessageType;

/**
 * @brief Message header structure
 */
typedef struct __attribute__((packed)) {
    uint32_t magic;      /* Protocol magic number */
    uint16_t type;       /* Message type */
    uint32_t length;     /* Payload length */
} MessageHeader;

/**
 * @brief Person information
 */
typedef struct {
    char name[MAX_STRING_LEN];
    uint64_t id;
    uint32_t image_count;
    uint64_t created_timestamp;
} PersonInfo;

/**
 * @brief Status response data
 */
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

/**
 * @brief Person list response data
 */
typedef struct {
    PersonInfo persons[MAX_PERSONS];
    uint32_t count;
} PersonListData;

/**
 * @brief Response data structure
 */
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

/**
 * @brief Helper function to check if message header is valid
 */
static inline bool is_valid_header(const MessageHeader *header) {
    return header && 
           header->magic == PROTOCOL_MAGIC && 
           header->length <= MAX_PAYLOAD_SIZE;
}

#ifdef __cplusplus
}
#endif

#endif /* PROTOCOL_H */
