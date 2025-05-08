
# WindowsKeyRemapper v7.3

![Screenshot of WindowsKeyRemapper UI](https://i.imgur.com/lgnE4Pl.png)

**WindowsKeyRemapper** is a small Win32-based utility that lets you redefine up to **five** simultaneous key-to-key mappings on Windows. It provides a simple visual UI, a toggle hotkey to enable/disable remapping at runtime, and a reset button to clear all mappings. Mappings persist across sessions in a JSON file (`mappings.json`).

---

## Features

* **Up to 5 concurrent mappings**
  Click a “source” key button, then click a “target” key button to swap them.
* **Runtime toggle**
  Define a dedicated toggle key (default: ▼) to quickly enable/disable all remaps.
* **Reset button**
  Clear all current mappings with a single click.
* **Persistent storage**
  All mappings are saved in `mappings.json` on exit and reloaded on startup.
* **Low-level hook with proper repeat handling**
  Uses a `WH_KEYBOARD_LL` hook to capture key events, preserve Alt+Tab/Esc, and support key-up/down correctly.
* **Verbose console log**
  Displays active remaps and status messages in a built-in console panel.

---

## How It Works

1. **UI Layout**

   * A grid of buttons represents every key on a standard QWERTY keyboard plus function keys.
   * “Reset”, “Toggle” and “Set Hotkey” buttons sit below.
   * A status bar and multi-line console show current mappings and log messages.

2. **Mapping Workflow**

   * **Select source**: click the button of the key you wish to remap (e.g. `Space`).
   * **Select target**: click the button of the key you want to send instead (e.g. `LAlt`).
   * Up to **five** mappings can be defined. Extra attempts will show “Max 5 reached.”

3. **Toggle Hotkey**

   * Click **Set Hotkey**, then press any key to assign it as your global on/off toggle.
   * While running, press your toggle key to pause/resume all remaps.

4. **Persistence**

   * Mappings are serialized to `mappings.json` in the same directory.
   * On next launch, the program reloads your last configuration.

---

## Building

1. **Requirements**

   * Windows 7 or later
   * Visual Studio 2019+ (MSVC toolchain)
   * Windows SDK (for `<windows.h>`, `<commctrl.h>`)

2. **Steps**

   ```bat
   git clone https://github.com/yourusername/WindowsKeyRemapper.git
   cd WindowsKeyRemapper
   rem Build the `WindowsKeyRemapper` Visual Studio solution in Release x64
   devenv WindowsKeyRemapper.sln /Build Release /Project WindowsKeyRemapper
   ```

3. **Deploy**

   * Copy `WindowsKeyRemapper.exe` alongside `mappings.json` (will be auto-created).
   * Run the EXE; remappings activate immediately.

---

## Usage Examples

* **Bind “Jump” to Left Alt**

  1. Click the **Space** button.
  2. Click the **LAlt** button.
  3. Now pressing Space will send Left Alt.

* **Bind “Esc” to Right Alt**

  1. Click **Esc** → then **RAlt**.
  2. Press your toggle key to pause/resume remapping as needed.

---

## Contributing

1. Fork the repository
2. Create a feature branch (`git checkout -b feat/awesome`)
3. Commit your changes (`git commit -am 'Add awesome feature'`)
4. Push to the branch (`git push origin feat/awesome`)
5. Open a PR against `main`

---

## License

This project is released under the **GNU General Public License v3.0**.
See [LICENSE](LICENSE) for full details.
