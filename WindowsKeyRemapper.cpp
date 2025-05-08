// WindowsKeyRemapper.cpp — Version 7.3 

#include <windows.h>
#include <commctrl.h>
#include <vector>
#include <unordered_map>
#include <fstream>

#pragma comment(lib, "Comctl32.lib")

static const wchar_t APP_TITLE[] = L"WindowsKeyRemapper v7.3";
static const wchar_t MAPPINGS_FILE[] = L"mappings.json";

constexpr int ID_BTN_BASE = 1000;
constexpr int ID_RESET = 2000;
constexpr int ID_TOGGLE_UI = 2001;
constexpr int ID_SET_HOTKEY = 2002;

HINSTANCE                     hInst;
HWND                          hWnd, hStatus, hConsole;
HFONT                         hKeyFont, hConsoleFont;
UINT                          sourceVk = 0;
bool                          enabled = true;
UINT                          toggleVk = VK_DOWN;
bool                          pendingHotkey = false;
std::unordered_map<UINT, UINT> keyMap;

// Full on-screen labels & VK codes
static const std::vector<std::vector<const wchar_t*>> keyLabels = {
    { L"Esc",  L"F1",  L"F2",  L"F3",  L"F4",  L"F5",  L"F6",  L"F7",  L"F8",  L"F9",  L"F10", L"F11", L"F12" },
    { L"`",    L"1",   L"2",   L"3",   L"4",   L"5",   L"6",   L"7",   L"8",   L"9",   L"0",   L"-",   L"=",   L"Bksp" },
    { L"Tab",  L"Q",   L"W",   L"E",   L"R",   L"T",   L"Y",   L"U",   L"I",   L"O",   L"P",   L"[",   L"]",   L"\\"   },
    { L"Caps", L"A",   L"S",   L"D",   L"F",   L"G",   L"H",   L"J",   L"K",   L"L",   L";",   L"'",   L"Enter" },
    { L"Shift",L"Z",   L"X",   L"C",   L"V",   L"B",   L"N",   L"M",   L",",   L".",   L"/",   L"Shift" },
    { L"Ctrl", L"Win", L"LAlt",L"Space",L"RAlt", L"Menu", L"Ctrl" }
};
static const std::vector<std::vector<UINT>> keyCodes = {
    { VK_ESCAPE, VK_F1,  VK_F2,  VK_F3,  VK_F4,  VK_F5,  VK_F6,  VK_F7,  VK_F8,  VK_F9,  VK_F10, VK_F11, VK_F12 },
    { VK_OEM_3,  '1',    '2',    '3',    '4',    '5',    '6',    '7',    '8',    '9',    '0',    VK_OEM_MINUS, VK_OEM_PLUS, VK_BACK },
    { VK_TAB,    'Q',    'W',    'E',    'R',    'T',    'Y',    'U',    'I',    'O',    'P',    VK_OEM_4,      VK_OEM_6,      VK_OEM_5 },
    { VK_CAPITAL,'A',    'S',    'D',    'F',    'G',    'H',    'J',    'K',    'L',    VK_OEM_1,      VK_OEM_7,      VK_RETURN },
    { VK_LSHIFT,'Z',     'X',    'C',    'V',    'B',    'N',    'M',    VK_OEM_COMMA,  VK_OEM_PERIOD, VK_OEM_2,      VK_RSHIFT },
    { VK_LCONTROL, VK_LWIN, VK_LMENU, VK_SPACE, VK_RMENU, VK_APPS, VK_RCONTROL }
};

// Persistence
void LoadMappings() {
    std::wifstream in(MAPPINGS_FILE);
    if (!in.is_open()) return;
    wchar_t ch; in >> ch; // '{'
    while (in >> std::ws && in.peek() != L'}') {
        in >> ch; int s; in >> s;
        in >> ch >> ch; // "\":"
        int d; in >> d;
        in >> ch; // ',' or '}'
        keyMap[(UINT)s] = (UINT)d;
    }
}
void SaveMappings() {
    std::wofstream out(MAPPINGS_FILE, std::ios::trunc);
    out << L"{\n";
    bool first = true;
    for (auto& p : keyMap) {
        if (!first) out << L",\n";
        first = false;
        out << L"  \"" << p.first << L"\": " << p.second;
    }
    out << L"\n}\n";
}

// Verbose console
void RefreshConsole() {
    std::wstring txt = enabled
        ? L"[ON] Active remaps:\r\n"
        : L"[OFF] Remapping paused\r\n";
    for (auto& p : keyMap) {
        wchar_t buf[64];
        swprintf_s(buf, L"  %u → %u\r\n", p.first, p.second);
        txt += buf;
    }
    SetWindowTextW(hConsole, txt.c_str());
}
void Log(const wchar_t* msg) {
    SetWindowTextW(hStatus, msg);
    RefreshConsole();
}

// Low-level hook preserving Alt+Tab/Esc and proper repeat
LRESULT CALLBACK LowLevelKeyboardProc(int n, WPARAM wParam, LPARAM lParam) {
    if (n == HC_ACTION) {
        auto* kb = reinterpret_cast<KBDLLHOOKSTRUCT*>(lParam);
        UINT vk = kb->vkCode;

        // 1) Preserve Alt+Tab and Alt+Esc
        if ((vk == VK_TAB || vk == VK_ESCAPE) && (GetAsyncKeyState(VK_MENU) & 0x8000)) {
            return CallNextHookEx(nullptr, n, wParam, lParam);
        }

        // 2) Ctrl+C exit
        if ((wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN) &&
            vk == 'C' && (GetAsyncKeyState(VK_CONTROL) & 0x8000)) {
            PostMessageW(hWnd, WM_CLOSE, 0, 0);
            return 1;
        }

        // 3) Capture new hotkey
        if ((wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN) && pendingHotkey) {
            toggleVk = vk;
            pendingHotkey = false;
            wchar_t buf[64];
            swprintf_s(buf, L"Toggle hotkey set to %u", vk);
            Log(buf);
            return 1;
        }

        // 4) Toggle hotkey pressed?
        if ((wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN) && vk == toggleVk) {
            enabled = !enabled;
            Log(enabled ? L"Remapping ENABLED" : L"Remapping DISABLED");
            return 1;
        }

        // 5) If disabled, pass through
        if (!enabled)
            return CallNextHookEx(nullptr, n, wParam, lParam);

        // 6) Remap on both down & up
        auto it = keyMap.find(vk);
        if (it != keyMap.end()) {
            INPUT in = { INPUT_KEYBOARD };
            in.ki.wVk = (WORD)it->second;
            if (wParam == WM_KEYUP || wParam == WM_SYSKEYUP) {
                in.ki.dwFlags = KEYEVENTF_KEYUP;
            }
            SendInput(1, &in, sizeof(in));
            return 1;
        }
    }
    return CallNextHookEx(nullptr, n, wParam, lParam);
}

// Build UI (dark theme + fonts)
void BuildUI() {
    HBRUSH bg = CreateSolidBrush(RGB(30, 30, 30));
    SetClassLongPtrW(hWnd, GCLP_HBRBACKGROUND, reinterpret_cast<LONG_PTR>(bg));

    RECT rc; GetClientRect(hWnd, &rc);
    int W = rc.right - rc.left, gap = 6;
    int keyW = W / 14, keyH = 48;

    hKeyFont = CreateFontW(14, 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Segoe UI");
    hConsoleFont = CreateFontW(12, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, FIXED_PITCH | FF_MODERN, L"Consolas");

    int y = gap, id = ID_BTN_BASE;
    for (size_t r = 0; r < keyLabels.size(); ++r) {
        int cols = (int)keyLabels[r].size();
        int totalW = cols * (keyW + gap) - gap;
        int x = (W - totalW) / 2;
        for (int c = 0; c < cols; ++c) {
            HWND b = CreateWindowW(L"BUTTON", keyLabels[r][c],
                WS_CHILD | WS_VISIBLE, x, y, keyW, keyH,
                hWnd, HMENU(UINT_PTR(id++)), hInst, nullptr);
            SendMessageW(b, WM_SETFONT, (WPARAM)hKeyFont, TRUE);
            x += keyW + gap;
        }
        y += keyH + gap;
    }

    auto mkBtn = [&](LPCWSTR txt, int cmd, int px) {
        HWND b = CreateWindowW(L"BUTTON", txt,
            WS_CHILD | WS_VISIBLE, px, y, keyW * 2, keyH,
            hWnd, HMENU(UINT_PTR(cmd)), hInst, nullptr);
        SendMessageW(b, WM_SETFONT, (WPARAM)hKeyFont, TRUE);
        };
    mkBtn(L"Reset", ID_RESET, gap);
    mkBtn(L"Toggle", ID_TOGGLE_UI, gap * 2 + keyW * 2);
    mkBtn(L"Set Hotkey", ID_SET_HOTKEY, gap * 3 + keyW * 4);

    y += keyH + gap;
    hStatus = CreateWindowW(L"STATIC", nullptr,
        WS_CHILD | WS_VISIBLE, 0, rc.bottom - 30, W, 30, hWnd, nullptr, hInst, nullptr);
    SendMessageW(hStatus, WM_SETFONT, (WPARAM)hKeyFont, TRUE);

    hConsole = CreateWindowW(L"EDIT", nullptr,
        WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY,
        0, y, W, rc.bottom - y - 30, hWnd, nullptr, hInst, nullptr);
    SendMessageW(hConsole, WM_SETFONT, (WPARAM)hConsoleFont, TRUE);

    Log(L"Click source→target (max 5). ▼ toggles; Set Hotkey to change.");
    LoadMappings();
    RefreshConsole();
}

// Window procedure
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
    case WM_COMMAND: {
        int id = LOWORD(wp), total = 0;
        for (auto& r : keyCodes) total += (int)r.size();
        if (id >= ID_BTN_BASE && id < ID_BTN_BASE + total) {
            int idx = id - ID_BTN_BASE;
            for (auto& row : keyCodes) {
                if (idx < (int)row.size()) {
                    UINT vk = row[idx];
                    if (!sourceVk) {
                        sourceVk = vk;
                        wchar_t buf[64]; swprintf_s(buf, L"Source = %u", vk);
                        Log(buf);
                    }
                    else {
                        if (keyMap.size() < 5) {
                            keyMap[sourceVk] = vk; SaveMappings();
                            wchar_t buf[64]; swprintf_s(buf, L"Mapped %u→%u", sourceVk, vk);
                            Log(buf);
                        }
                        else Log(L"Max 5 reached.");
                        sourceVk = 0;
                    }
                    break;
                }
                idx -= (int)row.size();
            }
        }
        else if (id == ID_RESET) {
            keyMap.clear(); SaveMappings(); Log(L"Mappings reset.");
        }
        else if (id == ID_TOGGLE_UI) {
            enabled = !enabled; Log(enabled ? L"Remap ON" : L"Remap OFF");
        }
        else if (id == ID_SET_HOTKEY) {
            pendingHotkey = true; Log(L"Press key for new toggle…");
        }
        break;
    }
    case WM_DESTROY:
        PostQuitMessage(0); return 0;
    }
    return DefWindowProcW(hwnd, msg, wp, lp);
}

// WinMain
int WINAPI wWinMain(HINSTANCE hi, HINSTANCE, LPWSTR, int cmd) {
    hInst = hi;
    INITCOMMONCONTROLSEX ic{ sizeof(ic),ICC_STANDARD_CLASSES };
    InitCommonControlsEx(&ic);

    WNDCLASSEXW wc{ sizeof(wc) };
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInst;
    wc.hCursor = LoadCursorW(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszClassName = L"WinKeyRemap";
    RegisterClassExW(&wc);

    hWnd = CreateWindowW(wc.lpszClassName, APP_TITLE, WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT, 1100, 700, NULL, NULL, hInst, NULL);

    SetWindowsHookExW(WH_KEYBOARD_LL, LowLevelKeyboardProc, NULL, 0);

    ShowWindow(hWnd, cmd);
    UpdateWindow(hWnd);
    BuildUI();

    MSG msg;
    while (GetMessageW(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    return 0;
}
