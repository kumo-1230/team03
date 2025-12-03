#pragma once

#include "CameraController.h"
#include "Scene.h"
#include "Camera.h"
#include <memory>
#include "Player.h"
#include "Menu.h"
#include <vector>
#include "sky_map.h"
#include "Lerp.h"
#include "math.h"
#include "System/AudioSource.h"

// ゲームシーン
class SceneTutorial : public Scene
{
private:
	std::unique_ptr<Player> player = nullptr;
	std::unique_ptr<Camera> camera = nullptr;
	std::unique_ptr<CameraController> cameraController = nullptr;
	std::unique_ptr<sky_map> skyMap = nullptr;

	AudioSource* BGMTutorial = nullptr;

public:
	SceneTutorial();
	//~SceneGame() {};
	~SceneTutorial() override {}

	// 初期化
	//void Initialize();
	void Initialize() override;

	// 終了化
	//void Finalize();
	void Finalize() override;

	// 更新処理
	//void Update(float elapsedTime);
	void Update(float elapsedTime) override;

	// 描画処理
	//void Render();
	void Render() override;

	// GUI描画
	//void DrawGUI();
	void DrawGUI() override;


};
