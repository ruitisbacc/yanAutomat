# yanAutomat - DFA Editor

Interactive graphical editor for **Deterministic Finite Automata (DFA)** written in C using [raylib](https://www.raylib.com/).


## Features

- **Visual automaton editing** - add states, transitions with drag & drop
- **Real-time validation** - shows missing transitions and errors
- **String testing** - test if automaton accepts a given string
- **Minimization** - reduce automaton to minimal equivalent form
- **Complement** - create complement automaton
- **Table view** - display automaton as transition table

## Controls

| Action | Control |
|--------|---------|
| Add state | Select ADD tool + click on canvas |
| Add transition | Select TRN tool + drag between states |
| Toggle accepting | Right-click on state or press `A` |
| Set initial state | Select state + press `I` |
| Delete | Select DEL tool + click |
| Pan canvas | Left-click on empty space + drag / Middle mouse + drag |
| Zoom | Mouse wheel |

## Building

### Prerequisites

- GCC (MinGW on Windows)
- GNU Make
- [raylib 5.0](https://github.com/raysan5/raylib/releases/tag/5.0) for Windows (mingw-w64)

### Setup

1. Download raylib 5.0 for Windows (mingw-w64) from [releases](https://github.com/raysan5/raylib/releases/tag/5.0)
2. Extract to `lib/raylib/` directory
3. Run `make`

```bash
# Download raylib (or do it manually)
mkdir -p lib
cd lib
wget https://github.com/raysan5/raylib/releases/download/5.0/raylib-5.0_win64_mingw-w64.zip
unzip raylib-5.0_win64_mingw-w64.zip
mv raylib-5.0_win64_mingw-w64 raylib
cd ..

# Build
make

# Run
./automaty.exe
```

## Project Structure

```
automaty/
├── include/
│   ├── automaton.h    # Data structures for automaton
│   ├── canvas.h       # Canvas drawing and interaction
│   ├── ui.h           # UI components
│   └── algorithms.h   # Algorithms (minimize, complement, test)
├── src/
│   ├── main.c         # Application entry point
│   ├── automaton.c    # Automaton management
│   ├── canvas.c       # Canvas rendering
│   ├── ui.c           # UI implementation
│   └── algorithms.c   # Algorithm implementations
├── assets/
│   └── Inter.ttf      # Font file
├── Makefile
└── README.md
```

## Algorithms

### String Testing
Simulates the automaton on input string and checks if final state is accepting.

### Minimization
Uses partition refinement algorithm (simplified Hopcroft) to find minimal equivalent DFA.

### Complement
Creates new automaton where accepting and non-accepting states are swapped.

## Screenshots

*Add screenshots here*

## License

MIT License - see [LICENSE](LICENSE) file.

## Author

Created as a tool for learning automata theory.
