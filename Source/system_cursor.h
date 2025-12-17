#ifndef SYSTEM_CURSOR_H_
#define SYSTEM_CURSOR_H_

#include <Windows.h>
#include <DirectXMath.h>

/**
 * @class SystemCursor
 * @brief Windowsシステムカーソルの表示・位置制御を管理するクラス
 *
 * Win32 APIを使用してシステムカーソルの表示/非表示、位置取得/設定を行います。
 */
class SystemCursor
{
public:
    /**
     * @brief システムカーソルを表示
     */
    static void Show();

    /**
     * @brief システムカーソルを非表示
     */
    static void Hide();

    /**
     * @brief システムカーソルの表示状態を取得
     * @return bool 表示中の場合true
     */
    static bool IsVisible();

    /**
     * @brief 現在のカーソル位置を取得（クライアント座標系）
     * @return DirectX::XMFLOAT2 カーソル位置
     */
    static DirectX::XMFLOAT2 GetPosition();

    /**
     * @brief カーソル位置を設定（クライアント座標系）
     * @param pos 設定する位置
     */
    static void SetPosition(const DirectX::XMFLOAT2& pos);

    /**
     * @brief カーソルを画面中央に移動
     */
    static void CenterCursor();

private:
    inline static bool is_visible_ = true; ///< カーソルの表示状態
};

#endif  // SYSTEM_CURSOR_H_