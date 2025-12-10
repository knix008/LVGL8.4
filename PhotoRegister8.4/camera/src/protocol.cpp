#include "protocol.h"
#include <stdexcept>
#include <cstring>
#include <arpa/inet.h>

namespace Protocol {

// ============================================================================
// Message Implementation
// ============================================================================

std::vector<uint8_t> Message::serialize() const {
    std::vector<uint8_t> data;
    data.reserve(HEADER_SIZE + payload.size());

    // Serialize header (network byte order for portability)
    uint32_t magic_net = htonl(header.magic);
    uint16_t type_net = htons(header.type);
    uint32_t length_net = htonl(header.length);

    // Add magic
    data.insert(data.end(), reinterpret_cast<const uint8_t*>(&magic_net),
                reinterpret_cast<const uint8_t*>(&magic_net) + sizeof(magic_net));

    // Add type
    data.insert(data.end(), reinterpret_cast<const uint8_t*>(&type_net),
                reinterpret_cast<const uint8_t*>(&type_net) + sizeof(type_net));

    // Add length
    data.insert(data.end(), reinterpret_cast<const uint8_t*>(&length_net),
                reinterpret_cast<const uint8_t*>(&length_net) + sizeof(length_net));

    // Add payload
    data.insert(data.end(), payload.begin(), payload.end());

    return data;
}

Message Message::deserialize(const std::vector<uint8_t>& data) {
    if (data.size() < HEADER_SIZE) {
        throw std::runtime_error("Message too short");
    }

    Message msg;
    size_t offset = 0;

    // Read magic
    uint32_t magic_net;
    std::memcpy(&magic_net, data.data() + offset, sizeof(magic_net));
    msg.header.magic = ntohl(magic_net);
    offset += sizeof(magic_net);

    // Read type
    uint16_t type_net;
    std::memcpy(&type_net, data.data() + offset, sizeof(type_net));
    msg.header.type = ntohs(type_net);
    offset += sizeof(type_net);

    // Read length
    uint32_t length_net;
    std::memcpy(&length_net, data.data() + offset, sizeof(length_net));
    msg.header.length = ntohl(length_net);
    offset += sizeof(length_net);

    // Validate header
    if (!msg.header.is_valid()) {
        throw std::runtime_error("Invalid message header");
    }

    // Validate payload length
    if (data.size() < HEADER_SIZE + msg.header.length) {
        throw std::runtime_error("Incomplete message payload");
    }

    // Read payload
    msg.payload.assign(data.begin() + offset, data.begin() + offset + msg.header.length);

    return msg;
}

void Message::write_string(const std::string& str) {
    // Write length as uint32_t
    uint32_t len = str.length();
    write_uint32(len);
    
    // Write string data
    payload.insert(payload.end(), str.begin(), str.end());
}

std::string Message::read_string(size_t& offset) const {
    // Read length
    uint32_t len = read_uint32(offset);
    
    if (offset + len > payload.size()) {
        throw std::runtime_error("String read out of bounds");
    }
    
    // Read string data
    std::string str(reinterpret_cast<const char*>(payload.data() + offset), len);
    offset += len;
    
    return str;
}

void Message::write_uint32(uint32_t value) {
    uint32_t value_net = htonl(value);
    const uint8_t* bytes = reinterpret_cast<const uint8_t*>(&value_net);
    payload.insert(payload.end(), bytes, bytes + sizeof(value_net));
}

uint32_t Message::read_uint32(size_t& offset) const {
    if (offset + sizeof(uint32_t) > payload.size()) {
        throw std::runtime_error("uint32_t read out of bounds");
    }
    
    uint32_t value_net;
    std::memcpy(&value_net, payload.data() + offset, sizeof(value_net));
    offset += sizeof(value_net);
    
    return ntohl(value_net);
}

void Message::write_uint16(uint16_t value) {
    uint16_t value_net = htons(value);
    const uint8_t* bytes = reinterpret_cast<const uint8_t*>(&value_net);
    payload.insert(payload.end(), bytes, bytes + sizeof(value_net));
}

uint16_t Message::read_uint16(size_t& offset) const {
    if (offset + sizeof(uint16_t) > payload.size()) {
        throw std::runtime_error("uint16_t read out of bounds");
    }
    
    uint16_t value_net;
    std::memcpy(&value_net, payload.data() + offset, sizeof(value_net));
    offset += sizeof(value_net);
    
    return ntohs(value_net);
}

void Message::write_uint8(uint8_t value) {
    payload.push_back(value);
}

uint8_t Message::read_uint8(size_t& offset) const {
    if (offset >= payload.size()) {
        throw std::runtime_error("uint8_t read out of bounds");
    }
    
    return payload[offset++];
}

void Message::write_float(float value) {
    // Convert float to network byte order (as uint32_t)
    uint32_t value_int;
    std::memcpy(&value_int, &value, sizeof(value));
    write_uint32(value_int);
}

float Message::read_float(size_t& offset) const {
    uint32_t value_int = read_uint32(offset);
    float value;
    std::memcpy(&value, &value_int, sizeof(value));
    return value;
}

void Message::write_bool(bool value) {
    payload.push_back(value ? 1 : 0);
}

bool Message::read_bool(size_t& offset) const {
    if (offset >= payload.size()) {
        throw std::runtime_error("bool read out of bounds");
    }
    
    return payload[offset++] != 0;
}

void Message::write_uint64(uint64_t value) {
    // Write as two uint32_t values (high and low)
    uint32_t high = htonl(static_cast<uint32_t>(value >> 32));
    uint32_t low = htonl(static_cast<uint32_t>(value & 0xFFFFFFFF));
    
    const uint8_t* high_bytes = reinterpret_cast<const uint8_t*>(&high);
    payload.insert(payload.end(), high_bytes, high_bytes + sizeof(high));
    
    const uint8_t* low_bytes = reinterpret_cast<const uint8_t*>(&low);
    payload.insert(payload.end(), low_bytes, low_bytes + sizeof(low));
}

uint64_t Message::read_uint64(size_t& offset) const {
    if (offset + sizeof(uint64_t) > payload.size()) {
        throw std::runtime_error("uint64_t read out of bounds");
    }
    
    uint32_t high = read_uint32(offset);
    uint32_t low = read_uint32(offset);
    
    return (static_cast<uint64_t>(high) << 32) | low;
}

// ============================================================================
// Utility Functions
// ============================================================================

const char* get_message_type_name(MessageType type) {
    switch (type) {
        // Request messages
        case MessageType::REQ_CAMERA_ON: return "REQ_CAMERA_ON";
        case MessageType::REQ_CAMERA_OFF: return "REQ_CAMERA_OFF";
        case MessageType::REQ_CAPTURE: return "REQ_CAPTURE";
        case MessageType::REQ_TRAIN: return "REQ_TRAIN";
        case MessageType::REQ_STATUS: return "REQ_STATUS";
        case MessageType::REQ_STREAM_START: return "REQ_STREAM_START";
        case MessageType::REQ_STREAM_STOP: return "REQ_STREAM_STOP";
        case MessageType::REQ_DELETE_PERSON: return "REQ_DELETE_PERSON";
        case MessageType::REQ_LIST_PERSONS: return "REQ_LIST_PERSONS";
        case MessageType::REQ_GET_SETTINGS: return "REQ_GET_SETTINGS";
        case MessageType::REQ_SET_SETTINGS: return "REQ_SET_SETTINGS";

        // Response messages
        case MessageType::RESP_SUCCESS: return "RESP_SUCCESS";
        case MessageType::RESP_ERROR: return "RESP_ERROR";
        case MessageType::RESP_STATUS: return "RESP_STATUS";
        case MessageType::RESP_PERSON_LIST: return "RESP_PERSON_LIST";
        case MessageType::RESP_SETTINGS: return "RESP_SETTINGS";

        // Stream messages
        case MessageType::STREAM_FACE_DETECTED: return "STREAM_FACE_DETECTED";
        case MessageType::STREAM_NO_FACE: return "STREAM_NO_FACE";
        case MessageType::STREAM_MULTIPLE_FACES: return "STREAM_MULTIPLE_FACES";

        // Event messages
        case MessageType::EVENT_TRAINING_STARTED: return "EVENT_TRAINING_STARTED";
        case MessageType::EVENT_TRAINING_PROGRESS: return "EVENT_TRAINING_PROGRESS";
        case MessageType::EVENT_TRAINING_COMPLETED: return "EVENT_TRAINING_COMPLETED";
        case MessageType::EVENT_TRAINING_FAILED: return "EVENT_TRAINING_FAILED";
        case MessageType::EVENT_CAMERA_ERROR: return "EVENT_CAMERA_ERROR";

        default: return "UNKNOWN";
    }
}

} // namespace Protocol
