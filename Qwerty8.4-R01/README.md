# Qwerty Input with LVGL 8.4

A virtual keyboard input application with Korean language support built with LVGL 8.4 (Light and Versatile Graphics Library), SDL2, and FreeType for TrueType font rendering.

## Features

- **Qwerty Keyboard Interface**: Virtual keyboard with full character input support
- **Korean Language Support**: Full Korean character composition and input
- **NotoSansKR Fonts**: Multiple font weights from assets directory for rich typography
- **FreeType Font Rendering**: Direct TrueType font rendering at runtime (no conversion needed)
- **Modern UI**: Built with LVGL 8.4 for smooth graphics
- **Compact Design**: Optimized 640×480 layout
- **Efficient Build System**: LVGL compiled once during setup, fast application builds

## Requirements

- **GCC compiler**
- **Make**
- **SDL2 development libraries** (libsdl2-dev)
- **FreeType development libraries** (libfreetype-dev)
- **LVGL v8.4** (automatically cloned and built by setup script)
- **Git** (for cloning LVGL)

## Quick Start

### 1. Run Setup Script

The setup script will install dependencies and build LVGL:

```bash
./setup.sh
```

This will:
- Check for and install required build tools (gcc, make, pkg-config)
- Install SDL2 and FreeType development libraries if missing
- Clone LVGL v8.4 into the project directory
- Build LVGL as a static library (`lvgl/build/liblvgl.a`)
- Verify lv_conf.h configuration

### 2. Build the Application

```bash
make
```

The build system is optimized for development:
- **Setup phase**: LVGL is compiled once into a static library
- **Application builds**: Only compile application sources (very fast)
- **No recompilation**: LVGL library is reused across builds

### 3. Run the Application

```bash
./qwerty
```

Or:
```bash
make run
```

## Usage

### Qwerty Keyboard Input

The application provides a virtual keyboard with full character input support in a compact 640×480 window layout.

### Language Support

- **English Mode**: Standard QWERTY keyboard layout
- **Korean Mode**: Full Korean language composition and input support with Hangul characters
- **Language Toggle**: Switch between English and Korean modes

### Controls

- **Character Keys**: Letters, numbers, and symbols for text input
- **← (Backspace)**: Delete last character (handles multi-byte UTF-8 properly)
- **Enter**: Show input result in popup dialog and clear text area
- **Space**: Insert space character
- **Clear**: Clear all text
- **Shift/Caps**: Modify input (language-dependent behavior)
- **Function buttons**: Color-coded for easy identification

### Button Color Scheme

The application uses color-coded buttons for easy identification:

- **🟠 Orange**: Function buttons (Clear)
- **🔵 Blue**: Action buttons (Enter)
- **⚪ Default**: Regular hex input keys (0-9, A-F)

This color scheme helps users quickly identify special function buttons versus regular input keys.

## Project Structure

```
Qwerty8.4/
├── main.c              # Main application with LVGL UI
├── qwerty.c            # Keyboard input logic with Korean support
├── qwerty.h            # Header file
├── lv_conf.h           # LVGL 8.4 configuration
├── Makefile            # Build configuration (application only)
├── setup.sh            # Environment setup script (builds LVGL 8.4)
├── assets/             # NotoSansKR TrueType font files
│   ├── NotoSansKR-Light.ttf
│   ├── NotoSansKR-Regular.ttf
│   ├── NotoSansKR-Medium.ttf
│   ├── NotoSansKR-SemiBold.ttf
│   ├── NotoSansKR-Bold.ttf
│   ├── NotoSansKR-ExtraBold.ttf
│   ├── NotoSansKR-ExtraLight.ttf
│   └── NotoSansKR-Thin.ttf
├── lvgl/               # LVGL 8.4 library (cloned and built by setup.sh)
│   └── build/
│       └── liblvgl.a   # Pre-built LVGL static library
├── .gitignore          # Git ignore rules
├── LICENSE
└── README.md           # This file
```

## Configuration

### LVGL Configuration (lv_conf.h)

The project includes a pre-configured `lv_conf.h` file with:
- **LVGL version**: v8.4
- **Color depth**: 32-bit (XRGB8888)
- **FreeType support**: Enabled (LV_USE_FREETYPE = 1)
- **Memory pool**: 256KB (for font rendering)
- **Font Manager**: Enabled for TrueType font support

### Display Resolution

Current resolution: **640×480** pixels

To change the display resolution, edit these constants in `main.c`:

```c
#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480
```

### Font Configuration

The application uses **NotoSansKR** fonts from the assets directory with **FreeType** for direct TrueType rendering:

- **Status Label (14px)**: NotoSansKR-Light.ttf
- **Text Area (20px)**: NotoSansKR-Regular.ttf
- **Keyboard Buttons (16px)**: NotoSansKR-Medium.ttf
- **Bold Status Text (20px)**: NotoSansKR-SemiBold.ttf

**Available Font Weights:**
- NotoSansKR-Thin.ttf
- NotoSansKR-ExtraLight.ttf
- NotoSansKR-Light.ttf
- NotoSansKR-Regular.ttf
- NotoSansKR-Medium.ttf
- NotoSansKR-SemiBold.ttf
- NotoSansKR-Bold.ttf
- NotoSansKR-ExtraBold.ttf
- NotoSansKR-Black.ttf

**Key Features:**
- **No font conversion needed!** FreeType renders `.ttf` files at runtime
- **Rich typography**: Multiple font weights available for visual hierarchy
- **Korean support**: Full support for Korean characters and composition
- **Scalable**: Fonts can be loaded at any size

## Recent Updates

### LVGL 8.4 Migration

**Latest Updates (v8.4):**
- **LVGL 8.4 API**: Updated all function calls to use LVGL 8.4 compatible APIs
- **NotoSansKR Fonts**: Replaced NanumGothicCoding with multiple NotoSansKR font weights
- **FreeType 8.4**: Uses modern lv_ft_font_init() API for font initialization
- **Improved Korean Support**: Full Hangul composition and character input
- **Fixed API Compatibility**:
  - `lv_scr_act()` instead of `lv_screen_active()`
  - `lv_btn_create()` instead of `lv_button_create()`
  - `lv_ft_font_init()` instead of `lv_freetype_font_create()`
  - `lv_obj_align()` instead of `lv_obj_center()`

**Build System Enhancements:**
- **Setup phase**: LVGL is compiled once into a static library
- **Application builds**: Only compile application sources (very fast)
- **No recompilation**: LVGL library is reused across builds
- **Clean separation**: Setup handles LVGL, Makefile handles application

## Building from Scratch

### Install Dependencies (Ubuntu/Debian)

```bash
sudo apt-get update
sudo apt-get install build-essential libsdl2-dev libfreetype-dev pkg-config git
```

### Setup and Build

```bash
./setup.sh  # Installs dependencies, clones and builds LVGL
make        # Builds the application
```

The setup script compiles LVGL once into a static library. Application builds are very fast afterward.

## Troubleshooting

### "LVGL library not found" Error

Run the setup script to build LVGL:
```bash
./setup.sh
```

### "SDL not found" Error

Install SDL2 development libraries:
```bash
sudo apt-get install libsdl2-dev
```

### "FreeType not found" Error

Install FreeType development libraries:
```bash
sudo apt-get install libfreetype-dev
```

### Font Display Issues

The application uses fonts from `assets/` directory. Verify:
1. `assets/NanumGothicCoding.ttf` exists
2. Font files are not corrupted

### Compilation Errors

1. Verify `lv_conf.h` exists in the project directory
2. Check that `lvgl/build/liblvgl.a` exists (run `./setup.sh` if missing)
3. Run `make clean` and then `make`
4. Check compiler flags support C11 standard

### Build System Issues

**If builds are slow:**
- Run `./setup.sh` first to build LVGL library
- Application builds should be very fast after setup
- Use `make clean-lvgl` to rebuild LVGL if needed

**If LVGL library is missing:**
- Run `./setup.sh` to build the LVGL static library
- Check that `lvgl/build/liblvgl.a` exists

## Development

### Button Size Customization

All buttons are **35×39px** (regular keys). To modify:

```c
// In create_key_button():
lv_obj_set_size(btn, width, 39);  // Change height here

// For individual buttons, change width parameter:
create_key_button(row, "A", callback, data, 38);  // Change width
```

### Button Color Customization

To change button colors, modify the styling in `create_gui()`:

```c
// Orange color for function buttons
lv_obj_set_style_bg_color(app_state.clear_button, lv_color_hex(0xFF8C00), 0);

// Blue color for action buttons  
lv_obj_set_style_bg_color(app_state.enter_button, lv_color_hex(0x0000FF), 0);
```

### Font Size Customization

Edit `init_fonts()` function in `main.c`:

```c
static lv_ft_info_t info_20;
info_20.name = "assets/NotoSansKR-Regular.ttf";
info_20.weight = 20;  // <- Change size here
info_20.style = FT_FONT_STYLE_NORMAL;
info_20.mem = NULL;
if (!lv_ft_font_init(&info_20)) {
    fprintf(stderr, "Error: Failed to load font\n");
}
app_state.korean_font_20 = info_20.font;
```

### Adding New Fonts

Place your `.ttf` font file in `assets/` directory and load it using the LVGL 8.4 FreeType API:

```c
static lv_ft_info_t info;
info.name = "assets/MyFont.ttf";
info.weight = 20;  // Font size in pixels
info.style = FT_FONT_STYLE_NORMAL;  // or FT_FONT_STYLE_ITALIC, FT_FONT_STYLE_BOLD
info.mem = NULL;
if (!lv_ft_font_init(&info)) {
    fprintf(stderr, "Error: Failed to load font\n");
}
lv_font_t *my_font = info.font;
```

### Modifying Hex Input Layout

Edit the `key_maps` array in `qwerty.c` to customize the hex input interface:

```c
KeyMap key_maps[16] = {
    {"0", "0"},  // {normal, shift}
    {"1", "1"},
    {"2", "2"},
    // ... hex keys 0-F
};
```

## Technical Details

### Technologies Used

- **LVGL**: v8.4 (Light and Versatile Graphics Library)
- **SDL2**: Display and input handling
- **FreeType**: TrueType font rendering with lv_ft_font_init() API
- **C11**: Programming language standard

### Architecture

```
User Input → LVGL Event → Qwerty Input Processing →
  Korean Character Composition → Text Display
```

### Character Handling

- **UTF-8 Support**: Full UTF-8 multi-byte character support
- **Korean Composition**: Proper Hangul character composition and decomposition
- **Backspace Handling**: Correctly handles multi-byte UTF-8 character deletion

### Memory Usage

- **LVGL pool**: 256KB (configured in lv_conf.h)
- **FreeType cache**: 256 glyphs (default)
- **Typical runtime**: ~10MB RAM

### Performance

- **Frame rate**: 30 FPS (SDL VSync)
- **Input latency**: < 5ms
- **Font rendering**: Hardware accelerated (SDL2)
- **Build time**: Very fast application builds (LVGL pre-compiled)

## License

See [LICENSE](LICENSE) file for details.

## Contributing

Contributions are welcome! Please feel free to submit issues or pull requests.

## Credits

- **LVGL**: https://lvgl.io/ - Light and Versatile Graphics Library
- **SDL2**: https://www.libsdl.org/ - Simple DirectMedia Layer
- **FreeType**: https://www.freetype.org/ - TrueType font rendering
- **NotoSansKR Font**: Google Fonts - Comprehensive Korean font support

## References

- [LVGL Documentation](https://docs.lvgl.io/)
- [LVGL v8.4 Documentation](https://docs.lvgl.io/8.4/)
- [LVGL FreeType Support](https://docs.lvgl.io/8.4/details/libs/freetype.html)
- [FreeType Documentation](https://www.freetype.org/freetype2/docs/documentation.html)
- [SDL2 Documentation](https://wiki.libsdl.org/)
- [NotoSansKR Font](https://fonts.google.com/noto/specimen/Noto+Sans+KR)
