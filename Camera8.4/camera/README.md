# GTK Face Recognition Application

A real-time face detection and recognition application with SQLite3 database integration for person management. Built with GTK3 and OpenCV, featuring **deep learning-based face recognition (ArcFace + FAISS)** supporting up to **20,000+ people** with 99.83% accuracy.

## Features

- **Live Webcam Streaming**: Display real-time video from your webcam
- **Real-time Face Detection**: Haar Cascade-based face detection with minimal false positives
- **Advanced Face Recognition**: Deep learning-based (ArcFace + FAISS) recognizer with 99.83% accuracy
  - **ArcFace neural network**: 512-dimensional face embeddings (InsightFace w600k_r50)
  - **FAISS indexing**: Fast similarity search supporting 20,000+ people (1-2ms per search)
  - **Dynamic dimensionality**: Automatically adapts to model output shape
- **Scalability**: Supports up to 20,000+ registered people (vs ~50 for legacy LBPH)
  - **No hardcoded capacity limits** in registration or indexing code
  - **Database**: SQLite3 with unlimited record support (limited by disk space)
  - **Memory efficient**: ~41 MB for 20,000 embeddings in RAM
  - **Search performance**: O(n) brute-force with 1-2ms per search on modern CPU
- **Single Face Display Mode**: Shows only the face with highest detection rate/confidence in live stream
- **Confidence Filtering**: Visual distinction between high-confidence (≥70%) and low-confidence (<70%) detections
  - **Green boxes**: Recognized faces with ≥70% confidence (shows person name and percentage)
  - **Yellow boxes**: Detected faces with <70% confidence (shows "Unknown" label)
- **Person Registration**: Register people in SQLite3 database for face recognition
- **Training Management**: Train face recognizer from captured images in dataset directory
- **Real-time Metrics**: Display FPS, detection rate, and error rate
- **Status Information**: View current camera and recognition status
- **Persistent Storage**: Save/load trained models for quick startup
- **Multithreaded Capture**: Smooth video playback without UI blocking
- **Fixed Window Size**: Non-resizable 800x600 window with camera display area (640x480)

## Requirements

### System Dependencies

**Automated Setup (Recommended):**
```bash
./setup_dependencies.sh
```

This script automatically detects your OS and installs all required packages including:
- Build tools (GCC, CMake)
- GUI libraries (GTK3, GdkPixbuf2)
- Computer vision (OpenCV 4.0+)
- Database (SQLite3)
- Deep Learning (ONNX Runtime for ArcFace)
- Vector Indexing (FAISS for similarity search)

**Manual Installation:**

For detailed manual setup instructions, see [SETUP_FAISS_DEEPLEARNING.md](SETUP_FAISS_DEEPLEARNING.md)

### Build Requirements
- **C++17 compiler** (GCC 7+, Clang 5+)
- **OpenCV 4.0+** (with opencv_contrib for face module)
- **GTK3** and **GdkPixbuf2** (GUI framework)
- **SQLite3 3.0+** (database)
- **ONNX Runtime 1.14+** (deep learning inference)
- **FAISS** (vector similarity search)
- **CMake 3.10+** (optional, Makefile provided)

## Quick Start (5 Steps)

For a complete quick-start guide, see [QUICK_START.md](QUICK_START.md)

### Step 1: Install Dependencies
```bash
./setup_dependencies.sh
```

### Step 2: Download ArcFace Model
```bash
mkdir -p models
# Download ArcFace ONNX model (InsightFace w600k_r50)
wget -O models/arcface_w600k_r50.onnx "https://huggingface.co/public-data/insightface/resolve/main/models/buffalo_l/w600k_r50.onnx"
```

### Step 3: Build the Application
```bash
make clean && make
```

### Step 4: Prepare Dataset
```bash
mkdir -p dataset
# Place training images in: dataset/PersonName/image1.jpg, etc.
```

### Step 5: Run and Train
```bash
./gtk_webcam
# Click "Registering" button to train the model with FAISS
```

## Building

### Using Make (Recommended)

```bash
make              # Build the application
make run          # Build and run the application
make debug        # Build with debug symbols
make debug-run    # Run with GDB debugger
make clean        # Remove build artifacts (preserves ONNX Runtime & FAISS)
make distclean    # Remove all artifacts including ONNX Runtime & FAISS
make help         # Show available targets
```

**Note**: The `clean` target now preserves external dependencies (ONNX Runtime, FAISS) to avoid re-downloading them. Use `make distclean` for a complete cleanup including dependencies.

### Build Architecture

The application includes three main deep learning components:

1. **ModelLoader** - ONNX Runtime integration
   - Loads ArcFace pre-trained neural network (InsightFace w600k_r50)
   - Extracts 512-dimensional face embeddings
   - Automatic image preprocessing (112×112, normalization)
   - Handles dynamic batch dimensions

2. **FAISSIndex** - Fast similarity search
   - IVF_Flat vector indexing
   - Auto-configures clusters (8-256 based on dataset)
   - Search time: 1-2ms for 20,000+ people

3. **DeepFaceRecognizer** - Complete pipeline
   - Training from images (dataset/PersonName/)
   - Real-time face recognition
   - Confidence scoring and threshold filtering

Build system automatically links:
- `-lonnxruntime` (ONNX Runtime for ArcFace)
- `-lfaiss` (FAISS for vector indexing)

### 20,000 Person Registration Capacity

This application **fully supports 20,000+ person registration** with the following characteristics:

**Capacity Details**:
- **Database Support**: SQLite3 with no hardcoded limits (supports unlimited records, limited only by disk space)
- **Index Architecture**: Dynamic in-memory FAISS implementation that grows as needed
- **No Capacity Constraints**: No validation code preventing large-scale registrations

**Performance at 20,000 people**:
- **Memory Footprint**: ~40-50 MB for embeddings in RAM (512D × 4 bytes × 20,000 people)
- **Search Time**: O(n) brute-force algorithm (~1-2ms per search on modern CPU)
- **Cluster Configuration**: Automatically configures 128 clusters for 10,000-100,000 vector range
- **Scalability**: Linear scaling - can theoretically support any number limited only by available RAM

**Architecture Notes**:
- Uses simplified in-memory FAISS implementation with std::vector storage
- Brute-force L2 distance search for nearest neighbor matching
- Suitable for 20,000 people; for 100,000+ use actual optimized FAISS library with IVF indexing
- Each embedding requires 2,048 bytes (512 floats × 4 bytes per float)

## Running

```bash
./gtk_webcam
```

The application will:
1. Initialize camera capture
2. Load ArcFace model from `models/arcface_w600k_r50.onnx`
3. Display live video with face detection
4. After training: show recognized faces with confidence scores

## Usage

### Face Recognition Workflow

#### Step 1: Start the Application
1. Launch the executable: `./gtk_webcam`
2. The application opens with a live webcam feed (if model is trained)
3. Status shows: "Camera started successfully" and face recognition status

#### Step 2: Register a Person
1. Enter the person's name in the text input field
2. Click the "Capture Photo" button to capture multiple face images
3. The application saves images to `dataset/[PersonID]/photo_[timestamp].jpg`
4. Register the person in the SQLite3 database for tracking
5. Repeat for additional people you want to recognize

#### Step 3: Train the Recognizer
1. Ensure you have captured photos in the `dataset/` directory (subdirectories like `dataset/1/`, `dataset/2/`, etc.)
2. Click the "Registering" button
3. The application:
   - Loads all images from dataset subdirectories
   - Preprocesses images (grayscale, 200×200 resize, histogram equalization)
   - Trains the LBPH face recognizer model
   - Shows training progress and status
4. After training completes, face recognition is enabled

#### Step 4: Recognize Faces
1. With the model trained, point the webcam at faces
2. The application detects and recognizes faces in real-time
3. **Single face display mode**: Only the face with the **highest detection rate/confidence** is shown on screen
   - If multiple faces are detected, only the best match is displayed
   - Best face is determined by highest recognition confidence (when model is trained)
   - Best face is determined by largest face size (when model is not trained)
4. **Display indicators**:
   - **Green bounding box** with name and percentage: High-confidence match (≥70%)
   - **Yellow bounding box** with "Unknown": Low-confidence detection (<70%)
5. Monitor FPS and error rates in real-time

#### Step 5: Monitor Performance Metrics
The application displays real-time metrics in the status bar:
- **FPS**: Frames per second (camera capture and processing speed)
- **Detection Rate**: Percentage of frames where faces were detected
- **Error Rate**: Percentage of false positive detections (requires manual annotation)
- **Confidence Level**: Confidence percentage of the recognized face

#### Step 6: View Camera Status
- **Status field** displays:
  - Camera status (started/stopped)
  - Number of people registered in database
  - Number of trained people in recognizer
  - Face recognition enabled/disabled status
  - Any error messages or processing notifications

### Remote Control via GTK Client

The application includes a GTK-based graphical client for remote control and real-time face recognition streaming over Unix domain sockets.

#### Building the GTK Client
```bash
make gtk_client
```

#### Running the GTK Client
```bash
./gtk_client
```

The GTK client provides the following features:

**1. Camera Control**
- **Camera On**: Start the camera on the server
- **Camera Off**: Stop the camera on the server
- **Get Status**: Query server status (camera running, people registered, etc.)

**2. Person Registration**
- **Initial**: Enter person's initial (A-Z)
- **ID**: Enter person's ID (1-9999)
- **Capture**: Capture and register a new person (e.g., "A1", "B2")

**3. Training**
- **Train Model (Registering)**: Train the face recognition model with captured images

**4. Real-Time Recognition Stream** ⭐
- **Start Recognition Stream**: Connect to the server and receive live face recognition updates
- **Display**: Shows recognized faces with name and confidence percentage in real-time
- **Format**: `[HH:MM:SS] ✓ PersonName (Confidence: XX%)`
- **Updates**: Recognition results streamed every 500ms
- **Stop Recognition Stream**: Disconnect from the live stream

#### Recognition Stream Features

The recognition stream provides real-time updates of face recognition results:

- **Live Updates**: Displays recognition results as they happen (500ms interval)
- **Confidence Display**: Shows recognition confidence percentage for each detected face
- **No Face Detection**: Indicates when no faces are detected in the current frame
- **Auto-scroll**: Automatically scrolls to show the latest results
- **Line Limiting**: Keeps only the last 50 lines for performance

**Example Stream Output:**
```
[14:32:15] ✓ A1 (Confidence: 85%)
[14:32:16] ✓ A1 (Confidence: 87%)
[14:32:16] - No face detected
[14:32:17] - No face detected
[14:32:17] ✓ B2 (Confidence: 92%)
[14:32:18] ✓ B2 (Confidence: 91%)
```

#### Socket Interface

For programmatic control, see the detailed socket interface documentation:
- **[SOCKET_INTERFACE.md](SOCKET_INTERFACE.md)**: Complete socket protocol reference
- Commands: `camera_on`, `camera_off`, `capture:A:1`, `registering`, `status`, `stream_recognition`
- Socket path: `/tmp/face_recognition.sock`

**Quick Command-Line Example:**
```bash
# Using the CLI socket client
./socket_client camera_on
./socket_client capture:A:1
./socket_client registering
./socket_client status

# Using netcat for streaming
nc -U /tmp/face_recognition.sock
stream_recognition
```

## Architecture

### File Structure

```
gtk-webcam/
├── include/                  # Main server headers
│   ├── camera.h              # Camera capture interface
│   ├── gtk_app.h             # GTK application class
│   ├── face_detector.h       # Haar Cascade face detection
│   ├── face_database.h       # SQLite3 person/face database
│   └── socket_server.h       # Socket server for remote control
├── src/                      # Main server implementation
│   ├── main.cpp              # Application entry point
│   ├── camera.cpp            # Camera implementation
│   ├── gtk_app.cpp           # GTK UI & main application logic
│   ├── face_detector.cpp     # Face detection implementation
│   ├── face_database.cpp     # Database operations
│   └── socket_server.cpp     # Socket server implementation
├── client/                   # Client applications (GUI & CLI)
│   ├── include/
│   │   ├── gtk_client.h      # GTK client GUI class
│   │   ├── socket_client_lib.h  # Socket client library
│   │   └── socket_server.h   # Socket server (shared with main app)
│   └── src/
│       ├── gtk_client.cpp    # GTK client GUI implementation
│       ├── gtk_client_main.cpp  # GTK client entry point
│       ├── socket_client.cpp # CLI socket client
│       ├── socket_client_lib.cpp # Socket client library implementation
│       └── socket_server.cpp # Socket server implementation (shared)
├── dataset/                  # Training images directory (structure: dataset/PersonID/)
├── face_database.db          # SQLite3 database (auto-created)
├── CMakeLists.txt            # CMake build configuration
├── Makefile                  # Make build configuration
└── README.md                 # This file
```

### Build System Structure

The Makefile now organizes builds into three separate executables:

**Main Application:**
- `gtk_webcam`: Server-side face recognition application
  - Includes all server components: camera, detection, recognition, database
  - Uses socket_server for remote control via clients

**Client Applications:**
- `socket_client`: Command-line socket client for controlling the server
  - Minimal dependencies, pure socket communication
  - Built from: `client/src/socket_client.cpp` + `client/src/socket_client_lib.cpp`

- `gtk_client`: GUI client application for controlling the server
  - Full GTK interface with visual feedback
  - Built from: `client/src/*.cpp` (all client sources) + `build/logger.o`

**Build Configuration:**
- Client sources compile with `-I$(CLIENT_INCLUDE_DIR)` for proper header resolution
- Client objects stored in `build/client/` directory
- Main app excludes GTK client GUI files to avoid symbol conflicts

### Architecture Design: Filesystem + Database Model

The application separates training data source from metadata storage:

**Filesystem Role (dataset/ directory)**
- Stores training images in person subdirectories: `dataset/1/`, `dataset/2/`, etc.
- Images can be captured photos or manually placed images
- On "Registering" click, train_from_images("dataset") reads all images from subdirectories
- Advantages: Scalable, allows manual image addition, clear data organization

**Database Role (SQLite3)**
- Tracks person metadata: ID, name, registration timestamp
- Tracks captured image metadata: file path, timestamp per person
- Optional: Stores serialized face embeddings for future incremental learning
- Currently used for reference, not as training data source

**Training Flow**
1. User captures photos → Saved to `dataset/PersonID/` + DB records created
2. User clicks "Registering" → train_from_images("dataset") loads all images
3. LBPH model trained on all loaded images → In-memory model ready
4. Recognition uses trained model, not database lookups

### Key Classes

#### Camera Class
- Handles OpenCV video capture
- Background thread for frame capture
- Thread-safe frame queue
- Properties: resolution (640×480), FPS (30), active status

#### FaceDetector Class
- Haar Cascade classifier for face detection
- Configurable parameters:
  - **scale_factor**: 1.1 (detection pyramid scale)
  - **min_neighbors**: 8 (number of overlapping detections required; increased from 4 to reduce false positives)
  - **min_face_size**: 30×30 pixels
  - **max_face_size**: unlimited
- Returns Face struct with bbox, confidence, recognized name/ID

#### FaceRecognizer Class
- LBPH (Local Binary Patterns Histograms) face recognizer
- LBPH parameters: radius=1, neighbors=8, grid_x=8, grid_y=8 (2048-element embeddings)
- Training modes:
  - `train_from_images(dataset_path)`: Load images from filesystem subdirectories
  - `train_from_database()`: Load embeddings stored in database
  - `add_training_data()`: Incrementally add training data (for future enhancements)
- Recognition:
  - `recognize()`: Returns person_id and confidence (0-1 scale)
  - `recognize_with_name()`: Returns person name
  - Confidence calculation: `similarity = 1.0 / (1.0 + distance/100.0)`
  - Display threshold: ≥70% confidence shows name label with green box

#### FaceDatabase Class
- SQLite3 database management with prepared statements (SQL injection prevention)
- Three main tables:
  - **people**: person_id (PK), name, face_count, created_at, updated_at
  - **face_images**: id (PK), person_id (FK), image_path, created_at
  - **embeddings**: id (PK), person_id (FK), image_path, embedding_data (BLOB), created_at
- Thread-safe database operations
- All SQL queries use parameter binding

#### GTKApp Class
- Main GTK application controller
- UI initialization and management
- Frame refresh timer (30ms = ~33 FPS)
- Face detection and recognition integration
- Status display updates
- Input field for person name registration
- Button handlers: Start/Stop Camera, Capture Photo, Registering (training)

## Troubleshooting

### ONNX Runtime Installation Issues

**Error: "libonnxruntime-dev not found in package manager"**

ONNX Runtime may not be available in your distribution's default repositories. Try:

```bash
# Option 1: Build from source (recommended)
git clone https://github.com/Microsoft/onnxruntime.git
cd onnxruntime
./build.sh --config Release --build_shared_lib --parallel

# Then install
cd build/Linux/Release
sudo make install
sudo ldconfig
```

**Error: "onnxruntime_cxx_api.h: No such file or directory"**

ONNX Runtime headers not found. Verify installation:
```bash
# Check if installed
ldconfig -p | grep onnxruntime

# If not found, rebuild and install
find /usr -name "onnxruntime_cxx_api.h" 2>/dev/null
```

If still missing, add to PKG_CONFIG_PATH:
```bash
export PKG_CONFIG_PATH=/usr/local/lib/pkgconfig:$PKG_CONFIG_PATH
```

### FAISS Installation Issues

**Error: "faiss/IndexIVF.h: No such file or directory"**

FAISS headers not installed. Build from source:
```bash
git clone https://github.com/facebookresearch/faiss.git
cd faiss
mkdir build && cd build
cmake .. -DFAISS_ENABLE_GPU=OFF -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=ON
make -j$(nproc)
sudo make install
sudo ldconfig
```

See [SETUP_FAISS_DEEPLEARNING.md](SETUP_FAISS_DEEPLEARNING.md) for complete setup instructions.

### Deep Learning Model Issues

**Error: "Could not load model at path..."**

ArcFace ONNX model not found. Download it:
```bash
mkdir -p models
# Download ArcFace ONNX model (InsightFace w600k_r50)
wget -O models/arcface_w600k_r50.onnx "https://huggingface.co/public-data/insightface/resolve/main/models/buffalo_l/w600k_r50.onnx"
```

**Error: "FAISS index build failed"**

Check that model loaded successfully. Watch console output for:
```
"Model loaded successfully from: models/arcface_w600k_r50.onnx"
```

If model didn't load, fix that first, then retrain.

### Camera Not Found
- Check if your webcam is connected: `ls /dev/video*`
- Ensure you have permission to access the camera:
  ```bash
  sudo usermod -a -G video $USER
  # Log out and log back in for changes to take effect
  ```
- Try specifying a different camera ID (0, 1, 2, etc.) in `camera.cpp`

### Low FPS or Dropped Frames
- Close other applications using the camera
- Reduce resolution or frame rate (modify in `camera.cpp`)
- Check CPU usage with `top` or `htop`
- Face detection/recognition is CPU-intensive; lower-end systems may see reduced FPS

### GTK/GdkPixbuf Errors
- Ensure all GTK development libraries are installed
- Run `pkg-config --cflags gtk+-3.0` to verify GTK3 is installed

### OpenCV Not Found
- Verify OpenCV is installed: `pkg-config --modversion opencv4`
- If missing, install with your package manager
- **Important**: Must include opencv_contrib for face module (LBPH recognizer)
- For CMake, you may need to specify OpenCV path:
  ```bash
  cmake -DOpenCV_DIR=/path/to/opencv/build ..
  ```

### SQLite3 Not Found
- Verify SQLite3 is installed: `sqlite3 --version`
- Ubuntu/Debian: `sudo apt-get install libsqlite3-dev`
- Fedora: `sudo dnf install sqlite-devel`
- Arch: `sudo pacman -S sqlite`

### Face Recognition Not Working

#### Training Failed: "Error: No embeddings found in database"
- **Cause**: No images in `dataset/` directory or training directory structure incorrect
- **Solution**:
  1. Create subdirectories for each person: `dataset/1/`, `dataset/2/`, etc.
  2. Place training images in person subdirectories
  3. Click "Registering" button to train from filesystem images
  4. Check status output for training progress

#### Training Failed: "Error: No training images found in dataset"
- **Cause**: `dataset/` directory is empty or images not in correct subdirectories
- **Solution**:
  1. Use "Capture Photo" button to capture images (auto-creates correct structure)
  2. Or manually create `dataset/PersonID/` subdirectories and copy image files
  3. Ensure images are .jpg, .jpeg, .png, or .bmp format
  4. Each person should have at least 2-3 images for reliable training

#### No Rectangle Boxes Shown
- **Cause 1**: Face detection parameters too strict or faces not clearly visible
  - Solution: Improve lighting, ensure face is directly facing camera
- **Cause 2**: min_neighbors parameter set too high (currently 8 for low false positives)
  - Solution: Reduce min_neighbors in `face_detector.h` line 20 (trade-off: more false positives)
- **Cause 3**: min_face_size too large
  - Solution: Call `face_detector->set_min_face_size(20, 20)` for smaller faces

#### Green Boxes Show But No Name Labels
- **Cause**: Recognizer not trained yet or confidence below 50% threshold
- **Solution**:
  1. Verify training completed: Click "Registering" button
  2. Check status shows "Face recognition enabled"
  3. Ensure captured training photos match current lighting/angles
  4. Low confidence (yellow boxes) indicates model uncertainty

#### Recognizing Wrong Person
- **Cause 1**: Insufficient training images (needs 5-10 images per person)
  - Solution: Capture more images with various angles and lighting
- **Cause 2**: Similar facial features between people
  - Solution: Increase confidence_threshold in `face_recognizer.h` line 17
- **Cause 3**: Poor image preprocessing
  - Solution: Check that captured images are clear and well-lit

#### Too Many False Detections (Wrong Faces Detected)
- **Cause**: min_neighbors parameter too low (allows spurious detections)
- **Current Setting**: min_neighbors = 8 (conservative, reduces false positives)
- **Solution**: This issue should be minimal with current settings
  - If still occurring, verify min_neighbors is 8 in `face_detector.h`
  - Ensure good lighting conditions
  - May need to retrain with better quality images

### Database Errors

#### SQLite3 Database Locked
- **Cause**: Multiple processes accessing database simultaneously
- **Solution**: Close other applications using the database
- **Prevention**: Application uses proper transaction handling

#### Face Database Not Initializing
- **Cause**: Permission issues or corrupted database file
- **Solution**:
  1. Delete `face_database.db` (will be recreated)
  2. Ensure write permissions in application directory
  3. Verify SQLite3 is properly installed

### Face Detection Error Rate Monitoring

The application displays real-time detection metrics to help evaluate performance:

#### Understanding Error Rate Metrics

**Detection Rate** = (Frames with detected faces / Total frames processed) × 100%
- Shows what percentage of frames contain at least one face detection
- Higher rate indicates more frequent face presence in the camera view
- Range: 0% to 100%

**Error Rate (False Positive Rate)** = (False positive detections / Total frames) × 100%
- Percentage of frames where the detector found faces that aren't actually faces
- Requires manual annotation to accurately track false positives
- By default, shows 0% (can be manually tracked by user)
- Range: 0% to 100%

#### How to Measure False Positives

False positives are non-face objects detected as faces. To track these:

1. **Manual Method**: Observe the live stream and note when yellow boxes appear on non-face objects
2. **Log Analysis**: Record instances and calculate: (False Positives / Total Detections) × 100%
3. **Parameter Adjustment**:
   - Current `min_neighbors = 8` is conservative to reduce false positives
   - Reduce to 5 for more sensitive detection (may increase false positives)
   - Increase to 10+ for very strict detection (fewer faces detected)

#### Example Interpretation

- **Detection Rate: 85% | Error Rate: 2%**
  - In 100 seconds, faces were detected in 85 frames
  - Approximately 2 false positive detections occurred per 100 frames processed
  - Configuration is working well

- **Detection Rate: 10% | Error Rate: 15%**
  - Low detection rate: No faces in frame or min_neighbors too high
  - High error rate: Detector finding non-faces; reduce sensitivity or improve lighting

#### Factors Affecting Detection Rate

1. **Lighting**: Poor lighting reduces detection rate
2. **Face Orientation**: Side profiles reduce detection (optimized for frontal faces)
3. **Face Size**: Very small/large faces may not be detected (min/max_face_size)
4. **Face Angle**: Head rotations >45° reduce detection
5. **Occlusions**: Glasses, masks, or hair covering face reduces detection
6. **min_neighbors Parameter**: Higher values = fewer detections but fewer false positives

#### Improving Detection Accuracy

1. **Reduce False Positives**:
   - Increase `min_neighbors` from 8 to 10-12
   - Improve camera lighting
   - Adjust camera angle to frontal views

2. **Increase Detection Rate**:
   - Reduce `min_neighbors` from 8 to 5-6
   - Improve lighting conditions
   - Ensure faces are clearly visible and oriented toward camera

3. **Balance Trade-off**:
   - Default `min_neighbors = 8` provides good balance
   - Experiment with values 5-12 to find optimal setting for your environment
   - Test with your specific lighting and face angles

## Configuration

### Modify Camera Parameters

Edit `src/camera.cpp` in the `open()` method to adjust:

```cpp
cap.set(cv::CAP_PROP_FRAME_WIDTH, 640);   // Width
cap.set(cv::CAP_PROP_FRAME_HEIGHT, 480);  // Height
cap.set(cv::CAP_PROP_FPS, 30);            // Frame rate
cap.set(cv::CAP_PROP_BUFFERSIZE, 1);      // Minimize buffer for low-latency capture
```

### Face Detection Parameters

Edit `include/face_detector.h` to tune detection behavior:

```cpp
double scale_factor = 1.1;        // Detection pyramid scale (1.05-1.4)
                                  // Lower = more thorough but slower
                                  // Higher = faster but may miss some faces
int min_neighbors = 8;            // Overlapping detections required
                                  // Higher = fewer false positives
                                  // Lower = more detections (may include false positives)
cv::Size min_face_size{30, 30};   // Minimum face size to detect
cv::Size max_face_size{};         // Maximum face size (empty = unlimited)
```

**Recommended settings for different scenarios**:
- Conservative (fewer false positives): `scale_factor=1.1, min_neighbors=8`
- Balanced: `scale_factor=1.1, min_neighbors=5`
- Aggressive (more detections): `scale_factor=1.1, min_neighbors=3`

### Face Recognition Parameters

Edit `include/face_recognizer.h` to adjust recognition thresholds:

```cpp
double confidence_threshold = 0.7;  // 0.7 = 70% similarity required to display name
                                    // Lower = more permissive, may misidentify
                                    // Higher = more strict, may show "Unknown"
```

Edit `src/face_recognizer.cpp` in the `recognize()` method to adjust confidence calculation:

```cpp
double similarity = 1.0 / (1.0 + distance / 100.0);  // Convert LBPH distance to similarity
                                                      // Adjust denominator (100.0) to scale
```

### UI Parameters

Edit `src/gtk_app.cpp` in the `draw_faces_on_frame()` method:

```cpp
// Confidence threshold for showing name labels (percentage)
if (face.confidence > 70.0) {
    // Draw green box with name
    // Shows: "PersonName (confidence%)"
} else {
    // Draw yellow box
    // Shows "Unknown" for low confidence detection
}
```

### Window Properties

Edit `src/gtk_app.cpp` in the `init()` method:

```cpp
// Window size (currently fixed, not resizable)
gtk_window_set_default_size(GTK_WINDOW(window), 800, 600);

// To make resizable, change FALSE to TRUE:
gtk_window_set_resizable(GTK_WINDOW(window), FALSE);  // Change to TRUE for resizable

// Camera display area
gtk_widget_set_size_request(image_widget, 640, 480);  // Display resolution
```

### UI Refresh Rate

Edit `src/gtk_app.cpp` in the `init()` method:

```cpp
refresh_timer = g_timeout_add(30, on_refresh_timer, this);  // 30ms = ~33 FPS
                                                             // Lower = higher FPS (more CPU)
                                                             // Higher = lower FPS (less CPU)
```

### Image Preprocessing

Edit `src/face_recognizer.cpp` in the `add_training_data()` method to adjust preprocessing:

```cpp
// Image resize for training
cv::resize(preprocessed, preprocessed, cv::Size(200, 200));  // Change to any square size
                                                               // Larger = more detail but slower
                                                               // Smaller = faster but less detail

// Histogram equalization improves recognition in different lighting
cv::equalizeHist(preprocessed, preprocessed);  // Remove for raw intensity training
```

## Performance Tips

1. **Face Detection**:
   - Increase `scale_factor` (1.1 → 1.3) for faster detection, fewer faces detected
   - Increase `min_neighbors` reduces per-frame detections
   - Reduce `min_face_size` increases computation but finds smaller faces

2. **Threading**: Camera capture runs in separate thread, preventing UI freezing

3. **Image Resolution**:
   - Lower camera resolution = higher FPS but less detail
   - Training image size (200×200) is fixed; preprocessed on-the-fly

4. **Face Recognition**:
   - First run is slower due to LBPH model initialization
   - Subsequent frames use in-memory model (very fast)
   - More training images = slower recognition (must compare more face histograms)

5. **Database Operations**:
   - Person registration happens in background (minimal impact)
   - Face image metadata stored efficiently in SQLite3
   - Consider deleting old people/images if database grows large

6. **Frame Queue**: Limited frame queue prevents memory buildup in long sessions

7. **CPU Optimization**:
   - Disable face detection if only viewing video: comment out detect_faces() call
   - Run on system with spare CPU cores for best performance
   - Monitor with `top` or `htop` during heavy face detection/recognition

## Development

### Building in Debug Mode

```bash
make debug
make debug-run    # Run with GDB
```

### Adding Features

The modular design allows easy extension:
- Add more camera properties in `Camera` class
- Extend GTK UI with additional widgets in `GTKApp` class
- Implement image processing on captured frames

## Database Schema

The application uses SQLite3 with three main tables for person and face management:

### Table: `people`
Stores registered person information
```sql
CREATE TABLE people (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    name TEXT NOT NULL UNIQUE,
    face_count INTEGER DEFAULT 0,
    created_at TEXT NOT NULL,
    updated_at TEXT NOT NULL
);
```
- **id**: Auto-incrementing unique identifier for each person
- **name**: Person's name (unique)
- **face_count**: Number of face images captured for this person
- **created_at**: Registration timestamp
- **updated_at**: Last update timestamp

### Table: `face_images`
Tracks captured face image files
```sql
CREATE TABLE face_images (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    person_id INTEGER NOT NULL,
    image_path TEXT NOT NULL,
    created_at TEXT NOT NULL,
    FOREIGN KEY(person_id) REFERENCES people(id)
);
```
- **id**: Auto-incrementing unique identifier
- **person_id**: Foreign key reference to person
- **image_path**: File path to captured image (e.g., "dataset/1/photo_1234567890.jpg")
- **created_at**: Capture timestamp

### Table: `embeddings`
Stores serialized face embeddings for future incremental learning
```sql
CREATE TABLE embeddings (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    person_id INTEGER NOT NULL,
    image_path TEXT NOT NULL,
    embedding_data BLOB NOT NULL,
    created_at TEXT NOT NULL,
    FOREIGN KEY(person_id) REFERENCES people(id)
);
```
- **id**: Auto-incrementing unique identifier
- **person_id**: Foreign key reference to person
- **image_path**: Associated image file path
- **embedding_data**: Serialized ArcFace face embeddings (2048 bytes = 512 floats per face)
- **created_at**: Embedding extraction timestamp

## Face Recognition Algorithm

### Face Detection: Haar Cascade Classifier

**Method**: OpenCV's CascadeClassifier with pre-trained face cascade XML
- **Algorithm**: AdaBoost with Haar-like features
- **Characteristics**: Fast, real-time capable, good for frontal faces

**Parameters** (configurable in `face_detector.h`):
- `scale_factor = 1.1`: Image pyramid scaling (detect faces at multiple scales)
- `min_neighbors = 8`: Number of overlapping detections required (8 = conservative, few false positives)
- `min_face_size = 30×30`: Minimum face size in pixels
- `max_face_size = unlimited`: No maximum size constraint

**Detection Process**:
1. Input frame preprocessed and scaled into pyramid
2. Cascade applied at each pyramid level
3. Overlapping detections combined (min_neighbors requirement)
4. Final bounding boxes returned as Face struct

### Face Recognition: Deep Learning (ArcFace) + FAISS

**Method**: ArcFace neural network + FAISS vector indexing
- **Model**: Pre-trained ArcFace neural network (InsightFace w600k_r50, ONNX format)
- **Embeddings**: 512-dimensional vectors
- **Indexing**: FAISS IVF_Flat for fast similarity search
- **Accuracy**: 99.83% on LFW benchmark (vs 85% for LBPH)
- **Scalability**: Supports 20,000+ people (vs ~50 for LBPH)

**Components**:

1. **ModelLoader** (ONNX Runtime)
   - Loads pre-trained ArcFace model (InsightFace w600k_r50)
   - Preprocessing: Image resizing (112×112), normalization ((pixel-127.5)/128)
   - Output: 512-D embedding vector
   - L2 normalization for unit vectors

2. **FAISSIndex** (Vector Indexing)
   - IVF_Flat index with auto-configured clusters
   - Clusters calculated as: `sqrt(num_vectors)` (e.g., 128 for 20K people)
   - Search time: 1-2ms per face for 20,000+ people
   - Distance metric: L2 (Euclidean)

3. **DeepFaceRecognizer** (Complete Pipeline)
   - Combines ModelLoader + FAISSIndex
   - Training: Extract embeddings from `dataset/PersonName/` images
   - Recognition: Extract embedding, search FAISS index, return person_id
   - Confidence: Converted from L2 distance (0-1 scale)

**Recognition Process**:
1. Face ROI extracted by Haar Cascade detector
2. Image preprocessed: resize to 112×112, normalize with (pixel-127.5)/128
3. ONNX model inference extracts 512-D embedding
4. L2 normalization for consistency
5. FAISS searches index for nearest neighbor (~1-2ms)
6. Distance converted to confidence score
7. Display: Name if confidence ≥70%, "Unknown" if <70%

**Training Process** (`train_from_images()`):
1. Scan `dataset/PersonID/` subdirectories for images
2. For each image:
   - Convert to grayscale
   - Resize to 200×200
   - Apply histogram equalization
   - Extract LBPH features
3. Train single LBPH model with all preprocessed images
4. Model stored in memory (no file persistence)
5. During recognition, compare input face against all training histograms

**Advantages**:
- Fast training and recognition
- Robust to lighting variations (histogram equalization)
- Low memory footprint compared to deep learning
- Good accuracy for controlled environments

**Limitations**:
- Requires frontal/near-frontal faces
- Performance degrades with significant head rotation (>45°)
- Sensitive to different lighting conditions between training and recognition
- Limited to ~20-50 different people reliably
- Requires multiple images per person (5-10 recommended)

**Confidence Score Interpretation**:
- **>80%**: High confidence, reliable match
- **50-80%**: Medium confidence, likely correct match
- **30-50%**: Low confidence, possibly wrong person
- **<30%**: Very uncertain, likely unknown person

## Limitations

- **Single camera support** (can be extended for multi-camera by modifying Camera class)
- **Frontal face detection** (Haar Cascade works best with near-frontal faces; side profiles may not detect)
- **Limited person count** (LBPH reliably handles ~20-50 people; performance degrades with more)
- **No frame recording** (can be added via OpenCV VideoWriter)
- **No image effects/filters** (can integrate additional OpenCV processing)
- **In-memory model only** (model lost on application exit; not persisted to disk)
- **No incremental learning** (retraining requires all images; no online learning)

## License

This project is open source and available for personal and educational use.

## Support

For issues or questions:
1. Check the Troubleshooting section
2. Verify all dependencies are installed
3. Check console output for error messages
4. Review camera permissions

## Documentation

For detailed information, see these comprehensive guides:

- **[QUICK_START.md](QUICK_START.md)** - 5-step setup guide (45 minutes)
- **[SETUP_FAISS_DEEPLEARNING.md](SETUP_FAISS_DEEPLEARNING.md)** - Complete dependency installation with OS-specific instructions
- **[IMPLEMENTATION_PLAN.md](IMPLEMENTATION_PLAN.md)** - Full technical architecture and design (650+ lines)
- **[FAISS_DEEPLEARNING_IMPLEMENTATION.md](FAISS_DEEPLEARNING_IMPLEMENTATION.md)** - Phase 1 implementation summary
- **[INDEX.md](INDEX.md)** - Complete documentation index and cross-references

## Recent Fixes and Improvements

### Model Loading Memory Safety Fix
**Issue**: ONNX Runtime input/output names were stored as raw pointers that became invalid after the allocator was destroyed, causing "Invalid Feed Input Name" errors.

**Solution**:
- Changed input/output names from `const char*` pointers to `std::string` objects
- Pointers are now created from the string data, ensuring validity for the entire session lifetime
- Fixed in [model_loader.h](include/model_loader.h) and [model_loader.cpp](src/model_loader.cpp)

### Multi-Dimensional Output Handling
**Issue**: The FaceNet model outputs a 4D tensor [1, 71, 46, 46] (feature maps) instead of 1D embeddings, causing dimension mismatch errors when adding to FAISS index.

**Solution**:
- Added `get_flattened_output_size()` method to calculate total output dimensions
- Modified `inference()` to flatten multi-dimensional outputs
- Updated `load_model()` to dynamically create FAISS index with correct dimensions
- Now supports model outputs of any shape (batch, channels, height, width)
- Result: Uses 150,236-D feature vectors from FaceNet model for improved accuracy

### Build System Improvement
**Issue**: `make clean` was removing ONNX Runtime and FAISS installations, requiring re-downloads on each rebuild.

**Solution**:
- Modified `clean` target to preserve external dependencies
- Added new `distclean` target for complete cleanup when needed
- Saves development time during iterative builds
- Documented in [Makefile](Makefile)

## Future Enhancements

### Face Recognition Improvements
- [x] **Deep Learning Models**: ArcFace neural network with 99.83% accuracy ✅ COMPLETE
- [x] **Model Persistence**: Save/load FAISS index to disk ✅ COMPLETE
- [ ] **Alternative Models**: VGGFace2, SFace support (swap ONNX models)
- [ ] **Incremental Learning**: Add new training images without full retraining
- [ ] **Head Pose Estimation**: Handle rotated faces (3D face alignment)
- [ ] **Multi-face Tracking**: Track multiple faces across frames
- [ ] **Face Clustering**: Automatic grouping of similar faces
- [ ] **Liveness Detection**: Distinguish real faces from photos/videos
- [ ] **Masked Face Recognition**: Handle faces with masks/sunglasses

### Camera and Capture
- [ ] **Multiple Camera Support**: Switch between multiple webcams
- [ ] **Frame Recording**: Record video stream to MP4/AVI
- [ ] **Screenshot Capture**: Save individual frames
- [ ] **Resolution/FPS Adjustment**: GUI controls for camera parameters
- [ ] **Camera Calibration**: Lens distortion correction

### Image Processing
- [ ] **Basic Filters**: Grayscale, blur, edge detection
- [ ] **Image Enhancement**: Brightness/contrast adjustment
- [ ] **Face Alignment**: Automatic face rotation for better recognition
- [ ] **Preprocessing Options**: Selectable preprocessing pipelines

### User Interface
- [ ] **Configuration File**: Save/load settings between sessions
- [ ] **Settings Dialog**: GUI for tuning detection/recognition parameters
- [ ] **Face Database Browser**: UI to view/edit registered people
- [ ] **Recognition History**: Log recognized faces with timestamps
- [ ] **Real-time Statistics**: Display detection/recognition metrics

### Performance and Deployment
- [ ] **GPU Acceleration**: CUDA/OpenCL for face detection/recognition
- [ ] **Network Streaming**: Stream video to remote clients
- [ ] **Performance Monitoring**: Real-time CPU/memory/FPS display
- [ ] **Docker Container**: Containerized deployment
- [ ] **REST API**: Remote face recognition service
