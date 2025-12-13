#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <cstdint>
#include <string>
#include <vector>
#include <cstring>

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

namespace Protocol {

// Protocol magic number "FREC" (Face RECognition)
constexpr uint32_t PROTOCOL_MAGIC = 0x46524543;
constexpr uint16_t PROTOCOL_VERSION = 1;

// Maximum message sizes
constexpr uint32_t MAX_PAYLOAD_SIZE = 1024 * 1024; // 1MB
constexpr uint32_t HEADER_SIZE = 10; // 4 + 2 + 4 bytes

/**
 * @brief Message types for communication
 */
enum class MessageType : uint16_t {
    // Request messages (Client -> Server)
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

    // Response messages (Server -> Client)
    RESP_SUCCESS = 0x1001,
    RESP_ERROR = 0x1002,
    RESP_STATUS = 0x1003,
    RESP_PERSON_LIST = 0x1004,
    RESP_SETTINGS = 0x1005,

    // Stream messages (Server -> Client)
    STREAM_FACE_DETECTED = 0x2001,
    STREAM_NO_FACE = 0x2002,
    STREAM_MULTIPLE_FACES = 0x2003,

    // Event messages (Server -> Client)
    EVENT_TRAINING_STARTED = 0x3001,
    EVENT_TRAINING_PROGRESS = 0x3002,
    EVENT_TRAINING_COMPLETED = 0x3003,
    EVENT_TRAINING_FAILED = 0x3004,
    EVENT_CAMERA_ERROR = 0x3005,

    // Unknown message
    UNKNOWN = 0xFFFF
};

/**
 * @brief Message header structure
 */
struct MessageHeader {
    uint32_t magic;      // Protocol magic number
    uint16_t type;       // Message type
    uint32_t length;     // Payload length

    MessageHeader() : magic(PROTOCOL_MAGIC), type(0), length(0) {}
    
    MessageHeader(MessageType msg_type, uint32_t payload_len)
        : magic(PROTOCOL_MAGIC), type(static_cast<uint16_t>(msg_type)), length(payload_len) {}

    bool is_valid() const {
        return magic == PROTOCOL_MAGIC && length <= MAX_PAYLOAD_SIZE;
    }

    MessageType get_type() const {
        return static_cast<MessageType>(type);
    }
} __attribute__((packed));

/**
 * @brief Base message class
 */
class Message {
public:
    MessageHeader header;
    std::vector<uint8_t> payload;

    Message() = default;
    Message(MessageType type) : header(type, 0) {}
    
    virtual ~Message() = default;

    /**
     * @brief Serialize message to bytes
     */
    std::vector<uint8_t> serialize() const;

    /**
     * @brief Deserialize message from bytes
     */
    static Message deserialize(const std::vector<uint8_t>& data);

    /**
     * @brief Write string to payload
     */
    void write_string(const std::string& str);

    /**
     * @brief Read string from payload
     */
    std::string read_string(size_t& offset) const;

    /**
     * @brief Write uint32_t to payload
     */
    void write_uint32(uint32_t value);

    /**
     * @brief Read uint32_t from payload
     */
    uint32_t read_uint32(size_t& offset) const;

    /**
     * @brief Write uint64_t to payload
     */
    void write_uint64(uint64_t value);

    /**
     * @brief Read uint64_t from payload
     */
    uint64_t read_uint64(size_t& offset) const;

    /**
     * @brief Write uint16_t to payload
     */
    void write_uint16(uint16_t value);

    /**
     * @brief Read uint16_t from payload
     */
    uint16_t read_uint16(size_t& offset) const;

    /**
     * @brief Write uint8_t to payload
     */
    void write_uint8(uint8_t value);

    /**
     * @brief Read uint8_t from payload
     */
    uint8_t read_uint8(size_t& offset) const;

    /**
     * @brief Write float to payload
     */
    void write_float(float value);

    /**
     * @brief Read float from payload
     */
    float read_float(size_t& offset) const;

    /**
     * @brief Write bool to payload
     */
    void write_bool(bool value);

    /**
     * @brief Read bool from payload
     */
    bool read_bool(size_t& offset) const;

protected:
    void finalize() {
        header.length = payload.size();
    }
};

// ============================================================================
// Request Messages
// ============================================================================

/**
 * @brief Camera control request
 */
class CameraControlMessage : public Message {
public:
    bool turn_on;

    CameraControlMessage(bool on)
        : Message(on ? MessageType::REQ_CAMERA_ON : MessageType::REQ_CAMERA_OFF),
          turn_on(on) {
        finalize();
    }

    static CameraControlMessage from_message(const Message& msg) {
        return CameraControlMessage(msg.header.get_type() == MessageType::REQ_CAMERA_ON);
    }
};

/**
 * @brief Capture person request
 */
class CaptureMessage : public Message {
public:
    std::string person_initial;
    uint64_t person_id;

    CaptureMessage(const std::string& initial, uint64_t id)
        : Message(MessageType::REQ_CAPTURE),
          person_initial(initial),
          person_id(id) {
        write_string(person_initial);
        write_uint64(person_id);
        finalize();
    }

    static CaptureMessage from_message(const Message& msg) {
        size_t offset = 0;
        std::string initial = msg.read_string(offset);
        uint64_t id = msg.read_uint64(offset);
        return CaptureMessage(initial, id);
    }
};

/**
 * @brief Train model request
 */
class TrainMessage : public Message {
public:
    TrainMessage() : Message(MessageType::REQ_TRAIN) {
        finalize();
    }

    static TrainMessage from_message(const Message& msg) {
        (void)msg;  // Unused - empty message type
        return TrainMessage();
    }
};

/**
 * @brief Status request
 */
class StatusRequestMessage : public Message {
public:
    StatusRequestMessage() : Message(MessageType::REQ_STATUS) {
        finalize();
    }

    static StatusRequestMessage from_message(const Message& msg) {
        (void)msg;  // Unused - empty message type
        return StatusRequestMessage();
    }
};

/**
 * @brief Stream control request
 */
class StreamControlMessage : public Message {
public:
    bool start;

    StreamControlMessage(bool start_stream)
        : Message(start_stream ? MessageType::REQ_STREAM_START : MessageType::REQ_STREAM_STOP),
          start(start_stream) {
        finalize();
    }

    static StreamControlMessage from_message(const Message& msg) {
        return StreamControlMessage(msg.header.get_type() == MessageType::REQ_STREAM_START);
    }
};

/**
 * @brief Delete person request
 */
class DeletePersonMessage : public Message {
public:
    std::string person_name;

    DeletePersonMessage(const std::string& name)
        : Message(MessageType::REQ_DELETE_PERSON),
          person_name(name) {
        write_string(person_name);
        finalize();
    }

    static DeletePersonMessage from_message(const Message& msg) {
        size_t offset = 0;
        std::string name = msg.read_string(offset);
        return DeletePersonMessage(name);
    }
};

/**
 * @brief List persons request
 */
class ListPersonsMessage : public Message {
public:
    ListPersonsMessage() : Message(MessageType::REQ_LIST_PERSONS) {
        finalize();
    }

    static ListPersonsMessage from_message(const Message& msg) {
        (void)msg;  // Unused - empty message type
        return ListPersonsMessage();
    }
};

/**
 * @brief Settings update request
 */
class SettingsMessage : public Message {
public:
    float confidence_threshold;
    uint32_t detection_interval_ms;
    bool auto_train;

    SettingsMessage(float threshold, uint32_t interval, bool auto_train_enabled)
        : Message(MessageType::REQ_SET_SETTINGS),
          confidence_threshold(threshold),
          detection_interval_ms(interval),
          auto_train(auto_train_enabled) {
        write_float(confidence_threshold);
        write_uint32(detection_interval_ms);
        write_bool(auto_train);
        finalize();
    }

    static SettingsMessage from_message(const Message& msg) {
        size_t offset = 0;
        float threshold = msg.read_float(offset);
        uint32_t interval = msg.read_uint32(offset);
        bool auto_train_enabled = msg.read_bool(offset);
        return SettingsMessage(threshold, interval, auto_train_enabled);
    }
};

// ============================================================================
// Response Messages
// ============================================================================

/**
 * @brief Success response
 */
class SuccessResponse : public Message {
public:
    std::string message;

    SuccessResponse(const std::string& msg = "")
        : Message(MessageType::RESP_SUCCESS),
          message(msg) {
        write_string(message);
        finalize();
    }

    static SuccessResponse from_message(const Message& msg) {
        size_t offset = 0;
        std::string text = msg.read_string(offset);
        return SuccessResponse(text);
    }
};

/**
 * @brief Error response
 */
class ErrorResponse : public Message {
public:
    uint32_t error_code;
    std::string error_message;

    ErrorResponse(uint32_t code, const std::string& msg)
        : Message(MessageType::RESP_ERROR),
          error_code(code),
          error_message(msg) {
        write_uint32(error_code);
        write_string(error_message);
        finalize();
    }

    static ErrorResponse from_message(const Message& msg) {
        size_t offset = 0;
        uint32_t code = msg.read_uint32(offset);
        std::string text = msg.read_string(offset);
        return ErrorResponse(code, text);
    }
};

/**
 * @brief Status response
 */
class StatusResponse : public Message {
public:
    bool camera_running;
    bool recognition_enabled;
    bool training_in_progress;
    uint32_t people_count;
    uint32_t total_faces;
    float fps;

    StatusResponse(bool cam_running, bool rec_enabled, bool training,
                  uint32_t people, uint32_t faces, float current_fps = 0.0f)
        : Message(MessageType::RESP_STATUS),
          camera_running(cam_running),
          recognition_enabled(rec_enabled),
          training_in_progress(training),
          people_count(people),
          total_faces(faces),
          fps(current_fps) {
        write_bool(camera_running);
        write_bool(recognition_enabled);
        write_bool(training_in_progress);
        write_uint32(people_count);
        write_uint32(total_faces);
        write_float(fps);
        finalize();
    }

    static StatusResponse from_message(const Message& msg) {
        size_t offset = 0;
        bool cam = msg.read_bool(offset);
        bool rec = msg.read_bool(offset);
        bool train = msg.read_bool(offset);
        uint32_t people = msg.read_uint32(offset);
        uint32_t faces = msg.read_uint32(offset);
        float current_fps = msg.read_float(offset);
        return StatusResponse(cam, rec, train, people, faces, current_fps);
    }
};

/**
 * @brief Person information
 */
struct PersonInfo {
    std::string name;
    uint32_t id;
    uint32_t image_count;
    uint64_t created_timestamp;

    void serialize(Message& msg) const {
        msg.write_string(name);
        msg.write_uint32(id);
        msg.write_uint32(image_count);
        // Write timestamp as two uint32_t
        msg.write_uint32(static_cast<uint32_t>(created_timestamp >> 32));
        msg.write_uint32(static_cast<uint32_t>(created_timestamp & 0xFFFFFFFF));
    }

    static PersonInfo deserialize(const Message& msg, size_t& offset) {
        PersonInfo info;
        info.name = msg.read_string(offset);
        info.id = msg.read_uint32(offset);
        info.image_count = msg.read_uint32(offset);
        uint32_t ts_high = msg.read_uint32(offset);
        uint32_t ts_low = msg.read_uint32(offset);
        info.created_timestamp = (static_cast<uint64_t>(ts_high) << 32) | ts_low;
        return info;
    }
};

/**
 * @brief Person list response
 */
class PersonListResponse : public Message {
public:
    std::vector<PersonInfo> persons;

    PersonListResponse(const std::vector<PersonInfo>& person_list)
        : Message(MessageType::RESP_PERSON_LIST),
          persons(person_list) {
        write_uint32(persons.size());
        for (const auto& person : persons) {
            person.serialize(*this);
        }
        finalize();
    }

    static PersonListResponse from_message(const Message& msg) {
        size_t offset = 0;
        uint32_t count = msg.read_uint32(offset);
        std::vector<PersonInfo> person_list;
        person_list.reserve(count);
        for (uint32_t i = 0; i < count; ++i) {
            person_list.push_back(PersonInfo::deserialize(msg, offset));
        }
        return PersonListResponse(person_list);
    }
};

// ============================================================================
// Stream Messages
// ============================================================================

/**
 * @brief Face detection stream message
 */
class FaceDetectionMessage : public Message {
public:
    std::string person_name;
    float confidence;
    uint64_t timestamp_ms;
    uint16_t bbox_x;
    uint16_t bbox_y;
    uint16_t bbox_width;
    uint16_t bbox_height;

    FaceDetectionMessage(const std::string& name, float conf, uint64_t ts,
                        uint16_t x = 0, uint16_t y = 0, uint16_t w = 0, uint16_t h = 0)
        : Message(MessageType::STREAM_FACE_DETECTED),
          person_name(name),
          confidence(conf),
          timestamp_ms(ts),
          bbox_x(x),
          bbox_y(y),
          bbox_width(w),
          bbox_height(h) {
        write_string(person_name);
        write_float(confidence);
        // Write timestamp as two uint32_t
        write_uint32(static_cast<uint32_t>(timestamp_ms >> 32));
        write_uint32(static_cast<uint32_t>(timestamp_ms & 0xFFFFFFFF));
        write_uint16(bbox_x);
        write_uint16(bbox_y);
        write_uint16(bbox_width);
        write_uint16(bbox_height);
        finalize();
    }

    static FaceDetectionMessage from_message(const Message& msg) {
        size_t offset = 0;
        std::string name = msg.read_string(offset);
        float conf = msg.read_float(offset);
        uint32_t ts_high = msg.read_uint32(offset);
        uint32_t ts_low = msg.read_uint32(offset);
        uint64_t ts = (static_cast<uint64_t>(ts_high) << 32) | ts_low;
        uint16_t x = msg.read_uint16(offset);
        uint16_t y = msg.read_uint16(offset);
        uint16_t w = msg.read_uint16(offset);
        uint16_t h = msg.read_uint16(offset);
        return FaceDetectionMessage(name, conf, ts, x, y, w, h);
    }
};

/**
 * @brief No face detected message
 */
class NoFaceMessage : public Message {
public:
    uint64_t timestamp_ms;

    NoFaceMessage(uint64_t ts)
        : Message(MessageType::STREAM_NO_FACE),
          timestamp_ms(ts) {
        write_uint32(static_cast<uint32_t>(timestamp_ms >> 32));
        write_uint32(static_cast<uint32_t>(timestamp_ms & 0xFFFFFFFF));
        finalize();
    }

    static NoFaceMessage from_message(const Message& msg) {
        size_t offset = 0;
        uint32_t ts_high = msg.read_uint32(offset);
        uint32_t ts_low = msg.read_uint32(offset);
        uint64_t ts = (static_cast<uint64_t>(ts_high) << 32) | ts_low;
        return NoFaceMessage(ts);
    }
};

// ============================================================================
// Event Messages
// ============================================================================

/**
 * @brief Training progress event
 */
class TrainingProgressMessage : public Message {
public:
    uint32_t current_step;
    uint32_t total_steps;
    std::string status_message;

    TrainingProgressMessage(uint32_t current, uint32_t total, const std::string& status)
        : Message(MessageType::EVENT_TRAINING_PROGRESS),
          current_step(current),
          total_steps(total),
          status_message(status) {
        write_uint32(current_step);
        write_uint32(total_steps);
        write_string(status_message);
        finalize();
    }

    static TrainingProgressMessage from_message(const Message& msg) {
        size_t offset = 0;
        uint32_t current = msg.read_uint32(offset);
        uint32_t total = msg.read_uint32(offset);
        std::string status = msg.read_string(offset);
        return TrainingProgressMessage(current, total, status);
    }
};

// ============================================================================
// Utility Functions
// ============================================================================

/**
 * @brief Get message type name for logging
 */
const char* get_message_type_name(MessageType type);

/**
 * @brief Error codes
 */
enum class ErrorCode : uint32_t {
    SUCCESS = 0,
    UNKNOWN_ERROR = 1,
    INVALID_MESSAGE = 2,
    CAMERA_NOT_RUNNING = 10,
    CAMERA_ALREADY_RUNNING = 11,
    CAMERA_DEVICE_ERROR = 12,
    CAPTURE_FAILED = 20,
    NO_FACE_DETECTED = 21,
    EMBEDDING_EXTRACTION_FAILED = 22,
    REGISTRATION_FAILED = 23,
    TRAINING_IN_PROGRESS = 30,
    TRAINING_FAILED = 31,
    PERSON_NOT_FOUND = 40,
    INVALID_PARAMETERS = 50,
    DATABASE_ERROR = 60,
};

} // namespace Protocol

#endif // PROTOCOL_H
