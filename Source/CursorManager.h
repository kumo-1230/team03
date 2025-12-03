#pragma once

#include <Windows.h>
#include <DirectXMath.h>
#include "System/Graphics.h"

class CursorManager
{
private:
	inline static bool currentShow = true;
public:


	static void ChangeCursorShow(bool b)
	{
		if (b != currentShow)
		{
			if (b == true)
			{
				ShowCursor(true); // カーソルを見えるようにする
				currentShow = b;
			}
			else
			{
				ShowCursor(false); // カーソルを見えないようにする
				currentShow = b;
			}
		}
	}
	
	static DirectX::XMFLOAT2 GetCursorPosition()
	{
		//現在のカーソル位置を取得
		POINT cursor;
		GetCursorPos(&cursor);

		//クライアント座標に変換
		ScreenToClient(Graphics::Instance().GetWindowHandle(), &cursor);

		return { static_cast<float>(cursor.x),static_cast<float>(cursor.y) };
	}

};