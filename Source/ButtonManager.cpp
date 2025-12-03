#include "ButtonManager.h"
#include "System/Graphics.h"
#include <ranges>

//ボタンの追加
void ButtonManager::SetButton(const char* fileName, DirectX::XMFLOAT2 p, DirectX::XMFLOAT2 l, DirectX::XMFLOAT2 sp, DirectX::XMFLOAT2 sl, int la, int n,bool v)
{
	auto newButton = std::make_unique<Button>(fileName, p, l, sp, sl, la, n, v);

	if (buttons.size() == 0)
	{
		buttons.emplace_back(std::move(newButton));
		return;
	}
	else
	{
		for (auto it = buttons.begin();it != buttons.end();it++)
		{
			if (la < (*it)->GetLayer())
			{
				buttons.insert(it, std::move(newButton));
				return;
			}
		}
		buttons.emplace_back(std::move(newButton));
		return;
	}
}

void ButtonManager::SetButton(const char* fileName, DirectX::XMFLOAT2 p, DirectX::XMFLOAT2 l, DirectX::XMFLOAT2 sp, DirectX::XMFLOAT2 sl, DirectX::XMFLOAT2 hp, DirectX::XMFLOAT2 hl, int la, int n, bool v)
{
	auto newButton = std::make_unique<Button>(fileName, p, l, sp, sl, hp, hl, la, n, v);

	if (buttons.size() == 0)
	{
		buttons.emplace_back(std::move(newButton));
		return;
	}
	else
	{
		for (auto it = buttons.begin(); it != buttons.end(); it++)
		{
			if (la < (*it)->GetLayer())
			{
				buttons.insert(it, std::move(newButton));
				return;
			}
		}
		buttons.emplace_back(std::move(newButton));
		return;
	}
}

void ButtonManager::Update()
{
	//現在のカーソル位置を取得
	POINT cursor;
	GetCursorPos(&cursor);

	//クライアント座標に変換
	ScreenToClient(Graphics::Instance().GetWindowHandle(), &cursor);

	float size = 10.0f; // 例えば2ピクセル四方
	DirectX::XMFLOAT2 pos = { ((float)cursor.x) - size * 0.5f, ((float)cursor.y) - size * 0.5f };
	DirectX::XMFLOAT2 len = { size, size }; // 幅・高さ
	bool anyHit = false;

	for (auto& b : std::ranges::reverse_view(buttons))
	{
		if (!b->GetValid())
			continue;

		if (b->HitButton(pos, len))
		{
			anyHit = true;

			// まだボタンが押されていない → 新規押下
			if (!onButton.holdButton && !onButton.downButton)
			{
				onButton.downButton = true;
				onButton.holdButton = false;
				onButton.upButton = false;

				currentButton = b.get(); // 押したボタンを記録
				nowHitButtonPos = b->GetPos();
				nowHitButtonLeng = b->Getlength();
			}
			else // すでに押されている → 押しっぱなし
			{
				onButton.downButton = false;
				onButton.holdButton = true;
				onButton.upButton = false;
			}

			return; // 当たったボタンが見つかったので他は見ない
		}
	}

	// どのボタンにも当たらなかった場合
	if (!anyHit)
	{
		if (onButton.holdButton)
		{
			// 離された
			onButton.upButton = true;
			onButton.holdButton = false;
			onButton.downButton = false;

			if (currentButton)
			{
				nowHitButtonPos = currentButton->GetPos();
				nowHitButtonLeng.x = 0;
				currentButton = nullptr; // 離したので解除
			}
		}
		else
		{
			// 何も押していない状態
			onButton.upButton = false;
			onButton.holdButton = false;
			onButton.downButton = false;
		}
	}

}


void ButtonManager::Render(const RenderContext& rc,bool stop)
{
	//現在のカーソル位置を取得
	POINT cursor;
	GetCursorPos(&cursor);

	//クライアント座標に変換
	ScreenToClient(Graphics::Instance().GetWindowHandle(), &cursor);

	float size = 10.0f; // 例えば2ピクセル四方
	DirectX::XMFLOAT2 pos = { ((float)cursor.x) - size * 0.5f, ((float)cursor.y) - size * 0.5f };
	DirectX::XMFLOAT2 len = { size, size }; // 幅・高さ

	for (auto& b : std::ranges::reverse_view(buttons))
	{
		if (!stop)
		{
			b->SetAlphaChange(alphaChange);

			if (alphaChange)
			{
				b->SetAlphaMax(alphaMax);
				b->SetProgress(progress);
			}

			if (b->GetValid() == true)
			{
				if (b->HitButton(pos, len))
				{
					b->SetRenderMode(BUTTON_R_MODE::NORMAL);
					b->SetSizeOffset(-1.0f);
				}
				else
				{
					b->SetRenderMode(BUTTON_R_MODE::HALF_INVISIBLE);
					b->SetSizeOffset(0.0f);
				}
			}
		}
		b->Render(rc);
	}
}

//すべてのボタンでどれに当たったか
int ButtonManager::HitButton()
{
	//現在のカーソル位置を取得
	POINT cursor;
	GetCursorPos(&cursor);

	//クライアント座標に変換
	ScreenToClient(Graphics::Instance().GetWindowHandle(), &cursor);

	float size = 10.0f; // 例えば2ピクセル四方
	DirectX::XMFLOAT2 pos = { ((float)cursor.x) - size * 0.5f, ((float)cursor.y) - size * 0.5f };
	DirectX::XMFLOAT2 len = { size, size }; // 幅・高さ

	for (auto& b : std::ranges::reverse_view(buttons))
	{
		if (b->GetValid() == true)
		{
			if (b->HitButton(pos, len))
			{
				return b->GetMode();
			}
		}
	}
	return -1;
}
