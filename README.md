
# DocGen

A modern C++/GTK3 document generator with hierarchical section management, Markdown/AsciiDoc export, and live HTML preview.

## Features
- Hierarchical document structure (I, II, III headline levels)
- Modern GTK3 UI with green-toned headline levels
- Markdown and AsciiDoc export
- Live HTML preview (WebKit2GTK)
- Robust test suite (GoogleTest)
- Coverage reporting (lcov/genhtml)
- Easy dependency installation script

## Quick Start
1. **Install dependencies:**
   ```bash
   ./resources/install_deps.sh
   ```
2. **Build the project:**
   ```bash
   mkdir build && cd build
   cmake -DCMAKE_BUILD_TYPE=Coverage ..
   make
   ```
3. **Run tests:**
   ```bash
   make test
   ```
4. **Generate coverage report:**
   ```bash
   make coverage
   # Open build/coverage/index.html in your browser
   ```

## Project Structure
- `app/include/` — Core headers
- `app/src/` — Source files
- `resources/` — Scripts and assets
- `doc/` — Documentation and diagrams
- `lib/` — External libraries (see lib/README.md)
- `examples/` — Example documents
- `tests/` — Test suite

## Documentation
- Architecture: `doc/ARCHITECTURE.md`, diagrams in `doc/images/`
- Example usage: `examples/example_set1.md`

## Contributing
Pull requests are welcome! Please ensure all tests pass and coverage is above 70%.

## License
MIT

---
For more details, see the documentation in the `doc/` folder.
