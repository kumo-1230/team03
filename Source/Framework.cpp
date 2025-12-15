#include <memory>
#include <sstream>
#include <imgui.h>

#include "Framework.h"
#include "System/Input.h"
#include "System/Graphics.h"
#include "System/ImGuiRenderer.h"
#include "SceneGame.h"
#include "SceneTitle.h"
#include "SceneManager.h"
#include "EffectManager.h"
#include "System/Audio.h"
#include "KeyInput.h"
#include <k_cursor.h>

// 垂直同期間隔設定
static const int syncInterval = 1;

//static SceneGame sceneGame;

// コンストラクタ
// Framework.cpp のコンストラクタ
Framework::Framework(HWND hWnd)
	: hWnd(hWnd)
{
	OutputDebugStringA("Framework constructor START\n");

	//オーディオ初期化
	OutputDebugStringA("Initializing Audio\n");
	Audio::Instance().Initialize();
	OutputDebugStringA("Audio initialized\n");

	// インプット初期化
	OutputDebugStringA("Initializing Input\n");
	Input::Instance().Initialize(hWnd);
	OutputDebugStringA("Input initialized\n");

	// グラフィックス初期化
	OutputDebugStringA("Initializing Graphics\n");
	Graphics::Instance().Initialize(hWnd);
	OutputDebugStringA("Graphics initialized\n");

	// IMGUI初期化
	OutputDebugStringA("Initializing ImGui\n");
	ImGuiRenderer::Initialize(hWnd, Graphics::Instance().GetDevice(), Graphics::Instance().GetDeviceContext());
	OutputDebugStringA("ImGui initialized\n");

	//エフェクトマネージャー初期化
	OutputDebugStringA("Initializing EffectManager\n");
	EffectManager::Instance().Initialize();
	OutputDebugStringA("EffectManager initialized\n");

	// カスタムカーソル初期化（追加）
	OutputDebugStringA("Initializing CustomCursor\n");
	CustomCursor::Instance().Initialize("Data/Sprite/new_cursor.png");
	CustomCursor::Instance().Hide();  // 初期状態は非表示
	OutputDebugStringA("CustomCursor initialized\n");

	// シーン初期化
	OutputDebugStringA("Initializing SceneManager\n");
	SceneManager::Instance().ChangeScene(new SceneTitle);
	OutputDebugStringA("SceneManager initialized\n");

	srand((unsigned int)time(NULL));

	OutputDebugStringA("Framework constructor END\n");
}
// デストラクタ
Framework::~Framework()
{
	// シーン終了化
	//sceneGame.Finalize();
	SceneManager::Instance().Clear();

	// IMGUI終了化
	ImGuiRenderer::Finalize();

	//エフェクトマネージャー終了化
	EffectManager::Instance().Finalize();

	//オーディオ終了化
	Audio::Instance().Finalize();
}

// 更新処理
void Framework::Update(float elapsedTime)
{
	// インプット更新処理
	Input::Instance().Update();

	KeyInput::Instance().Update();

	// IMGUIフレーム開始処理	
	ImGuiRenderer::NewFrame();

	CustomCursor::Instance().Update(elapsedTime);

	// シーン更新処理
	SceneManager::Instance().Update(elapsedTime);
}

// 描画処理
void Framework::Render(float elapsedTime)
{
	//別スレッド宙にデバイスコンテキストが使われていた場合に
	//同時アクセスをしないように排他制御する
	std::lock_guard<std::mutex> lock(Graphics::Instance().GetMutex());

	ID3D11DeviceContext* dc = Graphics::Instance().GetDeviceContext();

	// 画面クリア
	Graphics::Instance().Clear(0, 0, 0.1f, 1);

	// レンダーターゲット設定
	Graphics::Instance().SetRenderTargets();

	// シーン描画処理
	SceneManager::Instance().Render();


	// シーンGUI描画処理
	SceneManager::Instance().DrawGUI();

	// カスタムカーソルの描画
	CustomCursor::Instance().Render(dc);
#if 0
	// IMGUIデモウインドウ描画（IMGUI機能テスト用）
	ImGui::ShowDemoWindow();
#endif
	// IMGUI描画
	ImGuiRenderer::Render(dc);

	// 画面表示
	Graphics::Instance().Present(syncInterval);
}

// フレームレート計算
void Framework::CalculateFrameStats()
{
	// Code computes the average frames per second, and also the 
	// average time it takes to render one frame.  These stats 
	// are appended to the window caption bar.
	static int frames = 0;
	static float time_tlapsed = 0.0f;

	frames++;

	// Compute averages over one second period.
	if ((timer.TimeStamp() - time_tlapsed) >= 1.0f)
	{
		float fps = static_cast<float>(frames); // fps = frameCnt / 1
		float mspf = 1000.0f / fps;
		std::ostringstream outs;
		outs.precision(6);
		outs << "FPS : " << fps << " / " << "Frame Time : " << mspf << " (ms)";
		SetWindowTextA(hWnd, outs.str().c_str());

		// Reset for next average.
		frames = 0;
		time_tlapsed += 1.0f;
	}
}

// アプリケーションループ
int Framework::Run()
{
	MSG msg = {};

	while (WM_QUIT != msg.message)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			timer.Tick();
			CalculateFrameStats();

			float elapsedTime = timer.TimeInterval();
			Update(elapsedTime);
			Render(elapsedTime);
		}
	}
	return static_cast<int>(msg.wParam);
}

// メッセージハンドラ
LRESULT CALLBACK Framework::HandleMessage(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (ImGuiRenderer::HandleMessage(hWnd, msg, wParam, lParam))
		return true;

	switch (msg)
	{
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc;
		hdc = BeginPaint(hWnd, &ps);
		EndPaint(hWnd, &ps);
		break;
	}
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case WM_CREATE:
		break;
	case WM_KEYDOWN:
		if (wParam == VK_ESCAPE) PostMessage(hWnd, WM_CLOSE, 0, 0);
		break;
	case WM_ENTERSIZEMOVE:
		// WM_EXITSIZEMOVE is sent when the user grabs the resize bars.
		timer.Stop();
		break;
	case WM_EXITSIZEMOVE:
		// WM_EXITSIZEMOVE is sent when the user releases the resize bars.
		// Here we reset everything based on the new window dimensions.
		timer.Start();
		break;
	default:
		return DefWindowProc(hWnd, msg, wParam, lParam);
	}
	return 0;
}
