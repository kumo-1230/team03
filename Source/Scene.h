#pragma once

//シーン
class Scene
{
private:
	bool ready = false;
protected:
	//反応したボタンの数値を入れる
	int menuNum = -1;

public:
	Scene() {}
	virtual ~Scene(){}

	//初期化
	virtual void Initialize() = 0;

	//終了化
	virtual void Finalize() = 0;

	//更新処理
	virtual void Update(float elapsedTime) = 0;

	//描画処理
	virtual void Render() = 0;

	//GUI描画
	virtual void DrawGUI() = 0;

	//準備完了しているか
	bool IsReady() const { return ready; }

	//準備完了しているか
	void SetReady() { ready = true; }
};