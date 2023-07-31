#pragma once
#define STB_IMAGE_IMPLEMENTATION
//IMGUI
#include "Application.h"
#include "../IMGUI_DirectX11/imgui.h"
#include "../IMGUI_DirectX11/imgui_impl_dx11.h"
#include "../IMGUI_DirectX11/imgui_impl_win32.h"
#include <d3d11.h>
#include <tchar.h>
#include <vector>

// Data
static ID3D11Device* g_pd3dDevice = nullptr;
static ID3D11DeviceContext* g_pd3dDeviceContext = nullptr;
static IDXGISwapChain* g_pSwapChain = nullptr;
static UINT                     g_ResizeWidth = 0, g_ResizeHeight = 0;
static ID3D11RenderTargetView* g_mainRenderTargetView = nullptr;

enum OptionState
{
    Audio,
    Controls
};

//class MainMenu
//{


// Forward declarations of helper functions
//bool CreateDeviceD3D(HWND hWnd);
//void CleanupDeviceD3D();
//void CreateRenderTarget();
//void CleanupRenderTarget();
//LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
//
//void CreateButton(float xPos, float yPos, ImVec2 size, float fontScale, const char* title, bool* behavior = nullptr);

//Options op to check what options to load, float h (window height), float w (window width)
//void OptionsMenuBehavior(OptionState op, float h, float w);

void CreateButton(float xPos, float yPos, ImVec2 size, float fontScale, const char* title, bool* behavior)
{
    ImGui::SetCursorPosX((xPos - size.x) / 2);
    ImGui::SetCursorPosY(yPos);
    ImGui::SetWindowFontScale(fontScale);
    bool check = *behavior;
    if (ImGui::Button(title, size))
        *behavior = !check;
}

void CreateButtonOP(float xPos, float yPos, ImVec2 size, float fontScale, const char* title, bool* behavior)
{
    ImGui::SetCursorPosX((xPos - size.x) / 2);
    ImGui::SetCursorPosY(yPos);
    ImGui::SetWindowFontScale(fontScale);
    bool check = *behavior;
    if (ImGui::Button(title, size))
        *behavior = true;
}

void CheckBools(std::vector<bool>* menuChecks, bool currCheck)
{
    for (auto check : *menuChecks)
    {
        if (currCheck == true && check == true)
        {
            check = false;
        }
    }
}
//
//void OptionsMenuBehavior(Options op, float h, float w)
//{
//    ImGui::SetNextWindowPos(ImVec2(0, 0));
//    ImGui::SetNextWindowSize(ImVec2(w - 450, h  - 40));
//
//    ImGui::Begin("", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings);
//
//    if (op.cs == "AUDIO")
//    {
//
//    }
//    else if (op.cs == "CONTROLS")
//    {
//
//    }
//
//    ImGui::End();
//}

static OptionState currentOptionState;

// Main code


// Helper functions
void CreateRenderTarget()
{
    ID3D11Texture2D* pBackBuffer;
    g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
    g_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &g_mainRenderTargetView);
    pBackBuffer->Release();
}

void CleanupRenderTarget()
{
    if (g_mainRenderTargetView) { g_mainRenderTargetView->Release(); g_mainRenderTargetView = nullptr; }
}

bool CreateDeviceD3D(HWND hWnd)
{
    // Setup swap chain
    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferCount = 2;
    sd.BufferDesc.Width = 0;
    sd.BufferDesc.Height = 0;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hWnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    UINT createDeviceFlags = 0;
    //createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
    D3D_FEATURE_LEVEL featureLevel;
    const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
    HRESULT res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);
    if (res == DXGI_ERROR_UNSUPPORTED) // Try high-performance WARP software driver if hardware is not available.
        res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_WARP, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);
    if (res != S_OK)
        return false;

    CreateRenderTarget();
    return true;
}

void CleanupDeviceD3D()
{
    CleanupRenderTarget();
    if (g_pSwapChain) { g_pSwapChain->Release(); g_pSwapChain = nullptr; }
    if (g_pd3dDeviceContext) { g_pd3dDeviceContext->Release(); g_pd3dDeviceContext = nullptr; }
    if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = nullptr; }
}





// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Win32 message handler
// You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
// - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
// - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
// Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg)
    {
    case WM_SIZE:
        if (wParam == SIZE_MINIMIZED)
            return 0;
        g_ResizeWidth = (UINT)LOWORD(lParam); // Queue resize
        g_ResizeHeight = (UINT)HIWORD(lParam);
        return 0;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
            return 0;
        break;
    case WM_DESTROY:
        ::PostQuitMessage(0);
        return 0;
    }
    return ::DefWindowProcW(hWnd, msg, wParam, lParam);
}

void CreateScore(Application* app)
{
    static ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings;


    const ImGuiViewport* viewport = ImGui::GetMainViewport();

    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
    ImGui::Begin("Score", nullptr, flags);

    auto width = ImGui::GetWindowWidth();
    auto height = ImGui::GetWindowHeight();
}

int MainMenu(Application* app, int* level)
{
    // Create application window
    //ImGui_ImplWin32_EnableDpiAwareness();
    WNDCLASSEXW wc = { sizeof(wc), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(nullptr), nullptr, nullptr, nullptr, nullptr, L"ImGui Example", nullptr };
    ::RegisterClassExW(&wc);
    HWND hwnd = ::CreateWindowW(wc.lpszClassName, L"Drunken Alien Assault", WS_OVERLAPPEDWINDOW, 100, 100, 1280, 800, nullptr, nullptr, wc.hInstance, nullptr);

    // Initialize Direct3D
    if (!CreateDeviceD3D(hwnd))
    {
        CleanupDeviceD3D();
        ::UnregisterClassW(wc.lpszClassName, wc.hInstance);
        return 1;
    }
    // Show the window
    ::ShowWindow(hwnd, SW_SHOWDEFAULT);
    ::UpdateWindow(hwnd);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsLight();

    // Setup Platform/Renderer backends
    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);

    // Load Fonts
    // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
    // - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
    // - If the file cannot be loaded, the function will return a nullptr. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
    // - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
    // - Use '#define IMGUI_ENABLE_FREETYPE' in your imconfig file to use Freetype for higher quality font rendering.
    // - Read 'docs/FONTS.md' for more instructions and details.
    // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
    //io.Fonts->AddFontDefault();
    //io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\segoeui.ttf", 18.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
    //ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, nullptr, io.Fonts->GetGlyphRangesJapanese());
    //IM_ASSERT(font != nullptr);

    // Our state

    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    bool musicState = true;
    bool showOptions = false;
    bool startLevel1 = false;
    bool startLevel2 = false;
    bool startLevel3 = false;
    bool quitting = false;

    static ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings;
    static ImGuiWindowFlags flagsOp = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove;


    //Options Menu bools

    bool showAudioOp = false;
    bool showControls = false;

    bool showSettings = false;

    float musicVolume = 0.0f; 
    float audioVolume = 0.0f; 

    // Main loop
    bool done = false;
    while (!done)
    {
        // Poll and handle messages (inputs, window resize, etc.)
        // See the WndProc() function below for our to dispatch events to the Win32 backend.
        MSG msg;
        while (::PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE))
        {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
            if (msg.message == WM_QUIT)
                done = true;
        }
        if (done)
            break;

        // Handle window resize (we don't resize directly in the WM_SIZE handler)
        if (g_ResizeWidth != 0 && g_ResizeHeight != 0)
        {
            CleanupRenderTarget();
            g_pSwapChain->ResizeBuffers(0, g_ResizeWidth, g_ResizeHeight, DXGI_FORMAT_UNKNOWN, 0);
            g_ResizeWidth = g_ResizeHeight = 0;
            CreateRenderTarget();
        }

        // Start the Dear ImGui frame
        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();




        //Cretes Main Menu Window (Fullscreen) 0.1 (When Closed the Game starts running)
        {

            static float f = 0.0f;
            static int counter = 0;



            //Create Window
            ImVec2 buttonSize = { 200, 100 };

            const ImGuiViewport* viewport = ImGui::GetMainViewport();

            ImGui::SetNextWindowPos(ImVec2(0, 0));
            ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
            ImGui::Begin("Main Menu", nullptr, flags);

            auto width = ImGui::GetWindowWidth();
            auto height = ImGui::GetWindowHeight();

            //Create Quit Button
            CreateButton(width, 200, buttonSize, 1.5f, "QUIT", &quitting);
            //Create Play Button
            CreateButton(width - 450, 0, buttonSize, 1.5f, "PLAY Level 1", &startLevel1);
            CreateButton(width, 0, buttonSize, 1.5f, "PLAY Level 2", &startLevel2);
            CreateButton(width + 450, 0, buttonSize, 1.5f, "PLAY Level 3", &startLevel3);
            //Create Options button
            CreateButton(width, 100, buttonSize, 1.5f, "OPTIONS", &showOptions);

            ImGui::Checkbox("Music", &musicState);
            if (musicState)
            {
                //std::cout << "music playing";
                app->audioEngine.ResumeMusic();
                app->audioMuted = false;
            }
            else
            {
                //std::cout << "music paused";
                app->audioEngine.PauseMusic();
                app->audioMuted = true;
            }
            ImGui::SetCursorPos(ImVec2(0, 0));



            //Create FPS count 
            /*ImGui::SetWindowFontScale(1.0f);
            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate); */
            ImGui::End();
        }


        if (showOptions)
        {
            auto width = ImGui::GetWindowWidth();
            auto height = ImGui::GetWindowHeight();

            std::vector<bool> optionsCheck;

            optionsCheck.push_back(showAudioOp);
            optionsCheck.push_back(showControls);

            ImGui::SetNextWindowPos(ImVec2(0, 0));
            ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
            ImGui::Begin("Options", &showOptions, flagsOp);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)

            CreateButton(150, 10, ImVec2(100, 50), 1.2, "BACK", &showOptions);

            CreateButtonOP(420, 70, ImVec2(150, 25), 1.2, "AUDIO", &showAudioOp);
            //CreateButtonOP(420, 100, ImVec2(150, 25), 1.2, "CONTROLS", &showControls);




            //If Sub-Options Are Opened
            if (showAudioOp || showControls)
            {
                if (ImGui::BeginChild(1))
                {


                    if (showAudioOp)
                    {
                        currentOptionState = Audio;
                    }
                    /*else if (showControls)
                    {
                        currentOptionState = Controls;
                    }*/

                    if (currentOptionState == Audio)
                    {
                        ImGui::SetCursorPos(ImVec2(300, 50)); 
                        ImGui::Text("This is where the Audio Options will go!");
                        if (ImGui::SliderFloat("Music", &musicVolume, 0.0f, 100.0f))
                        {
                            std::cout << "Value Updated Volume: " << musicVolume << std::endl; 
                            app->audioEngine.SetGlobalMusicVolume(musicVolume / 100);
                        }
                        if (ImGui::SliderFloat("Audio", &audioVolume, 0.0f, 100.0f))
                        {
                            std::cout << "Value Updated Volume: " << audioVolume << std::endl;
                            app->audioEngine.SetGlobalSoundVolume(audioVolume / 100);
                        }
                        //render audio options
                    }
                    //else if (currentOptionState == Controls)
                    //{
                    //    ImGui::Text("This is where the Controls will go!");
                    //    //render controls
                    //}


                    ImGui::EndChild();
                }

            }


            ImGui::End();
        }

        //If QUIT is selected
        if (quitting)
        {
            auto width = ImGui::GetWindowWidth();
            auto height = ImGui::GetWindowHeight();

            ImGui::SetCursorPos(ImVec2(width / 2, height / 2));
            ImGui::OpenPopup("QUIT?");
            if (ImGui::BeginPopupModal("QUIT?", &quitting, ImGuiWindowFlags_AlwaysAutoResize))
            {
                ImGui::Text("Are you sure you want to Quit?");
                ImGui::Separator();

                if (ImGui::Button("Close"))
                {
                    ImGui::CloseCurrentPopup();
                    quitting = false;
                }
                ImGui::SameLine(0, 1);
                if (ImGui::Button("Accept"))
                {
                    done = true;
                    return 2;
                }

                ImGui::EndPopup();

            }

        }

        //If any of the play level buttons are selected
        if (startLevel1)
        {
            //Tell the window that it's handling is done
            *level = 1;
            app->playerSystem.paused = false;
            done = true;
        }
        if (startLevel2)
        {
            //Tell the window that it's handling is done
            *level = 2;
            app->playerSystem.paused = false;
            done = true;
        }
        if (startLevel3)
        {
            //Tell the window that it's handling is done
            *level = 3;
            app->playerSystem.paused = false;
            done = true;
        }

        // Rendering
        ImGui::Render();
        const float clear_color_with_alpha[4] = { clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w };
        g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, nullptr);
        g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, clear_color_with_alpha);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

        g_pSwapChain->Present(1, 0); // Present with vsync
        //g_pSwapChain->Present(0, 0); // Present without vsync
    }

    // Cleanup
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    CleanupDeviceD3D();
    ::DestroyWindow(hwnd);
    ::UnregisterClassW(wc.lpszClassName, wc.hInstance);

    return 0;
}

//inline bool loadImageByMemory(ID3D11Device* device, unsigned char* image, size_t imageSize, ID3D11ShaderResourceView** result)
//{
//    D3DX11_IMAGE_LOAD_INFO information; 
//}

//};