#ifndef WORLD_H_
#define WORLD_H_

#include <vector>
#include <memory>
#include <algorithm>
#include <DirectXMath.h>
#include "game_object.h"
#include "collider.h"
#include "rigidbody.h"
#include "System/ModelRenderer.h"

/**
 * @class World
 * @brief ゲームワールドを管理するシングルトンクラス
 *
 * ゲームオブジェクトの生成、更新、描画、物理演算、衝突判定を一元管理します。
 */
class World {
public:
    /**
     * @brief シングルトンインスタンスを取得
     * @return World& ワールドインスタンスへの参照
     */
    static World& Instance();

    GameObject* CreateObject(
        const char* model_filepath = nullptr,
        const DirectX::XMFLOAT3& pos = { 0.0f, 0.0f, 0.0f },
        const DirectX::XMFLOAT3& rotation = { 0.0f, 0.0f, 0.0f },
        const DirectX::XMFLOAT3& scale = { 1.0f, 1.0f, 1.0f });

    //GameObject* CreateObject(
    //    const DirectX::XMFLOAT3& pos = { 0.0f, 0.0f, 0.0f },
    //    const DirectX::XMFLOAT3& rotation = { 0.0f, 0.0f, 0.0f },
    //    const DirectX::XMFLOAT3& scale = { 1.0f, 1.0f, 1.0f });

    template<typename T>
    T* CreateObject(
        const char* model_filepath = nullptr,
        const DirectX::XMFLOAT3& pos = { 0.0f, 0.0f, 0.0f },
        const DirectX::XMFLOAT3& rotation = { 0.0f, 0.0f, 0.0f },
        const DirectX::XMFLOAT3& scale = { 1.0f, 1.0f, 1.0f });

    //template<typename T>
    //T* CreateObject(
    //    const DirectX::XMFLOAT3& pos = { 0.0f, 0.0f, 0.0f },
    //    const DirectX::XMFLOAT3& rotation = { 0.0f, 0.0f, 0.0f },
    //    const DirectX::XMFLOAT3& scale = { 1.0f, 1.0f, 1.0f });

    /**
     * @brief デフォルトのゲームオブジェクトを作成
     * @return GameObject* 作成されたオブジェクトへのポインタ
     */
    GameObject* CreateObject();

    /**
     * @brief ワールドの更新処理
     * @param elapsed_time 前フレームからの経過時間(秒)
     *
     * 物理演算、ゲームオブジェクトの更新、衝突判定、非アクティブオブジェクトの削除を実行します。
     */
    void Update(float elapsed_time);

    /**
     * @brief ワールド内の全オブジェクトを描画
     * @param rc レンダリングコンテキスト
     * @param model_renderer モデルレンダラー
     */
    void Render(const RenderContext& rc, ModelRenderer* model_renderer);

    /**
     * @brief ワールド内の全ゲームオブジェクトを削除
     */
    void Clear();

    /**
     * @brief ゲームオブジェクトの総数を取得
     * @return size_t オブジェクト数
     */
    size_t GetGameObjectCount() const;

    /**
     * @brief インデックスを指定してゲームオブジェクトを取得
     * @param index オブジェクトのインデックス
     * @return GameObject* オブジェクトへのポインタ（範囲外の場合はnullptr）
     */
    GameObject* GetGameObject(size_t index);

    /**
     * @brief ゲームオブジェクトを破棄
     * @param obj 破棄するオブジェクトへのポインタ
     */
    void DestroyGameObject(GameObject* obj);

    /**
     * @brief 重力加速度を設定
     * @param gravity 重力ベクトル（デフォルト: (0, -9.8, 0)）
     */
    void SetGravity(const DirectX::XMFLOAT3& gravity);

    /**
     * @brief 現在の重力加速度を取得
     * @return const DirectX::XMFLOAT3& 重力ベクトル
     */
    const DirectX::XMFLOAT3& GetGravity() const;

    /**
     * @brief コライダーのデバッグ描画を有効/無効化
     * @param enable trueで有効、falseで無効
     */
    void SetDebugDrawColliders(bool enable);

    /**
     * @brief コライダーのデバッグ描画が有効かどうか取得
     * @return bool 有効な場合true
     */
    bool GetDebugDrawColliders() const;

    /**
     * @brief デバッグ用のコライダー形状を描画
     * @param shape_renderer 図形描画用レンダラー
     */
    void DrawDebugPrimitives(ShapeRenderer* shape_renderer);

private:
    /**
     * @brief コンストラクタ（シングルトンのためprivate）
     */
    World();

    /**
     * @brief デストラクタ
     */
    ~World() = default;

    /**
     * @brief コピーコンストラクタ（削除）
     */
    World(const World&) = delete;

    /**
     * @brief 代入演算子（削除）
     */
    World& operator=(const World&) = delete;

    /**
     * @brief 非アクティブなオブジェクトをリストから削除
     */
    void RemoveInactiveObjects();

    /**
     * @brief 物理演算を適用（重力、抵抗力）
     * @param elapsed_time 経過時間
     */
    void ApplyPhysics(float elapsed_time);

    /**
     * @brief 全オブジェクト間の衝突判定と物理応答を実行
     */
    void DetectCollisions();

    /**
     * @struct CollisionPair
     * @brief 衝突ペアを表す構造体
     */
    struct CollisionPair {
        GameObject* obj_a; ///< オブジェクトA
        GameObject* obj_b; ///< オブジェクトB

        /**
         * @brief 衝突ペアの等価比較
         * @param other 比較対象
         * @return bool 同じペア（順序不問）の場合true
         */
        bool operator==(const CollisionPair& other) const;
    };

    bool debug_draw_colliders_ = _DEBUG; ///< デバッグ描画フラグ
    std::vector<CollisionPair> previous_collisions_; ///< 前フレームの衝突ペアリスト
    std::vector<std::unique_ptr<GameObject>> game_objects_; ///< 管理中のゲームオブジェクト
    DirectX::XMFLOAT3 gravity_; ///< 重力ベクトル
};

//GameObject* World::CreateObject(
//    const DirectX::XMFLOAT3& pos,
//    const DirectX::XMFLOAT3& rotation,
//    const DirectX::XMFLOAT3& scale) {
//    auto obj = std::make_unique<GameObject>();
//    GameObject* ptr = obj.get();
//    ptr->SetLocalPosition(pos);
//    ptr->SetAngleVector(rotation);
//    ptr->SetScaleVector(scale);
//    game_objects_.emplace_back(std::move(obj));
//    return ptr;
//}

template<typename T>
T* World::CreateObject(
    const char* model_filepath,
    const DirectX::XMFLOAT3& pos,
    const DirectX::XMFLOAT3& rotation,
    const DirectX::XMFLOAT3& scale) {
    static_assert(std::is_base_of<GameObject, T>::value,
        "基底クラスがGameObjectであるクラスを<>で指定してください");
    auto obj = std::make_unique<T>();
    T* ptr = obj.get();
	if (model_filepath) ptr->SetModel(model_filepath);
    ptr->SetLocalPosition(pos);
    ptr->SetAngle(rotation);
    ptr->SetScale(scale);
    game_objects_.emplace_back(std::move(obj));
    return ptr;
}

//template<typename T>
//T* World::CreateObject(const DirectX::XMFLOAT3& pos,
//    const DirectX::XMFLOAT3& rotation,
//    const DirectX::XMFLOAT3& scale) {
//    static_assert(std::is_base_of<GameObject, T>::value,
//        "基底クラスがGameObjectであるクラスを<>で指定してください");
//    auto obj = std::make_unique<T>();
//    T* ptr = obj.get();
//    ptr->SetLocalPosition(pos);
//	ptr->SetAngleVector(rotation);
//	ptr->SetScaleVector(scale);
//    game_objects_.emplace_back(std::move(obj));
//    return ptr;
//}

#endif  // WORLD_H_