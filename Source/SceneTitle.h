#pragma once
#include "System/Sprite.h"
#include "Scene.h"
#include "ui_panel.h"
#include "CameraController.h"
#include "Lerp.h"
#include "sky_map.h"
#include "System/AudioSource.h"

enum PwUp {
	RETURN = 0,
	STRING_UP,
	STRING_DOWN,
	HOMING_UP,
	HOMING_DOWN,
	ROTATE_UP,
	ROTATE_DOWN,
};

const int addCost[3][5] = {
  {1, 2, 3, 4, 5},
  {1, 2, 3, 4, 5},
  {1, 2, 3, 4, 5},
};

class SceneTitle : public Scene {
public:
	SceneTitle();
	~SceneTitle() override {}

	void Initialize() override;
	void Finalize() override;
	void Update(float elapsedTime) override;
	void Render() override;
	void DrawGUI() override;

private:
	std::unique_ptr<UiPanel> titleStartMenu;
	std::unique_ptr<Camera> camera{ nullptr };
	std::unique_ptr<CameraController> cameraController{ nullptr };
	std::unique_ptr<Sprite> sprTitle{ nullptr };
	std::unique_ptr<Sprite> sprButtonBack{ nullptr };
	std::unique_ptr<Sprite> sprFadeLoad{ nullptr };
	std::unique_ptr<sky_map> skyMap{ nullptr };

	AudioSource* clickSE{ nullptr };
	AudioSource* onCursorSE{ nullptr };
	AudioSource* onStartSE{ nullptr };
	AudioSource* backGroundMusic{ nullptr };

	int lastHoveredButtonId{ -1 };

	LightManager lightManager;
};