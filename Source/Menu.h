#pragma once
#include "ButtonManager.h"
#include "KeyInput.h"


namespace MENU
{
	//背景あり
	const int BACK_OFF (0);
	//背景なし
	const int BACK_ON (1);
}

class Menu
{
private:
	//メニューが表示されてるか
	bool menuStart = false;
	//int menuMode = 0;
	std::unique_ptr<ButtonManager> buttonManager;
	//背景の画像
	std::unique_ptr<Sprite> sprBack;

public:
	Menu();
	~Menu();

public:
	////////////////////////////

	//ボタン追加
	void SetButton(const char* fileName, DirectX::XMFLOAT2 p, DirectX::XMFLOAT2 l, DirectX::XMFLOAT2 sp, DirectX::XMFLOAT2 sl, int la, int n, bool v)
	{
		buttonManager->SetButton(fileName, p, l, sp, sl, la, n, v);
	}
	//ボタン追加
	void SetButton(const char* fileName, DirectX::XMFLOAT2 p, DirectX::XMFLOAT2 l, DirectX::XMFLOAT2 sp, DirectX::XMFLOAT2 sl, DirectX::XMFLOAT2 hp, DirectX::XMFLOAT2 hl, int la, int n, bool v)
	{
		buttonManager->SetButton(fileName, p, l, sp, sl, hp, hl, la, n, v);
	}

	//メニューを表示するか
	void SetMenuStart(bool S) { menuStart = S; }
	//メニューを表示してるか
	bool GetMenyuStart() { return menuStart; }
	//特定のボタンをゲット
	Button* GetButton(int i) { return buttonManager->GetButton(i); }
	ButtonManager* GetButtonManager() { return buttonManager.get(); }
	void SetAlphaMax(float a) { buttonManager->SetAlphaMax(a); }
	void SetAlphaChange(bool a) { buttonManager->SetAlphaChange(a); }
	void SetProgress(float a) { buttonManager->SetProgress(a); }

	Button* GetCurrentButton() { return buttonManager->GetCurrentButton(); }

	////////////////////////////
public:

	void Updeat(int* hb);
	void Render(const RenderContext& rc, int renderMode, bool stop);

};