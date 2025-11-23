# DocGen Architecture Documentation

## Overview
DocGen is a GTK3-based document management application with a clean, object-oriented design following modern C++ best practices. It provides section management with customizable headlines and levels, drag-and-drop reordering, set persistence with metadata, individual section deletion, and AsciiDoc/Markdown export with proper heading hierarchy and live HTML preview.

## Visual UML Diagram

See `doc/images/architecture.puml` for the latest PlantUML diagram. Render it with PlantUML to view the relationships, notes, and grouping visually.

![Architecture Diagram](images/architecture.png)

## Class Structure & Relationships

### MainWindow
- Manages the GTK application window and menu system
- Contains a `SectionManager` and a `TextViewer`
- Handles document title, preview (WebKitWebView), and menu actions
- Coordinates save/load, export, and UI updates

### SectionManager
- Manages a vector of `TextSection` objects
- Handles drag-and-drop reordering, set persistence, and document generation
- Notifies MainWindow of content changes

### TextSection
- Represents an individual section with header, headline, content, and type
- Manages its own GTK widgets and UI logic
- Notifies SectionManager on changes (headline, type, etc.)

### TextViewer
- Utility class for file I/O and existence checking

### Relationships
- `main` creates `MainWindow`
- `MainWindow` contains `SectionManager` and `TextViewer`
- `SectionManager` manages multiple `TextSection` objects
- `TextSection` notifies `SectionManager` on changes
- All UI classes use GTK3 widgets; preview uses WebKit2GTK

## File Structure

```
docgen/
├── app/include/
│   ├── main_window.h       # MainWindow class interface
│   ├── text_section.h      # TextSection class interface
│   ├── section_manager.h   # SectionManager class interface
│   └── text_viewer.h       # TextViewer class interface
├── app/src/
│   ├── main_window.cpp     # MainWindow implementation
│   ├── text_section.cpp    # TextSection implementation
│   ├── section_manager.cpp # SectionManager implementation
│   └── text_viewer.cpp     # TextViewer implementation
├── doc/images/
│   ├── architecture.puml   # PlantUML diagram
│   └── architecture.png    # Rendered diagram
```

## How to Update the Diagram
- Edit `doc/images/architecture.puml` to change the UML structure
- Render with PlantUML to generate `architecture.png`
- Update this documentation if class relationships change

---

For details on each class, see the grouped and commented header files in `app/include/`.
