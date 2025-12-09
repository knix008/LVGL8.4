# GTK Face Recognition Application - Architecture Document

## Table of Contents
1. [Overview](#overview)
2. [System Architecture](#system-architecture)
3. [Component Description](#component-description)
4. [Control Flow](#control-flow)
5. [Data Flow](#data-flow)
6. [Key Features](#key-features)

---

## Overview

The GTK Face Recognition Application is a real-time face detection and recognition system built with C++, GTK3, and deep learning models. It captures webcam frames, detects faces, extracts embeddings using ArcFace, and performs fast similarity search using FAISS for real-time face recognition.

**Key Technologies:**
- **GUI Framework:** GTK 3.0
- **Computer Vision:** OpenCV 4.x
- **Deep Learning:** ONNX Runtime (ArcFace model)
- **Vector Search:** FAISS (Facebook AI Similarity Search)
- **Face Detection:** Haar Cascade Classifier
- **Database:** SQLite3

---

## System Architecture

### High-Level Architecture Diagram

```
┌────────────────────────────────────────────────────────────────────────┐
│                    Main Server (gtk_webcam)                             │
│                    gtk_app.cpp / gtk_app.h                              │
│  ┌─────────────────────────────────────────────────────────────────┐  │
│  │  Core Recognition Pipeline                                       │  │
│  │  ├─ Frame Processor (frame_processor)                            │  │
│  │  ├─ Face Detection (FaceDetector - Haar Cascade)                │  │
│  │  ├─ Face Recognition (DeepFaceRecognizer - ArcFace + FAISS)    │  │
│  │  └─ Training Manager (training_manager)                         │  │
│  └─────────────────────────────────────────────────────────────────┘  │
│                            │                                             │
│                            ├─ Socket Server (socket_server)              │
│                            │                                             │
│                            ├─ Face Database (SQLite3)                   │
│                            │                                             │
│                            └─ UI Renderer (ui_renderer)                 │
└──────────────────────┬─────────────────────────────────────────────────┘
                       │
      ┌────────────────┼────────────────┐
      │                │                │
      │          Unix Domain Socket     │
      │          (/tmp/face_recognition.sock)
      │                │                │
      ▼                ▼                ▼
┌─────────────┐ ┌──────────────────────────────────┐
│socket_client│ │     gtk_client (GUI Client)      │
│  (CLI)      │ │  client/src/*.cpp                │
│             │ │  ├─ gtk_client_main.cpp          │
│             │ │  ├─ gtk_client.cpp               │
│             │ │  ├─ socket_client_lib.cpp        │
│             │ │  ├─ socket_server.cpp (shared)   │
│             │ │  └─ socket_client.cpp            │
│             │ └──────────────────────────────────┘
└─────────────┘
```

### Component Hierarchy

```
Main Server (gtk_webcam)
├── GTKApp (Main Application - src/gtk_app.cpp)
│   ├── Camera (Webcam capture)
│   ├── SocketServer (Remote command interface)
│   ├── FaceDetector (Haar Cascade detection)
│   ├── DeepFaceRecognizer (ArcFace + FAISS)
│   │   ├── ModelLoader (ONNX Runtime)
│   │   ├── FAISSIndex (Vector search)
│   │   └── FaceDatabase (Data persistence)
│   ├── FrameProcessor (Processing pipeline)
│   │   ├── FaceDetector
│   │   └── DeepFaceRecognizer
│   ├── TrainingManager (Training orchestration)
│   ├── UIRenderer (UI updates)
│   └── FaceDatabase (SQLite interface)

Client Applications
├── socket_client (CLI - client/src/socket_client.cpp)
│   └── SocketClientLib (Connection handler)
│
└── gtk_client (GUI - client/src/gtk_client.cpp)
    ├── GTKClient (GUI Application)
    ├── SocketClientLib (Connection handler)
    └── SocketServer (Server control)
```

---

## Component Description

### 1. **GTKApp** (gtk_app.cpp / gtk_app.h)
**Responsibility:** Main application controller and GUI manager

**Key Responsibilities:**
- Initialize GTK widgets (buttons, labels, image display)
- Manage camera start/stop operations
- Handle user interactions (capture, training, recognition toggle)
- Update UI with live frames and recognition results
- Coordinate between components

**Key Methods:**
- `init()` - Initialize application and load components
- `run()` - Start GTK main loop
- `toggle_camera()` - Start/stop webcam
- `capture_photo()` - Capture and register new person
- `train_model()` / `train_model_async()` - Train recognition model
- `refresh_frame()` - Update display with camera frames
- `on_training_finished()` - Handle training completion

**UI Components:**
- Start Camera / Stop Camera button
- Registering button (training)
- Capture Photo button
- Status label (feedback messages)
- Face info label (recognized person name)
- Confidence level label
- Recognition time label
- FPS counter label

---

### 2. **Camera** (camera.cpp / camera.h)
**Responsibility:** Webcam frame acquisition

**Key Responsibilities:**
- Open and manage camera device
- Capture frames from webcam
- Handle camera device errors

**Key Methods:**
- `open(device_id)` - Open camera device
- `read(frame)` - Capture a frame
- `release()` - Close camera

---

### 3. **FaceDetector** (face_detector.cpp / face_detector.h)
**Responsibility:** Face detection using Haar Cascade

**Key Responsibilities:**
- Detect faces in images using Haar Cascade classifier
- Extract face bounding boxes with confidence
- Handle multiple face detection

**Key Methods:**
- `initialize()` - Load Haar Cascade XML file
- `detect_faces(image)` - Detect all faces in image
- Returns: `vector<Face>` with bbox, id, name, confidence

**Face Structure:**
```cpp
struct Face {
    cv::Rect bbox;           // Bounding box coordinates
    int id;                  // Person ID (-1 for unknown)
    std::string name;        // Person name
    double confidence;       // Recognition confidence (0-100%)
};
```

---

### 4. **DeepFaceRecognizer** (deep_face_recognizer.cpp / deep_face_recognizer.h)
**Responsibility:** Deep learning-based face recognition using ArcFace

**Key Responsibilities:**
- Load and manage ArcFace ONNX model
- Extract face embeddings from face regions
- Build and manage FAISS index
- Recognize faces by similarity search
- Manage person label mappings

**Key Components:**
- **ModelLoader** - ONNX Runtime inference
- **FAISSIndex** - Fast similarity search
- **FaceDatabase** - Persistent storage

**Key Methods:**
- `load_model(path)` - Load ArcFace ONNX model
- `extract_embedding(face_image)` - Extract 128D embedding
- `train_from_images(dataset_path)` - Train from dataset
- `train_from_database()` - Train from stored embeddings
- `add_training_data(face_image, person_id)` - Incremental training
- `recognize(face_image, confidence)` - Recognize person by image
- `set_label_name(person_id, name)` - Register person name

**Key Data:**
- `person_id_to_name` - Map: person_id → person_name
- `name_to_person_id` - Map: person_name → person_id
- `model_trained` - Flag indicating if model is ready
- `confidence_threshold` - 0.70 (70% for recognition)

---

### 5. **ModelLoader** (model_loader.cpp / model_loader.h)
**Responsibility:** ONNX Runtime inference engine

**Key Responsibilities:**
- Load ArcFace ONNX model
- Perform inference to extract embeddings
- Manage ONNX session and inputs/outputs

**Key Methods:**
- `load_model(path)` - Load ONNX model file
- `inference(preprocessed_image)` - Run inference
- `is_model_loaded()` - Check if model is ready

**Model Details:**
- Model: arcface_w600k_r50.onnx (InsightFace)
- Input: 112×112 normalized RGB image
- Output: 128-dimensional face embedding

---

### 6. **FAISSIndex** (faiss_index.cpp / faiss_index.h)
**Responsibility:** Fast similarity search for face embeddings

**Key Responsibilities:**
- Build and manage FAISS index for embeddings
- Perform fast k-NN search
- Persist index to disk

**Key Methods:**
- `build_index(capacity)` - Create flat index
- `add_vector(person_id, embedding)` - Add single embedding
- `add_vectors(person_ids, embeddings)` - Batch add embeddings
- `search(embedding, confidence)` - Find best match
- `search_k(embedding, k, confidences)` - Find top-k matches
- `save_index(path)` - Persist to disk
- `load_index(path)` - Load from disk

**Index Type:** Flat L2 (Euclidean distance with cosine similarity conversion)

---

### 7. **FaceDatabase** (face_database.cpp / face_database.h)
**Responsibility:** SQLite database for face data persistence

**Key Responsibilities:**
- Manage person records
- Store face images and embeddings
- Retrieve training data

**Key Tables:**
```sql
-- People table
CREATE TABLE people (
    id INTEGER PRIMARY KEY,
    name TEXT UNIQUE,
    face_count INTEGER,
    created_at TIMESTAMP,
    updated_at TIMESTAMP
);

-- Face images table
CREATE TABLE face_images (
    id INTEGER PRIMARY KEY,
    person_id INTEGER FOREIGN KEY,
    image_path TEXT,
    created_at TIMESTAMP
);

-- Face embeddings table
CREATE TABLE face_embeddings (
    id INTEGER PRIMARY KEY,
    person_id INTEGER FOREIGN KEY,
    image_path TEXT,
    embedding_data BLOB,
    created_at TIMESTAMP
);
```

**Key Methods:**
- `add_person(name)` - Register new person
- `add_face_image(person_id, path)` - Store image path
- `add_face_embedding(person_id, path, embedding)` - Store embedding
- `get_person(id)` - Retrieve person record
- `get_all_people()` - Get all registered persons
- `get_all_face_embeddings()` - Get all embeddings for training

---

### 8. **FrameProcessor** (frame_processor.cpp / frame_processor.h)
**Responsibility:** Real-time frame processing pipeline

**Key Responsibilities:**
- Process frames at configured intervals
- Detect faces and perform recognition
- Cache recognition results between intervals
- Measure processing performance

**Key Methods:**
- `process_frame(frame)` - Process single frame
- `enable_recognition(enabled)` - Toggle recognition
- `set_recognition_interval(us)` - Set recognition frequency

**Recognition Cache:**
- Caches last recognition results
- Applies cached results to detected faces between recognition intervals
- Prevents flickering between "Unknown" and "Recognized"

**Performance Metrics:**
- Tracks processing_time_ms for display
- FPS calculation
- Face detection and recognition rates

---

### 9. **TrainingManager** (training_manager.cpp / training_manager.h)
**Responsibility:** Manage model training operations

**Key Responsibilities:**
- Orchestrate training from dataset or database
- Provide training progress and status
- Handle training completion

---

### 10. **UIRenderer** (ui_renderer.cpp / ui_renderer.h)
**Responsibility:** UI rendering utilities

**Key Responsibilities:**
- Draw bounding boxes on frames
- Render text labels
- Convert OpenCV Mat to GTK GdkPixbuf

---

### 11. **Configuration** (config.h)
**Responsibility:** Application-wide configuration constants

**Key Parameters:**
```cpp
namespace Config {
    const int WINDOW_WIDTH = 1280;
    const int WINDOW_HEIGHT = 720;
    const int DISPLAY_WIDTH = 640;
    const int DISPLAY_HEIGHT = 480;

    const int RECOGNITION_UPDATE_INTERVAL_US = 1500000;  // 1.5s
    const double RECOGNITION_CONFIDENCE_THRESHOLD = 0.70; // 70%
    const double BOUNDING_BOX_SCALE = 1.2;
}
```

---

## Client Components

### 12. **SocketServer** (socket_server.cpp / socket_server.h - server/include & client/include)
**Responsibility:** Unix domain socket server for remote command control

**Key Responsibilities:**
- Listen for incoming client connections on Unix domain socket
- Parse and execute commands from remote clients
- Register command handlers for specific operations
- Send responses back to clients

**Key Methods:**
- `start()` - Start socket server in background thread
- `stop()` - Stop server and close socket
- `register_command(name, callback)` - Register command handler
- `is_running()` - Check server status

**Supported Commands:**
- `camera_on` - Start camera
- `camera_off` - Stop camera
- `capture:initial:id` - Capture and register person
- `registering` - Train recognition model
- `status` - Get application status

**Response Format:**
- Success: `OK:message`
- Error: `ERROR:error_message`

---

### 13. **SocketClientLib** (socket_client_lib.cpp / socket_client_lib.h - client/src & client/include)
**Responsibility:** Client library for communicating with socket server

**Key Responsibilities:**
- Establish connections to Unix domain socket
- Send commands to server
- Parse server responses
- Provide high-level API methods

**Key Methods:**
- `camera_on()` - Send camera_on command
- `camera_off()` - Send camera_off command
- `capture(initial, id)` - Send capture command
- `registering()` - Send training command
- `status()` - Get server status
- `send_command(cmd, args)` - Send arbitrary command
- `send_raw(cmd_string)` - Send raw command string

**Response Structure:**
```cpp
struct Response {
    bool success;           // true for OK, false for ERROR
    std::string message;    // Response message content
};
```

---

### 14. **GTKClient** (gtk_client.cpp / gtk_client.h - client/src & client/include)
**Responsibility:** GUI client application for controlling face recognition server

**Key Responsibilities:**
- Provide graphical user interface for server control
- Display server responses and status
- Send commands to server via SocketClientLib
- Handle user interactions

**Key Methods:**
- `init()` - Initialize GTK UI
- `run()` - Start GTK main loop
- `cleanup()` - Clean up resources
- `handle_camera_on()` - Handle camera start button
- `handle_camera_off()` - Handle camera stop button
- `handle_capture()` - Handle photo capture request
- `handle_registering()` - Handle training request
- `handle_status()` - Handle status query
- `append_response(title, response)` - Display response in UI

**UI Components:**
- Camera On/Off buttons
- Capture Photo button (with person ID input)
- Training/Registering button
- Status label
- Response text display

---

### 15. **socket_client** (socket_client.cpp - client/src)
**Responsibility:** Command-line socket client for server control

**Key Responsibilities:**
- Provide CLI interface for server commands
- Parse command-line arguments
- Execute commands via SocketClientLib
- Display results to console

**Supported Commands:**
```bash
./socket_client camera_on
./socket_client camera_off
./socket_client capture A 1
./socket_client registering
./socket_client status
```

---

## Client Directory Structure

```
client/
├── include/                          # Client headers
│   ├── gtk_client.h                  # GTK client GUI class
│   ├── socket_client_lib.h           # Socket client library
│   └── socket_server.h               # Shared socket server header
│
└── src/                              # Client implementation
    ├── gtk_client.cpp                # GTK client GUI implementation
    ├── gtk_client_main.cpp           # GTK client entry point
    ├── socket_client.cpp             # CLI client program
    ├── socket_client_lib.cpp         # Socket client library implementation
    └── socket_server.cpp             # Socket server implementation (shared with main)
```

**Build Artifacts:**
- `socket_client` - CLI client executable
- `gtk_client` - GUI client executable
- Object files: `build/client/*.o`

---

## Control Flow

### 1. Application Startup Flow

```
main()
  │
  ├─→ GTKApp::init()
  │     ├─→ gtk_init()
  │     ├─→ Create GTK widgets (buttons, labels, image display)
  │     ├─→ Camera::open()
  │     ├─→ load_face_recognizer()
  │     │     ├─→ FaceDatabase::open() & initialize()
  │     │     ├─→ FaceDetector::initialize()
  │     │     ├─→ DeepFaceRecognizer::load_model() [ArcFace]
  │     │     ├─→ Try load FAISS index from disk
  │     │     │     └─→ DeepFaceRecognizer::load_index()
  │     │     └─→ Fallback: train_from_database()
  │     └─→ FrameProcessor::initialize()
  │
  └─→ GTKApp::run()
        └─→ gtk_main() [GTK main loop starts]
```

### 2. Live Stream Flow

```
gtk_main_loop (every ~33ms)
  │
  ├─→ on_refresh_timer() [periodic callback]
  │     │
  │     ├─→ Camera::read(frame)
  │     │
  │     ├─→ FrameProcessor::process_frame(frame)
  │     │     │
  │     │     ├─→ FaceDetector::detect_faces()
  │     │     │     └─→ Haar Cascade detection
  │     │     │
  │     │     ├─→ Is it time for recognition? [1.5s interval]
  │     │     │     │
  │     │     │     ├─→ YES:
  │     │     │     │     ├─→ For each detected face:
  │     │     │     │     │     ├─→ Extract face ROI
  │     │     │     │     │     ├─→ DeepFaceRecognizer::recognize()
  │     │     │     │     │     │     ├─→ extract_embedding()
  │     │     │     │     │     │     ├─→ FAISSIndex::search()
  │     │     │     │     │     │     └─→ Get person_name & confidence
  │     │     │     │     │     └─→ Cache results
  │     │     │     │     └─→ Update face.id, face.name, face.confidence
  │     │     │     │
  │     │     │     └─→ NO: Use cached recognition results
  │     │     │
  │     │     └─→ Return processed frame with face data
  │     │
  │     ├─→ Draw faces on frame (bounding boxes, labels)
  │     │
  │     ├─→ Convert Mat to GdkPixbuf
  │     │
  │     ├─→ gtk_image_set_from_pixbuf() [Display frame]
  │     │
  │     └─→ Update UI labels:
  │           ├─→ FPS counter
  │           ├─→ Person name
  │           ├─→ Confidence level
  │           └─→ Recognition processing time
```

### 3. Capture Photo Flow

```
on_capture_button_clicked()
  │
  ├─→ Open dialog: "Enter person initial and ID"
  │     └─→ Wait for user input: person_name (e.g., "A1")
  │
  ├─→ Create directory: dataset/A1/
  │
  ├─→ cv::imwrite() [Save frame to file]
  │
  ├─→ FaceDatabase::add_person(person_name)
  │     └─→ Insert into 'people' table if not exists
  │
  ├─→ FaceDetector::detect_faces() [On saved image]
  │     │
  │     ├─→ If face detected:
  │     │     └─→ Extract face ROI (crop)
  │     └─→ Else: Use full image
  │
  ├─→ DeepFaceRecognizer::extract_embedding(face_roi)
  │     ├─→ Preprocess: Resize to 112×112
  │     └─→ ModelLoader::inference() [ONNX model]
  │
  ├─→ FaceDatabase::add_face_embedding()
  │     └─→ Insert embedding into 'face_embeddings' table
  │
  ├─→ DeepFaceRecognizer::add_training_data(face_roi, person_id)
  │     ├─→ If FAISS index not built: Build with capacity 1000
  │     ├─→ FAISSIndex::add_vector() [Add to index]
  │     ├─→ load_labels_from_database() [Reload person names]
  │     ├─→ Save FAISS index to disk (faiss_index.bin)
  │     └─→ Set model_trained = true
  │
  ├─→ Reload FAISS index from disk into memory
  │
  ├─→ Enable face_recognition_enabled flag
  │
  └─→ Update status: "Person added to recognition model"
```

### 4. Training Flow (Manual)

```
on_train_button_clicked()
  │
  ├─→ Show dialog: "Training in progress..."
  │
  ├─→ Spawn training thread:
  │     │
  │     └─→ train_model_async()
  │           │
  │           ├─→ FaceDatabase::get_all_face_embeddings()
  │           │     └─→ Load all stored embeddings
  │           │
  │           ├─→ DeepFaceRecognizer::train_from_database()
  │           │     ├─→ Extract person_ids and embeddings
  │           │     ├─→ FAISSIndex::build_index()
  │           │     ├─→ FAISSIndex::add_vectors() [Batch add]
  │           │     ├─→ load_labels_from_database()
  │           │     ├─→ Save FAISS index to disk
  │           │     └─→ Set model_trained = true
  │           │
  │           ├─→ Queue on_training_complete() callback to GTK
  │           │
  │           └─→ Update training_success flag
  │
  └─→ on_training_complete() [GTK main thread]
        ├─→ Close dialog
        ├─→ Update status: "Training complete"
        └─→ Enable face recognition
```

---

## Data Flow

### 1. Photo Capture → Recognition

```
Live Frame
    │
    ├─→ Camera::read()
    │
    ├─→ User clicks "Capture Photo"
    │
    ├─→ cv::imwrite(filename, frame)
    │
    ├─→ cv::imread(filename) [Reload from disk]
    │
    ├─→ FaceDetector::detect_faces()
    │     └─→ Face ROI (bbox coordinates)
    │
    ├─→ Extract face region: face_roi = frame(bbox).clone()
    │
    ├─→ DeepFaceRecognizer::extract_embedding(face_roi)
    │     ├─→ Preprocess (resize to 112×112)
    │     ├─→ ONNX inference
    │     └─→ 128D embedding vector
    │
    ├─→ Convert to bytes: embedding_bytes
    │
    ├─→ FaceDatabase::add_face_embedding(person_id, path, embedding_bytes)
    │     └─→ SQLite: INSERT into face_embeddings
    │
    ├─→ FAISSIndex::add_vector(person_id, embedding)
    │     ├─→ In-memory index add
    │     └─→ Persist to faiss_index.bin
    │
    └─→ [Person ready for recognition]
```

### 2. Recognition Process

```
Detected Face in Live Stream
    │
    ├─→ Check if recognition interval elapsed [1.5s]
    │
    ├─→ Extract face ROI from detected bbox
    │
    ├─→ DeepFaceRecognizer::extract_embedding(face_roi)
    │     ├─→ Preprocess (resize to 112×112)
    │     ├─→ ONNX inference
    │     └─→ 128D embedding vector
    │
    ├─→ FAISSIndex::search(embedding, confidence)
    │     ├─→ Compute L2 distance to all stored embeddings
    │     ├─→ Find nearest neighbor
    │     ├─→ Convert to cosine similarity: (1 + cos_theta) / 2
    │     └─→ Return person_id and confidence [0-1]
    │
    ├─→ Apply threshold: if confidence >= 0.70:
    │     ├─→ face.id = person_id (recognized)
    │     ├─→ face.name = get_label_name(person_id)
    │     └─→ face.confidence = confidence * 100
    │
    └─→ Cache results for next 1.5s
        └─→ Apply cached values to subsequently detected faces
```

### 3. Database Persistence

```
Application State
    │
    ├─→ FAISS Index (Memory)
    │     ├─→ On save: faiss_index.bin (disk)
    │     └─→ On load: faiss_index.bin → Memory
    │
    ├─→ Face Embeddings (Memory Cache)
    │     └─→ SQLite: face_embeddings table
    │
    ├─→ Person Labels (Memory Map)
    │     ├─→ person_id_to_name
    │     ├─→ name_to_person_id
    │     └─→ SQLite: people table
    │
    └─→ Face Images (Filesystem)
        └─→ dataset/PersonName/1.jpg, dataset/PersonName/2.jpg, ...
```

### 4. Label Mapping

```
Person Registration
    │
    ├─→ FaceDatabase::add_person(name)
    │     └─→ SQLite: INSERT INTO people(name) → person_id
    │
    ├─→ DeepFaceRecognizer::register_person(name)
    │     ├─→ person_id_to_name[person_id] = name
    │     └─→ name_to_person_id[name] = person_id
    │
    └─→ load_labels_from_database()
        └─→ Rebuild maps from people table
```

---

## Key Features

### 1. **Real-Time Face Detection & Recognition**
- Haar Cascade face detection: ~30ms per frame
- ArcFace embedding extraction: ~20ms per face
- FAISS similarity search: <1ms for 20,000+ people
- Recognition interval: 1.5 seconds (configurable)

### 2. **Incremental Training**
- Add new person via capture without retraining entire model
- `add_training_data()` adds to FAISS in O(1) time
- Automatic FAISS index persistence

### 3. **Recognition Result Caching**
- Cache recognition results between 1.5s intervals
- Prevents flickering between "Unknown" and recognized state
- Improves visual stability on live stream

### 4. **Database Persistence**
- SQLite stores all person records and embeddings
- FAISS index persisted to disk for fast startup
- Can resume recognition immediately on app restart

### 5. **Multi-Face Handling**
- Detects and recognizes multiple faces simultaneously
- Independent confidence scores per face
- Unique colors for different recognition states

### 6. **Flexible Training**
- Train from dataset directory: `train_from_images()`
- Train from database: `train_from_database()`
- Retrain with all embeddings: `retrain_model()`

### 7. **Confidence Thresholding**
- Recognition threshold: 70% (configurable)
- Bounding box color indicates confidence state:
  - Green: Recognized (≥70%)
  - Red: Detected but not recognized (<70%)

### 8. **Performance Monitoring**
- Display processing time for recognition
- FPS counter
- Face detection rate
- Recognition performance metrics

---

## Threading Model

```
Main GTK Thread
├─→ GUI updates
├─→ Frame display
└─→ Event handling

Training Thread (async)
└─→ Model training (blocking operation)
    └─→ Signals completion back to main thread via callback
```

**Thread Safety:**
- ONNX Runtime not thread-safe for recognition (mutex protected)
- Database operations are thread-safe (SQLite)
- FAISS index operations are single-threaded

---

## Error Handling

1. **Model Loading Errors:** Show error dialog, disable recognition
2. **Camera Not Available:** Disable camera button, show message
3. **Database Errors:** Log and continue, warn user
4. **Training Errors:** Show error dialog, preserve previous state
5. **Face Detection/Recognition Errors:** Skip frame, log error

---

## Future Enhancement Opportunities

1. **GPU Acceleration:** Use CUDA for ONNX inference
2. **Batch Processing:** Process multiple frames in parallel
3. **Model Selection:** Support multiple face recognition models
4. **Face Verification:** One-to-one comparison mode
5. **Export/Import:** Save and load person databases
6. **REST API:** Web service interface
7. **Liveness Detection:** Prevent spoofing attacks
8. **Age/Gender Estimation:** Additional face attributes

---

## Conclusion

This architecture provides a scalable, modular design for real-time face recognition. The clear separation of concerns (detection, recognition, storage, UI) allows for easy maintenance and enhancement. The use of industry-standard components (ONNX, FAISS, SQLite) ensures compatibility and future-proofing.
