#pragma once

#include "Button.h"
#include <vector>

class ButtonManager
{
private:

	struct OnButtonBank
	{
		bool upButton = false;
		bool downButton = false;
		bool holdButton = false;
	};

	//ボタンたち
	std::vector<std::unique_ptr<Button>> buttons;

	DirectX::XMFLOAT2 nowHitButtonPos{};
	DirectX::XMFLOAT2 nowHitButtonLeng{};

	OnButtonBank onButton;

	Button* currentButton = nullptr;

	float alphaMax = 1.0f;

	//アルファ値変更
	bool alphaChange = false;

	float progress = 0.0f;
public:
	ButtonManager() {};
	~ButtonManager() {};

public:
	/////////////////////////////////////////////////////

	void SetButton(const char* fileName, DirectX::XMFLOAT2 p, DirectX::XMFLOAT2 l, DirectX::XMFLOAT2 sp, DirectX::XMFLOAT2 sl, int la,int n,bool v);
	void SetButton(const char* fileName, DirectX::XMFLOAT2 p, DirectX::XMFLOAT2 l, DirectX::XMFLOAT2 sp, DirectX::XMFLOAT2 sl, DirectX::XMFLOAT2 hp, DirectX::XMFLOAT2 hl, int la,int n,bool v);
	Button* GetButton(int i) { return buttons[i].get(); }

	DirectX::XMFLOAT2 GetnowHitButtonPos() { return nowHitButtonPos; }
	DirectX::XMFLOAT2 GetnowHitButtonLeng() { return nowHitButtonLeng; }
	bool GetOnButtonDown() { return onButton.downButton; }
	bool GetOnButtonHold() { return onButton.holdButton; }
	bool GetOnButtonUp() { return onButton.upButton; }
	void SetAlphaMax(float a) { alphaMax = a; }
	void SetAlphaChange(bool b) { alphaChange = b; }
	void SetProgress(float p) { progress = p; }
	Button* GetCurrentButton() { return currentButton; }

	int GetButtonSize() { return static_cast<int>(buttons.size()); }

	/////////////////////////////////////////////////////

public:
	void Update();
	void Render(const RenderContext& rc, bool stop);

	int HitButton();
};