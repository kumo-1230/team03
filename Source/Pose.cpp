#include "Pose.h"
#include "common.h"
#include "SceneManager.h"
#include "SceneLoading.h"
#include "SceneGame.h"
#include "SceneTitle.h"
#include "KeyInput.h"
#include "System/Audio.h"
#include "SceneTutorial.h"
#include "System/Graphics.h"

Pose::Pose()
{
	Initializer();
}

Pose::~Pose()
{
	delete setPause;
	delete pauseBackSE;
	delete pauseSelectSE;
}

void Pose::Initializer()
{
	sprSettingBack = std::make_unique<Sprite>("./Data/Sprite/setting_default.png");
	sprPoseBack = std::make_unique<Sprite>("./Data/Sprite/pause_default.png");

	setPause = Audio::Instance().LoadAudioSource("Data/Sound/Game/SE_game_pause.wav");
	setPause->SetVolume(1.0f);
	pauseBackSE = Audio::Instance().LoadAudioSource("Data/Sound/Pause/SE_setting_back.wav");
	pauseSelectSE = Audio::Instance().LoadAudioSource("Data/Sound/Pause/SE_setting_select.wav");

	poseMenu = std::make_unique<Menu>();
	poseMenu->SetButton("./Data/Sprite/pause_button.png", { SCREEN_W_CENTER - 417 / 2,50 }, { 417,476 }, { 0,0 }, {417,476}, 0, 0, true);
	poseMenu->SetButton("./Data/Sprite/pause_button.png", { SCREEN_W_CENTER - 417 / 2,SCREEN_H_CENTER }, { 417,476 }, { 0,476 }, { 417,476 }, 0, 1, true);
	poseMenu->SetButton("./Data/Sprite/pause_button.png", { SCREEN_W_CENTER + 417,0 }, { 417,476 }, { 0,476 * 2 }, { 417,476 }, 0, 2, true);
	poseMenu->SetButton("./Data/Sprite/pause_button.png", { SCREEN_W_CENTER + 417,SCREEN_H_CENTER }, { 417,476 }, { 0,476 * 3 }, { 417,476 }, 0, 3, true);
	poseMenu->SetMenuStart(false);

	settingMenu = std::make_unique<Menu>();
	settingMenu->SetButton("./Data/Sprite/empty.png", { 900,145  }, { 110,64 }, { 0,0 }, { 0,0 }, 0, 0, true);
	settingMenu->SetButton("./Data/Sprite/empty.png", { 1150,145 }, { 110,64 }, { 0,0 }, { 0,0 }, 0, 1, true);
	settingMenu->SetButton("./Data/Sprite/empty.png", { 1425,145 }, { 110,64 }, { 0,0 }, { 0,0 }, 0, 2, true);
	settingMenu->SetButton("./Data/Sprite/empty.png", { 960,390  }, { 165,64 }, { 0,0 }, { 0,0 }, 0, 3, true);
	settingMenu->SetButton("./Data/Sprite/empty.png", { 1290,390 }, { 220,64 }, { 0,0 }, { 0,0 }, 0, 4, true);
	settingMenu->SetButton("./Data/Sprite/empty.png", { 960,640  }, { 165,64 }, { 0,0 }, { 0,0 }, 0, 5, true);
	settingMenu->SetButton("./Data/Sprite/empty.png", { 1320,640 }, { 165,64 }, { 0,0 }, { 0,0 }, 0, 6, true);
	settingMenu->SetButton("./Data/Sprite/empty.png", { 840, 940 }, { 165,64 }, { 0,0 }, { 0,0 }, 0, 7, true);
	settingMenu->SetButton("./Data/Sprite/empty.png", { 1120,940 }, { 165,64 }, { 0,0 }, { 0,0 }, 0, 8, true);
	settingMenu->SetButton("./Data/Sprite/empty.png", { 1400,940 }, { 165,64 }, { 0,0 }, { 0,0 }, 0, 9, true);
	settingMenu->SetButton("./Data/Sprite/setting_back.png", { SCREEN_W - 246.0f,0 }, { 246,105 }, { 0,0 }, { 246,105 }, 0, 10, true);
	settingMenu->SetMenuStart(false);

	//for (int i = 0; i < settingMenu->GetButtonManager()->GetButtonSize(); i++)
	//{
	//	settingMenu->GetButton(i)->SetRenderMode(BUTTON_R_MODE::INVISIBLE);
	//}

	sensitivity       = static_cast<int>(SENSITIVITY_TYPE::NORMALE);
	fov               = static_cast<int>(FOV_TYPE::NORMALE);
	shake             = static_cast<int>(SHAKE_TYPE::ON);
	drunkenness       = static_cast<int>(DRUNKENNESS_TYPE::NONE);


	buttonSensitivity = 1;
	buttonFov         = 4;
	buttonShake       = 5;
	buttonDrunkenness = 9;

	sprSetting        = std::make_unique<Sprite>("Data/Sprite/setting_switch.png");

	lerp = std::make_unique<Lerp>(0.1f);
	lerpButton = std::make_unique<Lerp>(0.1f);
}

void Pose::Update(float elapsedTime)
{
	{
		if (KeyInput::Instance().GetKeyDown('P') && onPose == false)
		{
			onPose = true;
			fadeIn = true;

			poseMenu->SetMenuStart(true);
			settingMenu->SetMenuStart(false);
			lerp->SetAmount(0.0f);
			poseMenu->SetAlphaChange(true);
			poseMenu->SetAlphaMax(0.5f);

			setPause->Stop();
			setPause->Play(false);
		}
		else if (KeyInput::Instance().GetKeyDown('P') && onPose == true)
		{
			fadeOut = true;
			fadeIn = false;

			setPause->Stop();
			setPause->Play(false);
		}
		if(fadeOut) PoseOff();
	}

	if (!onPose) return;

	if (fadeIn)
	{
		lerp->Update(elapsedTime, Lerp::RESET_TYPE::Fixed, Lerp::ADD_TYPE::ADD);
		alhpa = lerp->GetOffset(Lerp::EASING_TYPE::Normal, 0.0f, 1.0f);
		poseMenu->SetProgress(lerp->GetProgress(Lerp::EASING_TYPE::Normal));
		lerpButton->SetAmount(0.0f);
		if (lerp->GetAmount() >= lerp->GetMaxAmount())
		{
			poseMenu->SetAlphaChange(false);
		}
	}
	else
	{
		lerp->Update(elapsedTime, Lerp::RESET_TYPE::Fixed, Lerp::ADD_TYPE::Subtract);
		lerpButton->Update(elapsedTime, Lerp::RESET_TYPE::Fixed, Lerp::ADD_TYPE::ADD);
		alhpa = lerp->GetOffset(Lerp::EASING_TYPE::Normal, 0.0f, 1.0f);
		poseMenu->SetAlphaChange(true);
		poseMenu->SetAlphaMax(0.0f);
		poseMenu->SetProgress(lerpButton->GetProgress(Lerp::EASING_TYPE::Normal));
	}

	if (poseMenu->GetMenyuStart())
	{
		poseMenu->Updeat(&num);

		if (num != -1)
		{
			numBank = num;
		}

		switch (numBank)
		{
		case 0:
			poseMenu->SetMenuStart(false);
			settingMenu->SetMenuStart(true);
			numBank = -1;
			break;
		case 1:
			PoseOff();
			fadeIn = false;
			break;
		case 2:
			if (PoseOff())
			{
				SceneManager::Instance().ChangeScene(new SceneLoading(new SceneTitle()));
			}
			fadeIn = false;
			break;
		case 3:
			if (PoseOff())
			{
				if (tutorial)
					SceneManager::Instance().ChangeScene(new SceneLoading(new SceneTutorial()));
				else
					SceneManager::Instance().ChangeScene(new SceneLoading(new SceneGame()));
			}
			fadeIn = false;
			break;
		default:
			break;
		}
	}
	else if(settingMenu->GetMenyuStart())
	{
		settingMenu->Updeat(&num);

		if (settingMenu->GetCurrentButton() != nullptr)
		{
			holdNum = settingMenu->GetCurrentButton()->GetMode();
		}

		switch (num)
		{
		case 0:
			sensitivity = static_cast<int>(SENSITIVITY_TYPE::HIGH);
			buttonSensitivity = num;
			break;
		case 1:
			sensitivity = static_cast<int>(SENSITIVITY_TYPE::NORMALE);
			buttonSensitivity = num;

			break;
		case 2:
			sensitivity = static_cast<int>(SENSITIVITY_TYPE::LOW);
			buttonSensitivity = num;

			break;
		case 3:
			fov = static_cast<int>(FOV_TYPE::HIGH);
			buttonFov = num;

			break;
		case 4:
			fov = static_cast<int>(FOV_TYPE::NORMALE);
			buttonFov = num;

			break;
		case 5:
			shake = static_cast<int>(SHAKE_TYPE::ON);
			buttonShake = num;

			break;
		case 6:
			shake = static_cast<int>(SHAKE_TYPE::OFF);
			buttonShake = num;

			break;
		case 7:
			drunkenness = static_cast<int>(DRUNKENNESS_TYPE::HIGH);
			buttonDrunkenness = num;
			shake = static_cast<int>(SHAKE_TYPE::OFF);
			buttonShake = 6;

			break;
		case 8:
			drunkenness = static_cast<int>(DRUNKENNESS_TYPE::LOW);
			buttonDrunkenness = num;

			break;
		case 9:
			drunkenness = static_cast<int>(DRUNKENNESS_TYPE::NONE);
			buttonDrunkenness = num;
			break;
		case 10:
			poseMenu->SetMenuStart(true);
			settingMenu->SetMenuStart(false);
			break;
		default:
			break;
		}

	}
	if (num != -1)
	{
		pauseSelectSE->Stop();
		pauseSelectSE->Play(false);
	}
	num = -1;
}

void Pose::Render(const RenderContext& rc)
{
	if (!onPose) return;
	sprPoseBack->Render(rc, 0, 0, 0, SCREEN_W, SCREEN_H, 0.0f, 1, 1, 1, alhpa);

	if (poseMenu->GetMenyuStart())
	{
		poseMenu->Render(rc, MENU::BACK_OFF, false);
	}
	else if (settingMenu->GetMenyuStart())
	{
		sprSettingBack->Render(rc, 0, 0, 0, SCREEN_W, SCREEN_H, 0.0f, 1, 1, 1, alhpa);

		int renderMode = 0;

		for (int i = 0; i < settingMenu->GetButtonManager()->GetButtonSize(); i++)
		{
			Button* b = settingMenu->GetButton(i);
			if (b->GetMode() == holdNum)
			{
				renderMode = 1;
			}

			if (b->GetMode() <= 2 && b->GetMode() == buttonSensitivity)
			{
				renderMode = 2;
			}
			else if (b->GetMode() <= 4 && b->GetMode() == buttonFov)
			{
				renderMode = 2;
			}
			else if (b->GetMode() <= 6 && b->GetMode() == buttonShake)
			{
				renderMode = 2;
			}
			else if (b->GetMode() <= 9 && b->GetMode() == buttonDrunkenness)
			{
				renderMode = 2;
			}

			if(i != 10)
			sprSetting->Render(rc, settingMenu->GetButton(i)->GetPos().x, settingMenu->GetButton(i)->GetPos().y + 5.0f, 0.0f,
				56, 56,
				56.0f * renderMode, 0, 56, 56,
				0.0f,
				1, 1, 1, alhpa);

			renderMode = 0;
		}
		settingMenu->Render(rc, MENU::BACK_OFF, false);
	}
}

bool Pose::PoseOff()
{
	if (lerp->GetAmount() <= 0.0f)
	{
		onPose = false;
		fadeIn = true;
		fadeOut = false;
		numBank = -1;
		poseMenu->SetMenuStart(false);
		settingMenu->SetMenuStart(false);
		// ÉJÅ[É\ÉãÇíÜâõÇ…ñﬂÇ∑
		POINT screenCenter{ (LONG)(SCREEN_W / 2), (LONG)(SCREEN_H / 2) };
		ClientToScreen(Graphics::Instance().GetWindowHandle(), &screenCenter);
		SetCursorPos(screenCenter.x, screenCenter.y);
		return true;
	}
	return false;
}
