#pragma once

#include "System/Sprite.h"
#include "Scene.h"

#include <thread>

//ローディング
class SceneLoading : public Scene
{
private:
	std::unique_ptr<Sprite> load_Back = nullptr;
	std::unique_ptr<Sprite> load_Sprite = nullptr;
	float angle = 0.0f;
	Scene* nextScene = nullptr;

	float sprPos = 0;
	int sprMax = 7;

	float count{};
	float timer;

	std::thread* thread = nullptr;

private:
	//ローディングスレッド
	static void LoadingThread(SceneLoading* scene);

public:
	//SceneLoading(){}
	SceneLoading(Scene* nexetScene): nextScene(nexetScene){}
	~SceneLoading() override {}

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