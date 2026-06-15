# ProjectPulse

A polished C++17 command-line task manager built with a clean class-based design.

## Features
- Add, edit, complete, delete, and search tasks
- Persistent storage using a simple text database
- Filter by status and sort by due date or priority
- Human-readable, modular code layout
- No external dependencies

## Build
```bash
cmake -S . -B build
cmake --build build
```

## Run
```bash
./build/project_pulse
```

On Windows, the executable will be in the `build` folder.

## Storage
The app stores tasks in `data/tasks.db` next to the executable working directory. If the file does not exist, it is created automatically.

## Notes
The project is intentionally written in a professional style with clear comments, separation of concerns, and basic validation.