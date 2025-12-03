#pragma once

#include "System/Sprite.h"
#include "Scene.h"
#include "Menu.h"
#include "CameraController.h"
#include "Lerp.h"
#include "sky_map.h"
#include "System/AudioSource.h"

enum PwUp
{
	//戻る
	RETURN = 0,
	//直進上げる
	STRING_UP,
	//直進下げる
	STRING_DOWN,
	//ホーミング上げる
	HOMING_UP,
	//ホーミング下げる
	HOMING_DOWN,
	//回転上げる
	ROTATE_UP,
	//回転下げる
	ROTATE_DOWN,
};

const int addCost[3][5] =
{
	{1,2,3,4,5},
	{1,2,3,4,5},
	{1,2,3,4,5},
};

//タイトルシーン
class SceneTitle : public Scene
{
private:
	std::unique_ptr<Menu> titleStartMenu;

	std::unique_ptr<Camera> camera = nullptr;
	std::unique_ptr<CameraController> cameraController = nullptr;
	std::unique_ptr<Sprite> sprTitle = nullptr;
	std::unique_ptr<Sprite> sprButtonBack = nullptr;
	std::unique_ptr<Sprite> sprFadeLoad = nullptr;
	//std::unique_ptr<Sprite> sprCursor = nullptr;

	std::unique_ptr<sky_map> skyMap = nullptr;

	AudioSource* clickSE = nullptr;
	AudioSource* onCursorSE = nullptr;
	AudioSource* onStartSE = nullptr;
	AudioSource* backGroundMusic = nullptr;

public:
	SceneTitle();
	~SceneTitle() override{}

	//初期化
	void Initialize() override;

	//終了化
	void Finalize() override;

	//更新処理
	void Update(float elapsedTime) override;

	//描画処理
	void Render() override;

	//GUI描画
	void DrawGUI() override;
};