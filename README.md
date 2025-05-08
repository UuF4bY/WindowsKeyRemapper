
# WindowsKeyRemapper v7.3

![Screenshot of WindowsKeyRemapper UI](https://i.imgur.com/lgnE4Pl.png)

**WindowsKeyRemapper** is a single-source Win32 utility that lets you redefine up to **five** simultaneous key-to-key mappings on Windows, with:

- A simple grid UI of all QWERTY keys + Fn keys  
- A runtime **Toggle** hotkey to enable/disable remaps  
- A **Reset** button to clear all mappings  
- Persistent storage in `mappings.json`  

---

## Features

- **Up to 5 concurrent mappings**  
  Click a “source” key, then click a “target” key to swap them.  
- **Runtime toggle**  
  Choose a global hotkey (default: ▼) to pause/resume remapping.  
- **Reset button**  
  Clear every mapping in one click.  
- **Persistent storage**  
  Saves to and loads from `mappings.json` in the same folder.  
- **Low-level keyboard hook**  
  Captures with `WH_KEYBOARD_LL`, preserves Alt+Tab/Esc, and handles key-up/down/repeats correctly.  
- **Built-in console log**  
  See active mappings and status messages at runtime.  

---

## How It Works

1. **UI Layout**  
   - A button for each key on a standard QWERTY + F1–F12.  
   - Below: **Set Hotkey**, **Toggle**, **Reset**.  
   - Status bar + multi-line console.  

2. **Mapping Workflow**  
   1. Click the **source** key you want to remap (e.g. `Space`).  
   2. Click the **target** key you’d like it to send (e.g. `LAlt`).  
   3. You can define up to five; extras are refused with a “Max 5 reached” message.  

3. **Toggle Hotkey**  
   - Click **Set Hotkey**, press any key to assign your on/off toggle.  
   - Press this hotkey during runtime to enable/disable all remaps on the fly.  

4. **Persistence**  
   - Mappings are written to `mappings.json` on exit.  
   - On launch, the JSON is reloaded so your configuration carries over.  

---

## Quick Compile Guide

_No project files or solution required—just the single `WindowsKeyRemapper.cpp` and a modern C++ compiler._

### Using MSVC (Developer Command Prompt)

1. Open **Developer Command Prompt for VS 2019+**.  
2. Navigate to the folder containing `WindowsKeyRemapper.cpp`.  
3. Run:
   ```bat
   cl /EHsc /std:c++17 WindowsKeyRemapper.cpp /link user32.lib gdi32.lib comctl32.lib
````

4. This produces `WindowsKeyRemapper.exe` in the same folder.

### Using MinGW-w64 (Git Bash or CMD)

1. Install MinGW-w64 and ensure `g++` is on your `PATH`.
2. In a terminal, CD to where `WindowsKeyRemapper.cpp` lives.
3. Run:

   ```bash
   g++ -std=c++17 -municode WindowsKeyRemapper.cpp -o WindowsKeyRemapper.exe \
       -luser32 -lgdi32 -lcomctl32
   ```
4. You’ll get `WindowsKeyRemapper.exe`.

---

## Deployment

1. Copy **only**:

   * `WindowsKeyRemapper.exe`
   * (optional) your existing `mappings.json`
2. Run the EXE—your remaps and toggle hotkey are ready immediately.

---

## Usage Examples

* **Bind “Jump” to Left Alt**

  1. Click **Space** → then **LAlt**.
  2. Press your toggle key to enable; now Space sends LAlt.

* **Bind “Esc” to Right Alt**

  1. Click **Esc** → then **RAlt**.
  2. Toggle on/off as needed.

---

## License

This tool is released under the **GNU GPL v3.0**.
See [LICENSE](LICENSE) for details.

```

