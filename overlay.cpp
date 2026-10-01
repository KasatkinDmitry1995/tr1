// overlay.cpp
#include "overlay.h"

#include <d3d11.h>
#include <dxgi1_2.h>
#include <dcomp.h>
#include <wrl/client.h>

#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"

#include "menu.h"

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dcomp.lib")

OverlayData data;

using namespace Microsoft::WRL;

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND, UINT, WPARAM, LPARAM);

namespace {
    HWND  g_overlayWnd = nullptr;
    HWND  g_targetWnd = nullptr;
    int   g_cx = 0, g_cy = 0;

    ComPtr<ID3D11Device>            g_d3dDevice;
    ComPtr<ID3D11DeviceContext>     g_d3dContext;
    ComPtr<IDXGISwapChain1>         g_swapChain;
    ComPtr<IDCompositionDevice>     g_dcompDevice;
    ComPtr<IDCompositionTarget>     g_dcompTarget;
    ComPtr<IDCompositionVisual>     g_dcompVisual;
    ComPtr<ID3D11RenderTargetView>  g_renderTargetView;

    bool g_imguiInitialized = false;
    bool g_quitRequested = false;

    // Внутренние функции
    bool InitD3D11(HWND hwnd, int width, int height);
    bool InitComposition(HWND hwnd);
    bool CreateRTV();
    void UpdateOverlayPosition();
    ImU32 ToImU32(const OverlayColor& c);
}


// ---------- Внутренние ----------
namespace {
    ImU32 ToImU32(const OverlayColor& c) {
        return IM_COL32(c.r, c.g, c.b, c.a);
    }

    bool InitD3D11(HWND hwnd, int width, int height) {
        D3D_FEATURE_LEVEL featureLevel;
        HRESULT hr = D3D11CreateDevice(
            nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr,
            D3D11_CREATE_DEVICE_BGRA_SUPPORT,
            nullptr, 0, D3D11_SDK_VERSION,
            &g_d3dDevice, &featureLevel, &g_d3dContext);
        if (FAILED(hr)) return false;

        ComPtr<IDXGIDevice> dxgiDevice;
        hr = g_d3dDevice.As(&dxgiDevice);
        if (FAILED(hr)) return false;

        ComPtr<IDXGIAdapter> dxgiAdapter;
        hr = dxgiDevice->GetAdapter(&dxgiAdapter);
        if (FAILED(hr)) return false;

        ComPtr<IDXGIFactory2> dxgiFactory;
        hr = dxgiAdapter->GetParent(IID_PPV_ARGS(&dxgiFactory));
        if (FAILED(hr)) return false;

        DXGI_SWAP_CHAIN_DESC1 desc = {};
        desc.Width = width;
        desc.Height = height;
        desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
        desc.SampleDesc.Count = 1;
        desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        desc.BufferCount = 2;
        desc.Scaling = DXGI_SCALING_STRETCH;
        desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
        desc.AlphaMode = DXGI_ALPHA_MODE_PREMULTIPLIED;

        hr = dxgiFactory->CreateSwapChainForComposition(
            g_d3dDevice.Get(), &desc, nullptr, &g_swapChain);
        return SUCCEEDED(hr);
    }

    bool InitComposition(HWND hwnd) {
        ComPtr<IDXGIDevice> dxgiDevice;
        HRESULT hr = g_d3dDevice.As(&dxgiDevice);
        if (FAILED(hr)) return false;

        hr = DCompositionCreateDevice(dxgiDevice.Get(), IID_PPV_ARGS(&g_dcompDevice));
        if (FAILED(hr)) return false;

        hr = g_dcompDevice->CreateTargetForHwnd(hwnd, TRUE, &g_dcompTarget);
        if (FAILED(hr)) return false;

        hr = g_dcompDevice->CreateVisual(&g_dcompVisual);
        if (FAILED(hr)) return false;

        hr = g_dcompVisual->SetContent(g_swapChain.Get());
        if (FAILED(hr)) return false;

        hr = g_dcompTarget->SetRoot(g_dcompVisual.Get());
        if (FAILED(hr)) return false;

        hr = g_dcompDevice->Commit();
        return SUCCEEDED(hr);
    }

    bool CreateRTV() {
        ComPtr<ID3D11Texture2D> backBuffer;
        HRESULT hr = g_swapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer));
        if (FAILED(hr)) return false;
        return SUCCEEDED(g_d3dDevice->CreateRenderTargetView(
            backBuffer.Get(), nullptr, &g_renderTargetView));
    }

    void UpdateOverlayPosition() {
        if (!g_targetWnd || !IsWindow(g_targetWnd)) {
            g_quitRequested = true;
            return;
        }

        RECT r;
        GetClientRect(g_targetWnd, &r);
        POINT pt = { 0, 0 };
        ClientToScreen(g_targetWnd, &pt);

        int newCx = r.right - r.left;
        int newCy = r.bottom - r.top;

        static int lastX = -1, lastY = -1, lastCx = -1, lastCy = -1;

        if (pt.x != lastX || pt.y != lastY) {
            SetWindowPos(g_overlayWnd, HWND_TOPMOST,
                pt.x, pt.y, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE);
            lastX = pt.x; lastY = pt.y;
        }

        if (newCx != lastCx || newCy != lastCy) {
            SetWindowPos(g_overlayWnd, HWND_TOPMOST,
                0, 0, newCx, newCy, SWP_NOACTIVATE | SWP_NOMOVE);

            g_cx = newCx; g_cy = newCy;

            if (g_swapChain) {
                g_renderTargetView.Reset();
                g_swapChain->ResizeBuffers(0, g_cx, g_cy, DXGI_FORMAT_UNKNOWN, 0);
                CreateRTV();
            }

            lastCx = newCx; lastCy = newCy;
        }
    }

    LRESULT CALLBACK OverlayProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
        if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wParam, lParam))
            return true;

        switch (msg) {
        //case WM_NCHITTEST:
        //    if(!Menu::IsOpen())
        //        return HTTRANSPARENT;
        case WM_DESTROY:
            g_quitRequested = true;
            PostQuitMessage(0);
            return 0;
        }
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
} // namespace

// ---------- Публичный API ----------
bool Overlay_Init(HWND targetWnd) {
    g_targetWnd = targetWnd;
    if (!g_targetWnd || !IsWindow(g_targetWnd)) return false;

    data.lines.reserve(512);
    data.circles.reserve(512);
    data.rects.reserve(512);
    data.filledRects.reserve(512);
    data.texts.reserve(128);

    RECT r;
    GetClientRect(g_targetWnd, &r);
    POINT pt = { 0, 0 };
    ClientToScreen(g_targetWnd, &pt);
    g_cx = r.right - r.left;
    g_cy = r.bottom - r.top;

    HINSTANCE hInstance = GetModuleHandleW(nullptr);
    const wchar_t CLASS_NAME[] = L"CSOverlayWindow";

    WNDCLASSW wc = {};
    wc.lpfnWndProc = OverlayProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hbrBackground = nullptr;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    RegisterClassW(&wc);

    g_overlayWnd = CreateWindowExW(
         WS_EX_LAYERED | WS_EX_TOPMOST | WS_EX_TOOLWINDOW | WS_EX_NOREDIRECTIONBITMAP,
        CLASS_NAME, L"", WS_POPUP,
        pt.x, pt.y, g_cx, g_cy,
        nullptr, nullptr, hInstance, nullptr);

    if (!g_overlayWnd) return false;

    if (!InitD3D11(g_overlayWnd, g_cx, g_cy)) return false;
    if (!InitComposition(g_overlayWnd)) return false;
    if (!CreateRTV()) return false;

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = nullptr;
    ImGui_ImplWin32_Init(g_overlayWnd);
    ImGui_ImplDX11_Init(g_d3dDevice.Get(), g_d3dContext.Get());
    g_imguiInitialized = true;


    ImFont* myFont = io.Fonts->AddFontFromFileTTF("C:/Windows/Fonts/arial.ttf", 18.0f);

    if (myFont) {
        io.FontDefault = myFont;  // теперь этот шрифт — глобальный
    }


    ShowWindow(g_overlayWnd, SW_SHOW);
    UpdateWindow(g_overlayWnd);
    g_quitRequested = false;
    return true;
}

void Overlay_Shutdown() {
    if (g_imguiInitialized) {
        ImGui_ImplDX11_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();
        g_imguiInitialized = false;
    }

    g_renderTargetView.Reset();
    g_dcompVisual.Reset();
    g_dcompTarget.Reset();
    g_dcompDevice.Reset();
    g_swapChain.Reset();
    g_d3dContext.Reset();
    g_d3dDevice.Reset();

    if (g_overlayWnd) {
        DestroyWindow(g_overlayWnd);
        g_overlayWnd = nullptr;
    }
    g_targetWnd = nullptr;
}

bool Overlay_PumpMessages() {
    MSG msg;
    while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
        if (msg.message == WM_QUIT) {
            g_quitRequested = true;
        }
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return !g_quitRequested;
}

void Overlay_BeginFrame() {
    UpdateOverlayPosition();

    data.width = g_cx;
    data.height = g_cy;

    float clearColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
    g_d3dContext->ClearRenderTargetView(g_renderTargetView.Get(), clearColor);
    g_d3dContext->OMSetRenderTargets(1, g_renderTargetView.GetAddressOf(), nullptr);

    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();
}

void Overlay_EndFrame() {

    // Рисуем примитивы
    ImDrawList* draw = ImGui::GetBackgroundDrawList();

    for (const auto& r : data.filledRects)
        draw->AddRectFilled(ImVec2(r.x, r.y), ImVec2(r.x + r.w, r.y + r.h), ToImU32(r.color));

    for (const auto& l : data.lines)
        draw->AddLine(ImVec2(l.x1, l.y1), ImVec2(l.x2, l.y2), ToImU32(l.color), l.thickness);

    for (const auto& r : data.rects)
        draw->AddRect(ImVec2(r.x, r.y), ImVec2(r.x + r.w, r.y + r.h),
            ToImU32(r.color), 0.0f, ImDrawFlags_None, r.thickness);

    for (const auto& c : data.circles)
        draw->AddCircle(ImVec2(c.cx, c.cy), c.radius, ToImU32(c.color), 0, c.thickness);

    for (const auto& t : data.texts) {
        int size = WideCharToMultiByte(CP_UTF8, 0, t.text.c_str(), -1, nullptr, 0, nullptr, nullptr);
        std::string utf8(size, 0);
        WideCharToMultiByte(CP_UTF8, 0, t.text.c_str(), -1, &utf8[0], size, nullptr, nullptr);
        draw->AddText(ImGui::GetFont(), t.size, ImVec2(t.x, t.y), ToImU32(t.color), utf8.c_str());
    }

    Menu::Render();

    ImGui::Render();
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
    g_swapChain->Present(0, 0);
}

HWND Overlay_GetHwnd() { return g_overlayWnd; }

void SetClickThrough(bool enabled) {
    LONG_PTR ex = GetWindowLongPtr(g_overlayWnd, GWL_EXSTYLE);
    if (enabled) {
        ex |= WS_EX_TRANSPARENT;
    }
    else {
        ex &= ~WS_EX_TRANSPARENT;
    }
    SetWindowLongPtr(g_overlayWnd, GWL_EXSTYLE, ex);
}