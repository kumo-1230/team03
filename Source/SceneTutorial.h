#pragma once
#include "System/Sprite.h"
#include "Scene.h"
#include "ui_panel.h"
#include "CameraController.h"
#include "sky_map.h"
#include "System/AudioSource.h"
#include "PlayerOld.h"
#include <memory>

class SceneTutorial : public Scene {
public:
	SceneTutorial();
	~SceneTutorial() override {}

	void Initialize() override;
	void Finalize() override;
	void Update(float elapsedTime) override;
	void Render() override;
	void DrawGUI() override;

private:
	std::unique_ptr<UiPanel> tutorialMenu;
	std::unique_ptr<Camera> camera{ nullptr };
	std::unique_ptr<CameraController> cameraController{ nullptr };
	std::unique_ptr<sky_map> skyMap{ nullptr };
	std::unique_ptr<PlayerOld> player{ nullptr };

	AudioSource* BGMTutorial{ nullptr };
};