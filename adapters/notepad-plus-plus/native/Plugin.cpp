#include <windows.h>
#include <shlobj.h>
#include <commdlg.h>
#include <wrl.h>
#include <filesystem>
#include <string>
#include <vector>
#include <memory>
#include <algorithm>
#include "PluginInterface.h"
#include "Docking.h"
#include "WebView2.h"

using Microsoft::WRL::Callback;
using Microsoft::WRL::ComPtr;
namespace fs = std::filesystem;
namespace {
HINSTANCE instance;
NppData npp;
FuncItem commands[5];
constexpr UINT refreshTimer = 1;
constexpr size_t maxDocument = 4 * 1024 * 1024;
#ifdef _WIN64
constexpr wchar_t buildArchitecture[] = L"x64";
#else
constexpr wchar_t buildArchitecture[] = L"x86";
#endif

std::wstring wide(const std::string& bytes, UINT cp = CP_UTF8) {
    if (bytes.empty()) return {};
    int count = MultiByteToWideChar(cp, 0, bytes.data(), static_cast<int>(bytes.size()), nullptr, 0);
    std::wstring out(count, L'\0');
    MultiByteToWideChar(cp, 0, bytes.data(), static_cast<int>(bytes.size()), out.data(), count);
    return out;
}
std::wstring quote(const std::wstring& value) {
    std::wstring result = L"\"";
    constexpr wchar_t hex[] = L"0123456789abcdef";
    for (wchar_t c : value) {
        if (c == L'\"' || c == L'\\') { result += L'\\'; result += c; }
        else if (c < 32 || c == 0x2028 || c == 0x2029) {
            result += L"\\u";
            for (int shift = 12; shift >= 0; shift -= 4) result += hex[(c >> shift) & 15];
        } else result += c;
    }
    return result + L"\"";
}
fs::path moduleDirectory() {
    std::vector<wchar_t> path(32768);
    GetModuleFileNameW(instance, path.data(), static_cast<DWORD>(path.size()));
    return fs::path(path.data()).parent_path();
}

struct Panel : std::enable_shared_from_this<Panel> {
    HWND window = nullptr;
    HWND errorLabel = nullptr;
    bool closed = false, ready = false, initializing = false;
    bool floating = false, exporting = false;
    unsigned exportSequence = 0;
    std::wstring exportPath;
    ComPtr<ICoreWebView2Controller> controller;
    ComPtr<ICoreWebView2> web;
    std::wstring mappedDirectory;
    HRESULT comResult = E_FAIL;
    void fail(const wchar_t* stage, HRESULT hr) {
        initializing = false;
        wchar_t text[640];
        swprintf_s(text, L"mdviewer: %s (0x%08X)\nInstall Microsoft Edge WebView2 Runtime and ensure the web folder is next to mdviewer.dll.\nUse Plugins > mdviewer > Refresh to retry.", stage, static_cast<unsigned>(hr));
        if (!closed && errorLabel) { SetWindowTextW(errorLabel, text); ShowWindow(errorLabel, SW_SHOW); }
    }
    void resize() {
        RECT r{}; GetClientRect(window, &r);
        if (controller) controller->put_Bounds(r);
        if (errorLabel) MoveWindow(errorLabel, 18, 18, std::max(1L, r.right-36), std::max(1L, r.bottom-36), TRUE);
    }
    void refresh(bool forExport = false) {
        if (closed || !ready || !web) return;
        if (exporting && !forExport) return;
        int view = 0;
        SendMessageW(npp._nppHandle, NPPM_GETCURRENTSCINTILLA, 0, reinterpret_cast<LPARAM>(&view));
        HWND editor = view == 1 ? npp._scintillaSecondHandle : npp._scintillaMainHandle;
        const auto length = SendMessageW(editor, SCI_GETLENGTH, 0, 0);
        std::wstring markdown;
        if (length < 0 || static_cast<size_t>(length) > maxDocument) {
            markdown = L"# Preview paused\nDocument exceeds the 4 MiB preview limit.";
        } else {
            std::string bytes(static_cast<size_t>(length) + 1, '\0');
            SendMessageW(editor, SCI_GETTEXT, bytes.size(), reinterpret_cast<LPARAM>(bytes.data()));
            bytes.resize(static_cast<size_t>(length));
            const auto codepage = static_cast<UINT>(SendMessageW(editor, SCI_GETCODEPAGE, 0, 0));
            markdown = wide(bytes, codepage == SC_CP_UTF8 ? CP_UTF8 : (codepage ? codepage : CP_ACP));
        }
        std::vector<wchar_t> path(32768);
        SendMessageW(npp._nppHandle, NPPM_GETFULLCURRENTPATH, path.size(), reinterpret_cast<LPARAM>(path.data()));
        fs::path file(path.data());
        // Local resource mapping is intentionally disabled in v0.1. An arbitrary folder
        // mapping would expose adjacent HTML/SVG and reparse points to the browser.
        const auto buffer = SendMessageW(npp._nppHandle, NPPM_GETCURRENTBUFFERID, 0, 0);
        std::wstring payload = L"{\"markdown\":" + quote(markdown) + L",\"title\":" + quote(file.filename().wstring())
            + L",\"key\":" + quote(std::to_wstring(buffer))
            + (forExport ? L",\"exportId\":" + std::to_wstring(exportSequence) : L"") + L"}";
        web->PostWebMessageAsJson(payload.c_str());
    }
    void finishExport(const wchar_t* message, bool failed = false) {
        exporting = false; exportPath.clear();
        if (!closed) {
            MessageBoxW(window, message, L"mdviewer PDF", MB_OK | (failed ? MB_ICONERROR : MB_ICONINFORMATION));
            refresh();
        }
    }
    void exportPdf() {
        if (!ready || !web || exporting) {
            MessageBoxW(window, L"Wait for the preview to finish loading or exporting, then try again.", L"mdviewer PDF", MB_OK); return;
        }
        ComPtr<ICoreWebView2_7> pdf;
        if (FAILED(web.As(&pdf))) { MessageBoxW(window, L"Update Microsoft Edge WebView2 Runtime to export PDF.", L"mdviewer PDF", MB_ICONERROR); return; }
        std::vector<wchar_t> current(32768), filename(32768);
        SendMessageW(npp._nppHandle, NPPM_GETFULLCURRENTPATH, current.size(), reinterpret_cast<LPARAM>(current.data()));
        auto name = fs::path(current.data()).filename().replace_extension(L".pdf").wstring();
        if (name.empty()) name = L"document.pdf";
        wcscpy_s(filename.data(), filename.size(), name.c_str());
        OPENFILENAMEW dialog{}; dialog.lStructSize = sizeof(dialog); dialog.hwndOwner = window;
        dialog.lpstrTitle = L"mdviewer - Save PDF";
        dialog.lpstrFilter = L"PDF document (*.pdf)\0*.pdf\0\0"; dialog.lpstrFile = filename.data();
        dialog.nMaxFile = static_cast<DWORD>(filename.size()); dialog.lpstrDefExt = L"pdf";
        dialog.Flags = OFN_EXPLORER | OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR;
        if (!GetSaveFileNameW(&dialog)) return;
        exportPath = filename.data(); exporting = true; ++exportSequence;
        refresh(true);
    }
    void writePdf() {
        ComPtr<ICoreWebView2_7> pdf;
        if (FAILED(web.As(&pdf))) { finishExport(L"PDF export is unavailable. Update WebView2 Runtime.", true); return; }
        const auto target = exportPath;
        const auto temporary = target + L".mdviewer-" + std::to_wstring(GetCurrentProcessId()) + L"-" + std::to_wstring(GetTickCount64()) + L".pdf";
        std::weak_ptr<Panel> weak = shared_from_this();
        HRESULT hr = pdf->PrintToPdf(temporary.c_str(), nullptr, Callback<ICoreWebView2PrintToPdfCompletedHandler>(
            [weak, target, temporary](HRESULT result, BOOL success)->HRESULT {
                const bool saved = SUCCEEDED(result) && success && MoveFileExW(temporary.c_str(), target.c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH);
                if (!saved) DeleteFileW(temporary.c_str());
                if (auto self = weak.lock()) self->finishExport(saved ? L"PDF saved successfully." : L"Could not save PDF. Check file permissions and whether the destination is open.", !saved);
                return S_OK;
            }).Get());
        if (FAILED(hr)) { DeleteFileW(temporary.c_str()); finishExport(L"Could not start PDF export.", true); }
    }
    void start() {
        if (closed || initializing || web) return;
        initializing = true;
        if (comResult == E_FAIL) comResult = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
        if (FAILED(comResult)) { fail(L"COM initialization failed", comResult); return; }
        PWSTR local = nullptr;
        HRESULT hr = SHGetKnownFolderPath(FOLDERID_LocalAppData, 0, nullptr, &local);
        if (FAILED(hr)) { fail(L"Cannot locate user data folder", hr); return; }
        fs::path userData = fs::path(local) / L"mdviewer" / L"WebView2";
        CoTaskMemFree(local);
        std::weak_ptr<Panel> weak = shared_from_this();
        hr = CreateCoreWebView2EnvironmentWithOptions(nullptr, userData.c_str(), nullptr,
            Callback<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler>(
            [weak](HRESULT result, ICoreWebView2Environment* environment)->HRESULT {
                auto self = weak.lock();
                if (!self || self->closed) return S_OK;
                if (FAILED(result) || !environment) { self->fail(L"WebView2 unavailable", result); return S_OK; }
                HRESULT created = environment->CreateCoreWebView2Controller(self->window,
                    Callback<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler>(
                    [weak](HRESULT status, ICoreWebView2Controller* controller)->HRESULT {
                        auto panel = weak.lock();
                        if (!panel || panel->closed) { if(controller) controller->Close(); return S_OK; }
                        if (FAILED(status) || !controller) { panel->fail(L"Cannot create preview", status); return S_OK; }
                        panel->controller = controller;
                        controller->get_CoreWebView2(&panel->web);
                        panel->configure();
                        return S_OK;
                    }).Get());
                if (FAILED(created)) self->fail(L"Cannot initialize browser controller", created);
                return S_OK;
            }).Get());
        if (FAILED(hr)) fail(L"Cannot start WebView2", hr);
    }
    void configure() {
        ComPtr<ICoreWebView2_3> mapping;
        HRESULT hr = web.As(&mapping);
        if (FAILED(hr)) { fail(L"WebView2 Runtime too old", hr); controller->Close(); web.Reset(); controller.Reset(); return; }
        const auto assets = moduleDirectory() / L"web";
        hr = mapping->SetVirtualHostNameToFolderMapping(L"mdviewer.local", assets.c_str(), COREWEBVIEW2_HOST_RESOURCE_ACCESS_KIND_DENY_CORS);
        if (FAILED(hr)) { fail(L"Cannot map preview assets", hr); return; }
        ComPtr<ICoreWebView2Settings> settings;
        web->get_Settings(&settings);
        settings->put_AreDevToolsEnabled(FALSE);
        settings->put_AreDefaultScriptDialogsEnabled(FALSE);
        settings->put_IsStatusBarEnabled(FALSE);
        settings->put_AreHostObjectsAllowed(FALSE);
        std::weak_ptr<Panel> weak = shared_from_this();
        EventRegistrationToken token{};
        web->add_NavigationStarting(Callback<ICoreWebView2NavigationStartingEventHandler>(
            [](ICoreWebView2*, ICoreWebView2NavigationStartingEventArgs* args)->HRESULT {
                LPWSTR uri = nullptr; args->get_Uri(&uri);
                const bool allow = uri && std::wstring(uri) == L"https://mdviewer.local/index.html";
                args->put_Cancel(!allow); CoTaskMemFree(uri); return S_OK;
            }).Get(), &token);
        web->add_NewWindowRequested(Callback<ICoreWebView2NewWindowRequestedEventHandler>(
            [](ICoreWebView2*, ICoreWebView2NewWindowRequestedEventArgs* args)->HRESULT { args->put_Handled(TRUE); return S_OK; }).Get(), &token);
        web->add_PermissionRequested(Callback<ICoreWebView2PermissionRequestedEventHandler>(
            [](ICoreWebView2*, ICoreWebView2PermissionRequestedEventArgs* args)->HRESULT { args->put_State(COREWEBVIEW2_PERMISSION_STATE_DENY); return S_OK; }).Get(), &token);
        ComPtr<ICoreWebView2_4> downloadWeb;
        web.As(&downloadWeb);
        if (downloadWeb) downloadWeb->add_DownloadStarting(Callback<ICoreWebView2DownloadStartingEventHandler>(
            [](ICoreWebView2*, ICoreWebView2DownloadStartingEventArgs* args)->HRESULT { args->put_Cancel(TRUE); return S_OK; }).Get(), &token);
        web->add_WebMessageReceived(Callback<ICoreWebView2WebMessageReceivedEventHandler>(
            [weak](ICoreWebView2*, ICoreWebView2WebMessageReceivedEventArgs* args)->HRESULT {
                auto self = weak.lock(); if (!self || self->closed) return S_OK;
                LPWSTR source = nullptr, message = nullptr;
                args->get_Source(&source); args->TryGetWebMessageAsString(&message);
                if (source && message && std::wstring(source) == L"https://mdviewer.local/index.html" && std::wstring(message) == L"ready") {
                    self->ready = true; self->initializing = false; ShowWindow(self->errorLabel, SW_HIDE); self->refresh();
                }
                if (source && message && std::wstring(source) == L"https://mdviewer.local/index.html" && self->exporting) {
                    if (std::wstring(message) == L"pdf-ready:" + std::to_wstring(self->exportSequence)) self->writePdf();
                    else if (std::wstring(message) == L"pdf-error:" + std::to_wstring(self->exportSequence)) self->finishExport(L"Preview rendering failed. PDF was not exported.", true);
                }
                CoTaskMemFree(source); CoTaskMemFree(message); return S_OK;
            }).Get(), &token);
        web->add_NavigationCompleted(Callback<ICoreWebView2NavigationCompletedEventHandler>(
            [weak](ICoreWebView2*, ICoreWebView2NavigationCompletedEventArgs* args)->HRESULT {
                auto self = weak.lock(); BOOL ok = FALSE; args->get_IsSuccess(&ok);
                if (self && !self->closed && !ok) self->fail(L"Preview assets could not be loaded", E_FAIL);
                return S_OK;
            }).Get(), &token);
        resize();
        controller->put_IsVisible(TRUE);
        web->Navigate(L"https://mdviewer.local/index.html");
    }
    void close() {
        closed = true; ready = false;
        if(window) KillTimer(window, refreshTimer);
        if(controller) controller->Close();
        web.Reset(); controller.Reset();
        if (SUCCEEDED(comResult)) CoUninitialize();
        comResult = E_FAIL;
    }
};
std::shared_ptr<Panel> panel;
std::shared_ptr<Panel> floatingPanel;
LRESULT CALLBACK windowProc(HWND window, UINT msg, WPARAM w, LPARAM l) {
    auto view = reinterpret_cast<Panel*>(GetWindowLongPtrW(window, GWLP_USERDATA));
    if (msg == WM_NCCREATE) {
        view = static_cast<Panel*>(reinterpret_cast<CREATESTRUCTW*>(l)->lpCreateParams);
        SetWindowLongPtrW(window, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(view));
    }
    // Each preview owns its controller; floating and docked views can coexist.
    switch (msg) {
    case WM_SIZE: if(view && view->window == window) view->resize(); break;
    case WM_SHOWWINDOW:
        if(view && view->window == window) {
            if (!view->floating) SendMessageW(npp._nppHandle, NPPM_SETMENUITEMCHECK, commands[0]._cmdID, w != 0);
            if(w) { if(view->controller) view->controller->put_IsVisible(TRUE); SetTimer(window, refreshTimer, 100, nullptr); }
            else if(view->controller) view->controller->put_IsVisible(FALSE);
        } break;
    case WM_TIMER:
        if(w == refreshTimer && view) { KillTimer(window, refreshTimer); view->refresh(); } return 0;
    case WM_CLOSE:
        if (view && view->floating) ShowWindow(window, SW_HIDE);
        else SendMessageW(npp._nppHandle, NPPM_DMMHIDE, 0, reinterpret_cast<LPARAM>(window));
        return 0;
    case WM_SETFOCUS: if(view && view->controller) view->controller->MoveFocus(COREWEBVIEW2_MOVE_FOCUS_REASON_PROGRAMMATIC); break;
    case WM_DESTROY: if(view && view->window == window) { view->close(); view->window = nullptr; } break;
    }
    return DefWindowProcW(window, msg, w, l);
}
void showPreview() {
    if (!panel) {
        panel = std::make_shared<Panel>();
        WNDCLASSW wc{}; wc.hInstance = instance; wc.lpfnWndProc = windowProc; wc.lpszClassName = L"mdviewer.panel"; wc.hCursor = LoadCursor(nullptr, IDC_ARROW); wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW+1);
        RegisterClassW(&wc);
        panel->window = CreateWindowExW(WS_EX_CONTROLPARENT, wc.lpszClassName, L"mdviewer", WS_CHILD | WS_CLIPCHILDREN, 0, 0, 600, 700, npp._nppHandle, nullptr, instance, panel.get());
        if(!panel->window) { panel.reset(); MessageBoxW(npp._nppHandle,L"Cannot create mdviewer panel.",L"mdviewer",MB_ICONERROR); return; }
        panel->errorLabel = CreateWindowW(L"STATIC", L"Starting Markdown preview…", WS_CHILD | WS_VISIBLE, 18, 18, 550, 150, panel->window, nullptr, instance, nullptr);
        DockedWidgetData data{}; data.hClient = panel->window; data.pszName = L"mdviewer"; data.dlgID = 0; data.uMask = DWS_DF_CONT_RIGHT; data.pszModuleName = L"mdviewer.dll";
        SendMessageW(npp._nppHandle, NPPM_DMMREGASDCKDLG, 0, reinterpret_cast<LPARAM>(&data));
        panel->start();
    }
    SendMessageW(npp._nppHandle, NPPM_DMMSHOW, 0, reinterpret_cast<LPARAM>(panel->window));
    panel->refresh();
}
void refreshPreview() { showPreview(); if(panel) { panel->start(); panel->refresh(); } }
void showFloatingPreview() {
    if (!floatingPanel) {
        WNDCLASSW wc{}; wc.hInstance = instance; wc.lpfnWndProc = windowProc; wc.lpszClassName = L"mdviewer.panel"; wc.hCursor = LoadCursor(nullptr, IDC_ARROW); wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW+1);
        RegisterClassW(&wc);
        floatingPanel = std::make_shared<Panel>(); floatingPanel->floating = true;
        auto& view = floatingPanel;
        view->window = CreateWindowExW(WS_EX_CONTROLPARENT, wc.lpszClassName, L"mdviewer - Floating preview", WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN, CW_USEDEFAULT, CW_USEDEFAULT, 800, 750, npp._nppHandle, nullptr, instance, view.get());
        if (!view->window) { floatingPanel.reset(); return; }
        view->errorLabel = CreateWindowW(L"STATIC", L"Starting Markdown preview...", WS_CHILD | WS_VISIBLE, 18, 18, 550, 150, view->window, nullptr, instance, nullptr);
        view->start();
    }
    ShowWindow(floatingPanel->window, SW_SHOWNORMAL); SetForegroundWindow(floatingPanel->window); floatingPanel->refresh();
}
void exportPdf() {
    auto view = floatingPanel && IsWindowVisible(floatingPanel->window) ? floatingPanel : panel;
    if (!view) { showPreview(); view = panel; }
    if (view) view->exportPdf();
}
void about() { const auto text = std::wstring(L"mdviewer 0.1.0 (") + buildArchitecture + L")\nOffline Markdown preview\nCommonMark, tables, tasks, math, Mermaid and syntax highlighting.\nGPL-3.0-or-later"; MessageBoxW(npp._nppHandle, text.c_str(), L"About mdviewer", MB_OK); }
}

BOOL APIENTRY DllMain(HMODULE module, DWORD reason, LPVOID) {
    if(reason == DLL_PROCESS_ATTACH) { instance = module; DisableThreadLibraryCalls(module); }
    return TRUE;
}
extern "C" __declspec(dllexport) void setInfo(NppData data) { npp = data; }
extern "C" __declspec(dllexport) const wchar_t* getName() { return L"mdviewer"; }
extern "C" __declspec(dllexport) BOOL isUnicode() { return TRUE; }
extern "C" __declspec(dllexport) FuncItem* getFuncsArray(int* count) {
    wcscpy_s(commands[0]._itemName, L"Preview"); commands[0]._pFunc = showPreview;
    wcscpy_s(commands[1]._itemName, L"Refresh"); commands[1]._pFunc = refreshPreview;
    wcscpy_s(commands[2]._itemName, L"About"); commands[2]._pFunc = about;
    wcscpy_s(commands[3]._itemName, L"Floating preview"); commands[3]._pFunc = showFloatingPreview;
    wcscpy_s(commands[4]._itemName, L"Export PDF..."); commands[4]._pFunc = exportPdf;
    *count = 5; return commands;
}
extern "C" __declspec(dllexport) void beNotified(SCNotification* event) {
    if (!event) return;
    if (event->nmhdr.code == NPPN_SHUTDOWN) {
        if(panel) { panel->close(); if(panel->window) DestroyWindow(panel->window); panel.reset(); }
        if(floatingPanel) { floatingPanel->close(); if(floatingPanel->window) DestroyWindow(floatingPanel->window); floatingPanel.reset(); }
    } else {
        const auto code = event->nmhdr.code;
        if(code == NPPN_BUFFERACTIVATED || code == NPPN_FILESAVED ||
            (code == SCN_MODIFIED && (event->modificationType & (SC_MOD_INSERTTEXT | SC_MOD_DELETETEXT)))) {
            for (const auto& view : {panel, floatingPanel})
                if (view && view->window && IsWindowVisible(view->window)) SetTimer(view->window, refreshTimer, 180, nullptr);
        }
    }
}
extern "C" __declspec(dllexport) LRESULT messageProc(UINT, WPARAM, LPARAM) { return TRUE; }
