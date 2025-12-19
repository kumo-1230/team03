/**
 * @file game_object.h
 * @brief ゲームオブジェクトの基底クラス定義
 */

#ifndef GAME_OBJECT_H_
#define GAME_OBJECT_H_

#include <memory>
#include <vector>
#include <DirectXMath.h>
#include "System/Model.h"
#include "System/ResourceManager.h"
#include "imgui_logger.h"
#include "collider.h"
#include "box_collider.h"
#include "sphere_collider.h"
#include "capsule_collider.h"
#include "aabb_collider.h"

 // 前方宣言
class Rigidbody;
class ModelRenderer;
struct RenderContext;

/**
 * @brief 階層関係のタイプ
 */
enum class HierarchyType {
    kNone,           ///< 親子関係なし
    kTransformOnly,  ///< トランスフォームのみ継承
    kFull           ///< 完全な親子関係（アクティブ状態も継承）
};

/**
 * @brief ゲームオブジェクトの基底クラス
 *
 * 3D空間におけるゲームオブジェクトを表現するクラス。
 * 位置、回転、スケールなどのトランスフォーム情報、
 * モデル、コライダー、リジッドボディなどのコンポーネントを管理する。
 */
class GameObject {
public:
    /// @brief デフォルトコンストラクタ
    GameObject()
        : position_({ 0.0f, 0.0f, 0.0f }),
        angle_({ 0.0f, 0.0f, 0.0f }),
        scale_({ 1.0f, 1.0f, 1.0f }),
        velocity_({ 0.0f, 0.0f, 0.0f }),
        rigidbody_(nullptr),
        parent_(nullptr),
        hierarchy_type_(HierarchyType::kNone),
        active_(true),
        elapsed_time_(0.0f)
    {
        UpdateTransform();
    }

    /**
     * @brief モデルパス、位置、回転、スケールを指定して構築
     * @param model_filepath モデルファイルのパス
     * @param pos 初期位置
     * @param rotation 初期回転（ラジアン）
     * @param scale 初期スケール
     */
    GameObject(const char* model_filepath,
        const DirectX::XMFLOAT3& pos = {0.0f, 0.0f, 0.0f }, 
        const DirectX::XMFLOAT3& rotation = { 0.0f, 0.0f, 0.0f },
        const DirectX::XMFLOAT3& scale = { 1.0f, 1.0f, 1.0f }) {
        if (model_filepath) SetModel(model_filepath);
        SetLocalPosition(pos);
        angle_ = rotation;
        scale_ = scale;
        UpdateTransform();
    }

    /**
     * @brief 位置、回転、スケールを指定して構築
     * @param pos 初期位置
     * @param rotation 初期回転（ラジアン）
     * @param scale 初期スケール
     */
    //GameObject(
    //    const DirectX::XMFLOAT3& pos,
    //    const DirectX::XMFLOAT3& rotation = { 0.0f, 0.0f, 0.0f },
    //    const DirectX::XMFLOAT3& scale = { 1.0f, 1.0f, 1.0f }) {
    //    SetLocalPosition(pos);
    //    angle_ = rotation;
    //    scale_ = scale;
    //    UpdateTransform();
    //}

    /// @brief デストラクタ
    virtual ~GameObject();

    /**
     * @brief 更新処理 (UpdatePosition()前)
     * @param elapsed_time 前フレームからの経過時間（秒）
     */
    virtual void Update(float elapsed_time) {};

    /**
	 * @brief 座標更新処理 (Update()後)
     * @param elapsed_time 前フレームからの経過時間（秒）
     */
    void UpdatePosition(float elapsed_time);

    /**
     * @brief 描画処理
     * @param rc レンダーコンテキスト
     * @param model_renderer モデルレンダラー
     */
    virtual void Render(const RenderContext& rc, ModelRenderer* model_renderer);

    /**
     * @brief トランスフォーム行列を更新
     *
     * 位置、回転、スケールから変換行列を再計算する。
     * SetPosition等のトランスフォーム変更メソッドから自動的に呼ばれる。
     */
    void UpdateTransform();

    // ========================================
    // 親子関係
    // ========================================

    /**
     * @brief 親オブジェクトを設定
     * @param parent 親オブジェクト
     * @param keep_world_position trueの場合、ワールド座標を維持する
     */
    void SetParent(GameObject* parent, bool keep_world_position = false);

    /**
     * @brief トランスフォームのみ継承する親子関係を設定
     * @param parent 親オブジェクト
     * @param keep_world_position trueの場合、ワールド座標を維持する
     */
    void SetParentTransformOnly(GameObject* parent, bool keep_world_position = false);

    /**
     * @brief 親から切り離す
     */
    void DetachFromParent();

    /**
     * @brief 親オブジェクトを取得
     * @return 親オブジェクト（なければnullptr）
     */
    GameObject* GetParent() const { return parent_; }

    /**
     * @brief 子オブジェクトのリストを取得
     * @return 子オブジェクトのリスト
     */
    const std::vector<GameObject*>& GetChildren() const { return children_; }

    // ========================================
    // 位置・回転・スケール
    // ========================================

    /**
     * @brief ローカル座標を設定
     * @param pos ローカル座標
     */
    void SetLocalPosition(const DirectX::XMFLOAT3& pos);

    /**
     * @brief ローカル座標を設定
     * @param x X座標
     * @param y Y座標
     * @param z Z座標
     */
    void SetLocalPosition(float x, float y, float z);

    /**
     * @brief ローカル座標を設定
     * @param v 座標ベクトル
     */
    void SetLocalPositionVector(DirectX::FXMVECTOR v);

    /**
     * @brief 座標を設定（SetLocalPositionのエイリアス）
     * @param pos 座標
     */
    void SetPosition(const DirectX::XMFLOAT3& pos) { SetLocalPosition(pos); }

    /**
     * @brief 座標を設定（SetLocalPositionのエイリアス）
     * @param x X座標
     * @param y Y座標
     * @param z Z座標
     */
    void SetPosition(float x, float y, float z) { SetLocalPosition(x, y, z); }

    /**
     * @brief 座標を設定（SetLocalPositionのエイリアス）
     * @param v 座標ベクトル
     */
    void SetPositionVector(DirectX::FXMVECTOR v) { SetLocalPositionVector(v); }

    /**
     * @brief ローカル座標を取得
     * @return ローカル座標
     */
    const DirectX::XMFLOAT3& GetLocalPositionFloat3() const { return position_; }

    /**
     * @brief ワールド座標を設定
     * @param world_pos ワールド座標
     */
    void SetWorldPosition(const DirectX::XMFLOAT3& world_pos);

    /**
     * @brief ワールド座標を設定
     * @param x X座標
     * @param y Y座標
     * @param z Z座標
     */
    void SetWorldPosition(float x, float y, float z);

    /**
     * @brief ワールド座標を設定
     * @param v 座標ベクトル
     */
    void SetWorldPositionVector(DirectX::FXMVECTOR v);

    /**
     * @brief ワールド座標を取得
     * @return ワールド座標
     */
    DirectX::XMFLOAT3 GetWorldPositionFloat3() const;

    /**
     * @brief 回転を設定（ラジアン）
     * @param v 回転ベクトル（X, Y, Z軸周りの回転角）
     */
    void SetAngleVector(DirectX::FXMVECTOR v);

    /**
     * @brief 回転を設定（ラジアン）
     * @param angle 回転角（X, Y, Z軸周りの回転角）
     */
    void SetAngle(const DirectX::XMFLOAT3& angle);

    /**
     * @brief 回転を設定（ラジアン）
     * @param x X軸周りの回転角
     * @param y Y軸周りの回転角
     * @param z Z軸周りの回転角
     */
    void SetAngle(float x, float y, float z);

    /**
     * @brief 回転を取得（ラジアン）
     * @return 回転角（X, Y, Z軸周りの回転角）
     */
    const DirectX::XMFLOAT3& GetAngle() const { return angle_; }

    /**
     * @brief 回転を設定（度数法）
     * @param deg 回転角（度）
     */
    void SetAngleDegree(const DirectX::XMFLOAT3& deg);

    /**
     * @brief 回転を設定（度数法）
     * @param x X軸周りの回転角（度）
     * @param y Y軸周りの回転角（度）
     * @param z Z軸周りの回転角（度）
     */
    void SetAngleDegree(float x, float y, float z);

    /**
     * @brief 回転を取得（度数法）
     * @return 回転角（度）
     */
    DirectX::XMFLOAT3 GetAngleDegree() const;

    /**
     * @brief スケールを設定
     * @param v スケールベクトル
     */
    void SetScaleVector(DirectX::FXMVECTOR v);

    /**
     * @brief スケールを設定
     * @param scale スケール
     */
    void SetScale(const DirectX::XMFLOAT3& scale) { scale_ = scale; UpdateTransform(); }

    /**
     * @brief スケールを設定
     * @param x Xスケール
     * @param y Yスケール
     * @param z Zスケール
     */
    void SetScale(float x, float y, float z) { scale_ = { x, y, z }; UpdateTransform(); }

    /**
     * @brief スケールを取得
     * @return スケール
     */
    const DirectX::XMFLOAT3& GetScale() const { return scale_; }

    // ========================================
    // 移動・回転操作
    // ========================================

    /**
     * @brief オブジェクトを移動
     * @param offset 移動量
     */
    void Move(const DirectX::XMFLOAT3& offset);

    /**
     * @brief オブジェクトを移動
     * @param x X方向の移動量
     * @param y Y方向の移動量
     * @param z Z方向の移動量
     */
    void Move(float x, float y, float z);

    /**
     * @brief オブジェクトを移動
     * @param v 移動量ベクトル
     */
    void MoveVector(DirectX::FXMVECTOR v);

    /**
     * @brief 前方に移動
     * @param distance 移動距離
     */
    void MoveForward(float distance);

    /**
     * @brief 右方向に移動
     * @param distance 移動距離
     */
    void MoveRight(float distance);

    /**
     * @brief 上方向に移動
     * @param distance 移動距離
     */
    void MoveUp(float distance);

    /**
     * @brief 回転を加算（ラジアン）
     * @param delta 回転量
     */
    void Rotate(const DirectX::XMFLOAT3& delta);

    /**
     * @brief 回転を加算（ラジアン）
     * @param x X軸周りの回転量
     * @param y Y軸周りの回転量
     * @param z Z軸周りの回転量
     */
    void Rotate(float x, float y, float z);

    /**
     * @brief 回転を加算（度数法）
     * @param x X軸周りの回転量（度）
     * @param y Y軸周りの回転量（度）
     * @param z Z軸周りの回転量（度）
     */
    void RotateDegree(float x, float y, float z);

    /**
     * @brief 指定した位置を向く
     * @param target ターゲット位置
     */
    void LookAt(const DirectX::XMFLOAT3& target);

    /**
     * @brief 指定した位置を向く
     * @param target ターゲット位置（XMVECTOR）
     */
    void LookAtVector(DirectX::FXMVECTOR target);

    // ========================================
    // 方向ベクトル
    // ========================================

    /**
     * @brief 前方ベクトルを取得
     * @return 前方ベクトル（正規化済み）
     */
    DirectX::XMVECTOR GetForwardVector() const;

    /**
     * @brief 右方向ベクトルを取得
     * @return 右方向ベクトル（正規化済み）
     */
    DirectX::XMVECTOR GetRightVector() const;

    /**
     * @brief 上方向ベクトルを取得
     * @return 上方向ベクトル（正規化済み）
     */
    DirectX::XMVECTOR GetUpVector() const;

    /**
     * @brief 前方ベクトルを取得（XMFLOAT3版）
     * @return 前方ベクトル（正規化済み）
     */
    DirectX::XMFLOAT3 GetForwardFloat3() const;

    /**
     * @brief 右方向ベクトルを取得（XMFLOAT3版）
     * @return 右方向ベクトル（正規化済み）
     */
    DirectX::XMFLOAT3 GetRightFloat3() const;

    /**
     * @brief 上方向ベクトルを取得（XMFLOAT3版）
     * @return 上方向ベクトル（正規化済み）
     */
    DirectX::XMFLOAT3 GetUpFloat3() const;

    // ========================================
    // 速度
    // ========================================

    /**
     * @brief 速度を設定
     * @param v 速度ベクトル
     */
    void SetVelocityVector(DirectX::FXMVECTOR v);

    /**
     * @brief 速度を設定
     * @param vel 速度
     */
    void SetVelocity(const DirectX::XMFLOAT3& vel) { velocity_ = vel; }

    /**
     * @brief 速度を設定
     * @param x X方向の速度
     * @param y Y方向の速度
     * @param z Z方向の速度
     */
    void SetVelocity(float x, float y, float z) { velocity_ = { x, y, z }; }

    /**
     * @brief 速度を取得
     * @return 速度
     */
    const DirectX::XMFLOAT3& GetVelocityFloat3() const { return velocity_; }

    /**
     * @brief 速度を加算
     * @param vel 加算する速度
     */
    void AddVelocity(const DirectX::XMFLOAT3& vel);

    /**
     * @brief 速度を加算
     * @param x X方向の速度
     * @param y Y方向の速度
     * @param z Z方向の速度
     */
    void AddVelocity(float x, float y, float z);

    /**
     * @brief 速度を加算
     * @param v 加算する速度ベクトル
     */
    void AddVelocityVector(DirectX::FXMVECTOR v);

    /**
     * @brief 移動を停止（速度を0にする）
     */
    void StopMovement();

    /**
     * @brief 速度の大きさを取得
     * @return 速度の大きさ
     */
    float GetSpeed() const;

    /**
     * @brief 速度の大きさを設定（方向は保持）
     * @param speed 速度の大きさ
     */
    void SetSpeed(float speed);

    // ========================================
    // 距離・方向計算
    // ========================================

    /**
     * @brief 他のオブジェクトとの距離を取得
     * @param other 対象オブジェクト
     * @return 距離
     */
    float GetDistanceTo(const GameObject* other) const;

    /**
     * @brief 指定した点との距離を取得
     * @param point 対象の点
     * @return 距離
     */
    float GetDistanceTo(const DirectX::XMFLOAT3& point) const;

    /**
     * @brief 他のオブジェクトとの距離の二乗を取得
     * @param other 対象オブジェクト
     * @return 距離の二乗
     * @note 平方根の計算を省略できるため高速
     */
    float GetDistanceSquaredTo(const GameObject* other) const;

    /**
     * @brief 他のオブジェクトへの方向ベクトルを取得
     * @param other 対象オブジェクト
     * @return 方向ベクトル（正規化済み）
     */
    DirectX::XMVECTOR GetDirectionTo(const GameObject* other) const;

    /**
     * @brief 指定した点への方向ベクトルを取得
     * @param point 対象の点
     * @return 方向ベクトル（正規化済み）
     */
    DirectX::XMVECTOR GetDirectionTo(const DirectX::XMFLOAT3& point) const;

    /**
     * @brief 他のオブジェクトが範囲内にあるか判定
     * @param other 対象オブジェクト
     * @param range 判定範囲
     * @return 範囲内ならtrue
     */
    bool IsInRange(const GameObject* other, float range) const;

    // ========================================
    // トランスフォーム
    // ========================================

    /**
     * @brief ワールド変換行列を取得
     * @return ワールド変換行列
     */
    DirectX::XMFLOAT4X4 GetWorldTransformFloat4X4() const;

    /**
     * @brief ワールド変換行列を取得（XMMATRIX版）
     * @return ワールド変換行列
     */
    DirectX::XMMATRIX GetWorldTransformMatrix() const;

    /**
     * @brief 変換行列を設定
     * @param transform 変換行列
     */
    void SetTransform(const DirectX::XMFLOAT4X4& transform);

    /**
     * @brief ローカル変換行列を取得
     * @return ローカル変換行列
     */
    const DirectX::XMFLOAT4X4& GetTransform() const { return transform_; }

    // ========================================
    // モデル
    // ========================================

    /**
     * @brief モデルを設定
     * @param filepath モデルファイルのパス
     */
    void SetModel(const char* filepath);

    /**
     * @brief モデルを設定
     * @param model モデルの共有ポインタ
     */
    void SetModel(const std::shared_ptr<Model>& model);

    /**
     * @brief モデルを取得
     * @return モデルの共有ポインタ
     */
    std::shared_ptr<Model> GetModel() const { return model_; }

    // ========================================
    // コライダー
    // ========================================

    /**
     * @brief コライダーを追加
     * @tparam T コライダーの型（Colliderの派生クラス）
     * @tparam Args コンストラクタ引数の型
     * @param args コンストラクタに渡す引数
     * @return 追加されたコライダー
     */
    template<typename T, typename... Args>
    T* AddCollider(Args&&... args) {
        static_assert(std::is_base_of<Collider, T>::value,
            "T must be derived from Collider");

        T* collider = new T(std::forward<Args>(args)...);
        collider->SetOwner(this);
        colliders_.push_back(collider);

        return collider;
    }

    /**
     * @brief Sphere コライダーを追加
     * @param radius 球の半径
     * @return 追加された SphereCollider
     */
    SphereCollider* AddSphereCollider(float radius = 1.0f) {
        SphereCollider* collider = new SphereCollider(radius);
        collider->SetOwner(this);
        colliders_.push_back(collider);
        return collider;
    }

    /**
     * @brief Box コライダーを追加（サイズ指定）
     * @param size ボックスのサイズ（x, y, z）
     * @return 追加された BoxCollider
     */
    BoxCollider* AddBoxCollider(const DirectX::XMFLOAT3& size) {
        BoxCollider* collider = new BoxCollider(size);
        collider->SetOwner(this);
        colliders_.push_back(collider);
        return collider;
    }

    /**
     * @brief Box コライダーを追加（各軸指定）
     * @param x X サイズ
     * @param y Y サイズ
     * @param z Z サイズ
     * @return 追加された BoxCollider
     */
    BoxCollider* AddBoxCollider(float x, float y, float z) {
        return AddBoxCollider(DirectX::XMFLOAT3(x, y, z));
    }

    /**
     * @brief AABB コライダーを追加（サイズ指定）
     * @param size AABB のサイズ（x, y, z）
     * @return 追加された AABBCollider
     */
    AABBCollider* AddAABBCollider(const DirectX::XMFLOAT3& size) {
        AABBCollider* collider = new AABBCollider(size);
        collider->SetOwner(this);
        colliders_.push_back(collider);
        return collider;
    }

    /**
     * @brief AABB コライダーを追加（各軸指定）
     * @param x X サイズ
     * @param y Y サイズ
     * @param z Z サイズ
     * @return 追加された AABBCollider
     */
    AABBCollider* AddAABBCollider(float x, float y, float z) {
        return AddAABBCollider(DirectX::XMFLOAT3(x, y, z));
    }

    /**
     * @brief Capsule コライダーを追加
     * @param radius カプセルの半径
     * @param height カプセルの高さ
     * @return 追加された CapsuleCollider
     */
    CapsuleCollider* AddCapsuleCollider(float radius = 0.5f, float height = 2.0f) {
        CapsuleCollider* collider = new CapsuleCollider(radius, height);
        collider->SetOwner(this);
        colliders_.push_back(collider);
        return collider;
    }

    /**
     * @brief コライダーを削除
     * @param collider 削除するコライダー
     */
    void RemoveCollider(Collider* collider);

    /**
     * @brief すべてのコライダーを削除
     */
    void RemoveAllColliders();

    /**
     * @brief コライダーのリストを取得
     * @return コライダーのリスト
     */
    const std::vector<Collider*>& GetColliders() const { return colliders_; }

    /**
     * @brief 指定したインデックスのコライダーを取得
     * @param index インデックス
     * @return コライダー（存在しない場合はnullptr）
     */
    Collider* GetCollider(size_t index = 0) const {
        return index < colliders_.size() ? colliders_[index] : nullptr;
    }

    /**
     * @brief コライダーの数を取得
     * @return コライダーの数
     */
    size_t GetColliderCount() const { return colliders_.size(); }

    virtual void OnCollisionEnter(GameObject* other) {}
    virtual void OnCollisionStay(GameObject* other) {}
    virtual void OnCollisionExit(GameObject* other) {}

    // ========================================
    // リジッドボディ
    // ========================================

    /**
     * @brief リジッドボディを追加
     * @return 追加されたリジッドボディ
     */
    Rigidbody* AddRigidbody();

    /**
     * @brief リジッドボディを設定
     * @param rigidbody リジッドボディ
     */
    void SetRigidbody(Rigidbody* rigidbody);

    /**
     * @brief リジッドボディを取得
     * @return リジッドボディ（なければnullptr）
     */
    Rigidbody* GetRigidbody() const { return rigidbody_; }

    /**
     * @brief リジッドボディを削除
     */
    void RemoveRigidbody();

    // ========================================
    // アクティブ状態
    // ========================================

    /**
     * @brief アクティブ状態を取得
     * @return アクティブならtrue
     */
    bool IsActive() const { return active_; }

    /**
     * @brief アクティブ状態を設定
     * @param active アクティブ状態
     */
    void SetActive(bool active) { active_ = active; }

    /**
     * @brief 階層を含めたアクティブ状態を取得
     * @return 自身と親が全てアクティブならtrue
     */
    bool IsActiveInHierarchy() const;

    DirectX::XMVECTOR GetWorldPositionVector() const;

    /**
     * @brief オブジェクトを破棄
     */
    void Destroy();

    // ========================================
    // その他
    // ========================================

    /**
     * @brief 経過時間を取得
     * @return 経過時間（秒）
     */
    float GetElapsedTime() const { return elapsed_time_; }

    /**
     * @brief 階層タイプを取得
     * @return 階層タイプ
     */
    HierarchyType GetHierarchyType() const { return hierarchy_type_; }

	template<typename... Args>
    inline void Log(Args&&... args) const {
        ImGuiLogger::Instance().AddLog(std::forward<Args>(args)...);
    }

protected:
    // トランスフォーム
    DirectX::XMFLOAT3 position_ = { 0.0f, 0.0f, 0.0f };  ///< ローカル座標
    DirectX::XMFLOAT3 angle_ = { 0.0f, 0.0f, 0.0f };      ///< 回転角（ラジアン）
    DirectX::XMFLOAT3 scale_ = { 1.0f, 1.0f, 1.0f };      ///< スケール
    DirectX::XMFLOAT4X4 transform_ = {                     ///< ローカル変換行列
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1
    };

    // 物理
    DirectX::XMFLOAT3 velocity_ = { 0.0f, 0.0f, 0.0f };   ///< 速度

    // コンポーネント
    std::vector<Collider*> colliders_;  ///< コライダーのリスト
    Rigidbody* rigidbody_ = nullptr;    ///< リジッドボディ

    // モデル
    std::shared_ptr<Model> model_;  ///< 3Dモデル

    // 階層構造
    GameObject* parent_ = nullptr;              ///< 親オブジェクト
    std::vector<GameObject*> children_;         ///< 子オブジェクトのリスト
    HierarchyType hierarchy_type_ = HierarchyType::kNone;  ///< 階層タイプ

    // 状態
    bool active_ = true;          ///< アクティブ状態
    float elapsed_time_ = 0.0f;   ///< 経過時間
};

#endif  // GAME_OBJECT_H_