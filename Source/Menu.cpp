#include "Menu.h"
#include <DirectXMath.h>
#include <imgui.h>
#include "imgui_impl_win32.h"       // Win32 用バックエンド
#include "imgui_impl_dx11.h"        // DirectX11 用バックエンド
#include "common.h"

Menu::Menu()
{
	buttonManager.reset(new ButtonManager());
	//buttonManager->SetButton("Data/Sprite/test.png", { 10,10 }, { 100,100 }, 0, 0);
	//sprBack.reset(new Sprite("Data/Sprite/menuBack.png"));
}

Menu::~Menu()
{
}

//反応したボタンの値を入れる
void Menu::Updeat(int* hb)
{
	buttonManager->Update();

	if (KeyInput::Instance().GetKeyDown(VK_F1))
	{
		if (menuStart)
		{
			menuStart = false;
		}
		else
		{
			menuStart = true;
		}
	}

	if (menuStart)
	{
		if(KeyInput::Instance().GetKeyDown(VK_LBUTTON))
		{
			*hb = buttonManager->HitButton();
		}
		else
		{
			*hb = -1;
		}
	}
}

void Menu::Render(const RenderContext& rc, int renderMode, bool stop)
{
	if (menuStart)
	{
		switch (renderMode)
		{
		case MENU::BACK_ON:
			sprBack->Render(rc, 10, 10, 0, SCREEN_W - 20, SCREEN_H - 20, 0, 1, 1, 1, 1);
			break;
		}
		buttonManager->Render(rc,stop);
	}
}
