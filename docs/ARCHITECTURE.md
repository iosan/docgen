# DocGen Architecture Documentation

## Overview
DocGen is a GTK3-based document management application with a clean, object-oriented design following modern C++ best practices. It provides section management, drag-and-drop reordering, set persistence, and AsciiDoc export capabilities.

## Class Structure

### 1. **MainWindow** (`include/main_window.h`, `src/main_window.cpp`)
**Responsibility:** Main application window and UI coordination

**Key Features:**
- Manages the GTK application window and menu system
- Creates and manages menu bar (File, Edit, About)
- Handles save/load of section sets (.txt format)
- Manages AsciiDoc document generation
- Window title updates to show current set file
- Prompts for unsaved changes

**Key Methods:**
- `MainWindow(GtkApplication* app)` - Constructor, initializes window and UI
- `show()` - Displays the window
- `createMenuBar()` - Creates File/Edit/About menus
- `createUI()` - Sets up order box, text container, and CSS styling
- `updateTitle()` - Updates window title with current set name
- `promptSaveIfNeeded()` - Checks for unsaved changes
- Static callbacks: `onAddSection()`, `onSaveSet()`, `onOpenSet()`, `onCreateDoc()`, `onClearAll()`, `onQuit()`, `onAbout()`

**Design Pattern:** Facade pattern - provides simplified interface to complex subsystems

---

### 2. **TextSection** (`include/text_section.h`, `src/text_section.cpp`)
**Responsibility:** Encapsulates individual text section with header and content

**Key Features:**
- Self-contained section with header input, display label, and text view
- Manages its own GTK widgets (container, entry, label, text_view, scrolled_window)
- Includes corresponding order button for drag-drop ordering
- Automatic header synchronization between input field and display/order labels

**Key Methods:**
- `TextSection(int position, const std::string& default_header)` - Creates section with position ID
- `setContent(const std::string& content)` - Sets text content
- `setHeader(const std::string& header)` - Updates header label and order button
- `show()` / `hide()` - Visibility control
- `getContainer()` / `getOrderButton()` - Access to GTK widgets

**Design Pattern:** Encapsulation - all section-related UI and state in one class

---

### 3. **SectionManager** (`include/section_manager.h`, `src/section_manager.cpp`)
**Responsibility:** Manages collection of text sections and drag-drop ordering

**Key Features:**
- Maintains vector of TextSection objects using smart pointers
- Handles main section (special section for primary file content)
- Implements real-time drag-and-drop reordering with visual feedback
- Synchronizes order box and text container during reordering
- Save/Load section sets with content preservation
- Generates AsciiDoc documents with proper formatting

**Key Methods:**
- `SectionManager(GtkWidget* text_container, GtkWidget* order_box)` - Initializes manager
- `addSection(const std::string& header, const std::string& content)` - Creates new section
- `clearAll()` - Removes all sections and resets state
- `saveToFile(const std::string& filepath)` - Saves sections in current order
- `loadFromFile(const std::string& filepath)` - Loads sections from file
- `generateAsciiDoc(const std::string& title)` - Creates AsciiDoc formatted document
- `getSectionsInOrder()` - Returns sections in display order
- `setupDragAndDrop()` - Configures drag-drop for order buttons
- Static drag-drop callbacks: `onDragBegin()`, `onDragEnd()`, `onDragMotion()`, `onDragLeave()`, `onOrderBoxDragMotion()`

**Design Pattern:** Manager/Collection pattern - centralized control of related objects

---

### 4. **TextViewer** (`include/text_viewer.h`, `src/text_viewer.cpp`)
**Responsibility:** File I/O operations

**Key Features:**
- Simple utility class for loading text files
- File existence checking
- Exception-based error handling

**Key Methods:**
- `loadFile(const std::string& filepath)` - Reads and returns file content
- `fileExists(const std::string& filepath)` - Checks if file exists

**Design Pattern:** Utility class - stateless helper for file operations

---

## File Structure

```
docgen/
├── include/
│   ├── main_window.h       # MainWindow class interface
│   ├── text_section.h      # TextSection class interface
│   ├── section_manager.h   # SectionManager class interface
│   └── text_viewer.h       # TextViewer class interface
├── src/
│   ├── main.cpp            # Application entry point (minimal)
│   ├── main_window.cpp     # MainWindow implementation
│   ├── text_section.cpp    # TextSection implementation
│   ├── section_manager.cpp # SectionManager implementation
│   ├── text_viewer.cpp     # TextViewer implementation
│   └── main_old.cpp        # Previous monolithic version (backup)
├── tests/
│   └── test_main.cpp       # Unit tests for TextViewer
└── CMakeLists.txt          # Build configuration

```

## Dependency Graph

```
main.cpp
  └── MainWindow
      ├── SectionManager
      │   └── TextSection (multiple instances)
      └── TextViewer
```

![UML Class Diagram](images/architecture.png)

## Key Design Improvements

### 1. **Separation of Concerns**
- UI logic (MainWindow) separated from business logic (SectionManager)
- Each section is self-contained (TextSection)
- File operations isolated (TextViewer)

### 2. **Memory Management**
- Smart pointers (`std::unique_ptr`) for automatic cleanup
- RAII principles - resources tied to object lifetime
- No manual memory management needed

### 3. **Encapsulation**
- Private members with public interfaces
- Implementation details hidden
- GTK widget lifecycle managed internally

### 4. **Maintainability**
- Single Responsibility Principle - each class has one clear purpose
- Easy to locate and modify specific functionality
- Reduced coupling between components

### 5. **Testability**
- Business logic (TextViewer) easily unit tested
- Clear interfaces make mocking possible
- Existing tests remain functional

## GTK Integration

The code maintains clean separation while working with C-style GTK:
- Static callback functions bridge GTK C API to C++ methods
- User data pointers pass `this` to callbacks
- `g_object_set_data()` stores context where needed

## Building

```bash
cd build
cmake ..
make
./docgen
```

## Running Tests

```bash
cd build
./tests
```

All 28 unit tests pass successfully, covering:
- TextViewer file operations (3 tests)
- TextSection functionality (7 tests)
- SectionManager operations (18 tests)

## Features

### Section Management
- Add sections with custom headers
- Edit section content in read-only view mode
- Clear all sections

### Drag-and-Drop Ordering
- Real-time reordering of sections
- Visual feedback during drag operations
- Position determined by mouse position (left/right half)
- Synchronized order box and text display

### Set Persistence
- Save section sets to .txt files
- Load existing section sets
- Custom format: `[SECTION:header]` ... `[END_SECTION]`
- Preserves content and ordering

### AsciiDoc Export
- Generate AsciiDoc documents from sections
- Automatic filename suggestion (.adoc extension)
- Document title from set filename
- Proper AsciiDoc formatting (= title, == sections)
- File overwrite confirmation
