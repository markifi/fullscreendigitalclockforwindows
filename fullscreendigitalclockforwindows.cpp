#include <windows.h>
#include <string>
#include <ctime>
#include <fstream>
#include <cstdlib>
#include <cstring>

#define WM_APP_RELOAD_CONFIG (WM_APP + 1)

struct Config {
    std::wstring fontName = L"Atkinson Hyperlegible Mono Light";
    int fontSize = 115;
    std::string color = "FF0000";
    std::string bgColor = "000000";
    std::wstring alignH = L"center";
    std::wstring alignV = L"middle";
    std::wstring timeFormat = L"%H:%M:%S";
    int padRight = 25;
    int padVertical = 25;
};

Config g_config;
HFONT g_hFont = nullptr;
std::wstring g_lastText;

// Internal helper to convert hex string to Windows GDI color format on-the-fly
inline COLORREF HexToColor(const std::string& hex) {
    unsigned int r = 0, g = 0, b = 0;
    if (hex.length() >= 6) {
        sscanf_s(hex.c_str(), "%02x%02x%02x", &r, &g, &b);
    }
    return RGB(r, g, b);
}

std::wstring GetConfigPath() {
    if (const char* appdata = std::getenv("APPDATA")) {
        std::wstring path = std::wstring(appdata, appdata + std::strlen(appdata)) + L"\\justabigclock";
        CreateDirectoryW(path.c_str(), nullptr);
        return path + L"\\justabigclock.conf";
    }
    return L"";
}

void CreateDefaultConfigIfNotExists(const std::wstring& path) {
    if (GetFileAttributesW(path.c_str()) == INVALID_FILE_ATTRIBUTES) {
        if (std::ofstream out(path.c_str()); out.is_open()) {
            out << "# justabigclock configuration file\n"
                   "#\n"
                   "# format also accepts ISO8601 if you want that\n"
                   "#\n"
                   "font-name=Atkinson Hyperlegible Mono Light\n"
                   "font-size=115\n"
                   "color=FF0000\n"
                   "bg-color=000000\n"
                   "alignment-horizontal=center\n"
                   "alignment-vertical=middle\n"
                   "format=%H:%M:%S\n"
                   "pad-right=25\n"
                   "pad-vertical=25\n";
        }
    }
}

void LoadConfig() {
    std::wstring path = GetConfigPath();
    if (path.empty()) return;
    CreateDefaultConfigIfNotExists(path);

    std::ifstream file(path.c_str());
    std::string line;
    while (std::getline(file, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        
        size_t start = line.find_first_not_of(" \t");
        if (start == std::string::npos || line[start] == '#') continue;
        line = line.substr(start);

        size_t pos = line.find('=');
        if (pos == std::string::npos) continue;

        std::string k = line.substr(0, pos);
        std::string v = line.substr(pos + 1);

        k.erase(k.find_last_not_of(" \t") + 1);
        v.erase(0, v.find_first_not_of(" \t"));
        v.erase(v.find_last_not_of(" \t") + 1);

        try {
            if (k == "font-size") g_config.fontSize = std::stoi(v);
            else if (k == "pad-right") g_config.padRight = std::stoi(v);
            else if (k == "pad-vertical") g_config.padVertical = std::stoi(v);
        } catch (...) {
            // Ignore malformed numbers gracefully
        }

        if (k == "color") g_config.color = v;
        else if (k == "bg-color") g_config.bgColor = v;
        else if (k == "font-name") g_config.fontName = {v.begin(), v.end()};
        else if (k == "alignment-horizontal") g_config.alignH = {v.begin(), v.end()};
        else if (k == "alignment-vertical") g_config.alignV = {v.begin(), v.end()};
        else if (k == "format") g_config.timeFormat = {v.begin(), v.end()};
    }
}

std::wstring GetTimeString() {
    time_t now = time(nullptr);
    tm tm_local;
    localtime_s(&tm_local, &now);

    std::wstring fmt = (g_config.timeFormat == L"ISO8601") ? L"%Y-%m-%dT%H:%M:%S" : g_config.timeFormat;
    wchar_t buf[64];
    wcsftime(buf, ARRAYSIZE(buf), fmt.c_str(), &tm_local);
    return buf;
}

void RecreateFont(HWND hwnd) {
    if (g_hFont) {
        DeleteObject(g_hFont);
        g_hFont = nullptr;
    }
    HDC hdc = GetDC(hwnd);
    int height = -MulDiv(g_config.fontSize, GetDeviceCaps(hdc, LOGPIXELSY), 72);
    ReleaseDC(hwnd, hdc);

    g_hFont = CreateFontW(height, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, g_config.fontName.c_str());
}

struct ThreadArgs { HWND hwnd; HANDLE hProcess; };
DWORD WINAPI WaitNotepadThread(LPVOID lpParam) {
    auto* args = (ThreadArgs*)lpParam;
    WaitForSingleObject(args->hProcess, INFINITE);
    CloseHandle(args->hProcess);
    PostMessageW(args->hwnd, WM_APP_RELOAD_CONFIG, 0, 0);
    delete args;
    return 0;
}

void OpenConfigInNotepad(HWND hwnd) {
    std::wstring path = GetConfigPath();
    if (path.empty()) return;
    std::wstring cmd = L"notepad.exe \"" + path + L"\"";

    STARTUPINFOW si = { sizeof(si) };
    PROCESS_INFORMATION pi = { 0 };
    if (CreateProcessW(nullptr, (LPWSTR)cmd.c_str(), nullptr, nullptr, FALSE, 0, nullptr, nullptr, &si, &pi)) {
        CloseHandle(pi.hThread);
        CreateThread(nullptr, 0, WaitNotepadThread, new ThreadArgs{hwnd, pi.hProcess}, 0, nullptr);
    }
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
case WM_CREATE:
        LoadConfig();
        RecreateFont(hwnd);
        SetTimer(hwnd, 1, 50, nullptr);
        
        // Enable the Move command in the Alt+Space system menu
        {
            HMENU hMenu = GetSystemMenu(hwnd, FALSE);
            if (hMenu) {
                EnableMenuItem(hMenu, SC_MOVE, MF_ENABLED | MF_BYCOMMAND);
            }
        }
        return 0;

    case WM_TIMER:
        if (wParam == 1) {
            std::wstring current = GetTimeString();
            if (current != g_lastText) {
                g_lastText = current;
                InvalidateRect(hwnd, nullptr, FALSE);
            }
        }
        return 0;

    case WM_APP_RELOAD_CONFIG:
        LoadConfig();
        RecreateFont(hwnd);
        g_lastText = GetTimeString();
        InvalidateRect(hwnd, nullptr, FALSE);
        return 0;

    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        RECT rc;
        GetClientRect(hwnd, &rc);

        HDC memDC = CreateCompatibleDC(hdc);
        HBITMAP memBitmap = CreateCompatibleBitmap(hdc, rc.right, rc.bottom);
        HBITMAP oldBitmap = (HBITMAP)SelectObject(memDC, memBitmap);

        HBRUSH bgBrush = CreateSolidBrush(HexToColor(g_config.bgColor));
        FillRect(memDC, &rc, bgBrush);
        DeleteObject(bgBrush);

        HFONT oldFont = (HFONT)SelectObject(memDC, g_hFont);
        SetTextColor(memDC, HexToColor(g_config.color));
        SetBkMode(memDC, TRANSPARENT);

        RECT textRect = rc;
        UINT dtFlags = DT_SINGLELINE;

        if (g_config.alignH == L"left") { dtFlags |= DT_LEFT; textRect.left += g_config.padRight; }
        else if (g_config.alignH == L"center") { dtFlags |= DT_CENTER; }
        else { dtFlags |= DT_RIGHT; textRect.right -= g_config.padRight; }

        if (g_config.alignV == L"top") { dtFlags |= DT_TOP; textRect.top += g_config.padVertical; }
        else if (g_config.alignV == L"bottom") { dtFlags |= DT_BOTTOM; textRect.bottom -= g_config.padVertical; }
        else { dtFlags |= DT_VCENTER; }

        DrawTextW(memDC, g_lastText.c_str(), -1, &textRect, dtFlags);
        BitBlt(hdc, 0, 0, rc.right, rc.bottom, memDC, 0, 0, SRCCOPY);

        SelectObject(memDC, oldFont);
        SelectObject(memDC, oldBitmap);
        DeleteObject(memBitmap);
        DeleteDC(memDC);
        EndPaint(hwnd, &ps);
        return 0;
    }

case WM_SYSKEYDOWN:
        if (wParam == VK_SPACE) {
            HMENU hMenu = GetSystemMenu(hwnd, FALSE);
            if (hMenu) {
                POINT pt = { 20, 20 };
                ClientToScreen(hwnd, &pt);
                // Manually display the system menu since Windows suppresses it for borderless popup windows
                int cmd = TrackPopupMenu(hMenu, TPM_RETURNCMD | TPM_LEFTALIGN | TPM_TOPALIGN, pt.x, pt.y, 0, hwnd, nullptr);
                if (cmd != 0) {
                    PostMessage(hwnd, WM_SYSCOMMAND, cmd, 0);
                }
            }
            return 0;
        }
        break;

    case WM_KEYDOWN:
        if (wParam == VK_ESCAPE || wParam == 'Q') DestroyWindow(hwnd);
        else if (wParam == 'E') OpenConfigInNotepad(hwnd);
        return 0;

    case WM_DESTROY:
        KillTimer(hwnd, 1);
        if (g_hFont) {
            DeleteObject(g_hFont);
            g_hFont = nullptr;
        }
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int) {
    const wchar_t CLASS_NAME[] = L"FullScreenClockClass";
    WNDCLASSW wc = { CS_HREDRAW | CS_VREDRAW, WndProc, 0, 0, hInstance, nullptr, LoadCursor(nullptr, IDC_ARROW), nullptr, nullptr, CLASS_NAME };
    RegisterClassW(&wc);

HWND hwnd = CreateWindowExW(0, CLASS_NAME, L"Clock", WS_POPUP | WS_VISIBLE | WS_SYSMENU,
    0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN), nullptr, nullptr, hInstance, nullptr);

    if (!hwnd) return 0;
    g_lastText = GetTimeString();
    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return (int)msg.wParam;
}