/**
 * @file game_object.h
 * @brief ゲームオブジェクトの基底クラス定義
 */

#ifndef GAME_OBJECT_H_
#define GAME_OBJECT_H_

#include <DirectXMath.h>
#include <memory>
#include <string>
#include <vector>
#include "System/ModelRenderer.h"
#include "System/ResourceManager.h"

 /**
  * @class GameObject
  * @brief ゲーム内のオブジェクトを表す基底クラス
  *
  * 3D空間における位置、回転、スケール、速度などの基本的な属性を持ち、
  * 親子関係によるトランスフォームの階層構造をサポートします。
  */
class GameObject {
public:
    /**
     * @enum Tag
     * @brief ゲームオブジェクトの種類を識別するタグ
     */
    enum class Tag {
        kDefault,       // タグなし
        kPlayer,        // プレイヤー
        kEnemy,         // 敵
        kBullet,        // 弾
        kItem,          // アイテム
		kWall,          // 壁
		kInteractive,   // インタラクティブオブジェクト
        kObstacle,      // 障害物
        kTerrain,       // 地形
        kEffect,        // エフェクト
        kTrigger,       // トリガー領域
        kCamera,        // カメラ
        kLight,         // ライト
    };

    /**
     * @enum HierarchyType
     * @brief 親子関係のタイプを定義
     */
    enum class HierarchyType {
        None,           ///< 親子関係なし
        TransformOnly,  ///< 座標のみ継承（Active状態は独立）
        Full           ///< 完全な親子関係（Active状態なども継承）
    };

    /**
     * @brief デフォルトコンストラクタ
     *
     * すべての属性を初期値で初期化します。
     */
    GameObject()
        : position_(0.0f, 0.0f, 0.0f),
        angle_(0.0f, 0.0f, 0.0f),
        scale_(1.0f, 1.0f, 1.0f),
        velocity_(0.0f, 0.0f, 0.0f),
        elapsed_time_(0.0f),
        active_(true),
        tag_(Tag::kDefault),
        model_(nullptr),
        parent_(nullptr),
        hierarchy_type_(HierarchyType::None) {
    }

    /**
     * @brief モデルファイルパスを指定するコンストラクタ
     * @param model_filepath モデルファイルのパス
     * @param pos 初期位置
     * @param angle 初期回転角（ラジアン）
     * @param scale 初期スケール
     * @param velocity 初期速度
     * @param active 初期アクティブ状態
     */
    GameObject(const char* model_filepath,
        const DirectX::XMFLOAT3& pos = { 0.0f, 0.0f, 0.0f },
        const DirectX::XMFLOAT3& angle = { 0.0f, 0.0f, 0.0f },
        const DirectX::XMFLOAT3& scale = { 1.0f, 1.0f, 1.0f },
        const DirectX::XMFLOAT3& velocity = { 0.0f, 0.0f, 0.0f },
        bool active = true, Tag tag = Tag::kDefault)
        : position_(pos),
        angle_(angle),
        scale_(scale),
        velocity_(velocity),
        elapsed_time_(0.0f),
        active_(active),
        tag_(tag),
        parent_(nullptr),
        hierarchy_type_(HierarchyType::None) {
        model_ = ResourceManager::Instance().LoadModel(model_filepath);
        UpdateTransform();
    }

    /**
     * @brief モデルオブジェクトを指定するコンストラクタ
     * @param model モデルオブジェクトの共有ポインタ
     * @param pos 初期位置
     * @param angle 初期回転角（ラジアン）
     * @param scale 初期スケール
     * @param velocity 初期速度
     * @param active 初期アクティブ状態
     */
    GameObject(const std::shared_ptr<Model>& model,
        const DirectX::XMFLOAT3& pos = { 0.0f, 0.0f, 0.0f },
        const DirectX::XMFLOAT3& angle = { 0.0f, 0.0f, 0.0f },
        const DirectX::XMFLOAT3& scale = { 1.0f, 1.0f, 1.0f },
        const DirectX::XMFLOAT3& velocity = { 0.0f, 0.0f, 0.0f },
        bool active = true, Tag tag = Tag::kDefault)
        : position_(pos),
        angle_(angle),
        scale_(scale),
        velocity_(velocity),
        elapsed_time_(0.0f),
        active_(active),
        tag_(tag),
        model_(model),
        parent_(nullptr),
        hierarchy_type_(HierarchyType::None) {
        UpdateTransform();
    }

    /**
     * @brief 仮想デストラクタ
     *
     * 子オブジェクトの親参照をクリアします。
     */
    virtual ~GameObject() {
        for (GameObject* child : children_) {
            if (child) {
                child->parent_ = nullptr;
            }
        }
    }

    /**
     * @brief 更新処理
     * @param elapsed_time 経過時間（秒）
     *
     * オブジェクトの位置を速度に基づいて更新し、子オブジェクトも再帰的に更新します。
     */
    virtual void Update(float elapsed_time) {
        if (!IsActiveInHierarchy()) return;

        elapsed_time_ += elapsed_time;

        position_.x += velocity_.x * elapsed_time;
        position_.y += velocity_.y * elapsed_time;
        position_.z += velocity_.z * elapsed_time;

        UpdateTransform();

        for (GameObject* child : children_) {
            if (child) {
                child->Update(elapsed_time);
            }
        }
    }

    /**
     * @brief 描画処理
     * @param rc レンダリングコンテキスト
     * @param model_renderer モデルレンダラー
     *
     * オブジェクトと子オブジェクトを再帰的に描画します。
     */
    virtual void Render(const RenderContext& rc, ModelRenderer* model_renderer) {
        if (!IsActiveInHierarchy() || !model_) return;

        model_renderer->Draw(ShaderId::PBR, model_);

        for (GameObject* child : children_) {
            if (child) {
                child->Render(rc, model_renderer);
            }
        }
    }

    /**
     * @brief タグを設定
     * @param tag オブジェクトのタグ
     */
    void SetTag(Tag tag) { tag_ = tag; }

    /**
     * @brief タグを取得
     * @return オブジェクトのタグ
     */
    Tag GetTag() const { return tag_; }

    /**
     * @brief 指定したタグを持つか判定
     * @param tag 判定するタグ
     * @return 一致する場合true
     */
    bool HasTag(Tag tag) const { return tag_ == tag; }

    /**
     * @brief 複数のタグのいずれかを持つか判定
     * @param tags 判定するタグの配列
     * @return いずれかのタグと一致する場合true
     */
    bool HasAnyTag(const std::vector<Tag>& tags) const {
        for (Tag tag : tags) {
            if (tag_ == tag) return true;
        }
        return false;
    }

    /**
     * @brief 親子関係を設定（座標のみ継承）
     * @param parent 親オブジェクト
     * @param keepWorldPosition trueの場合、現在のワールド座標を維持
     *
     * 親の移動・回転・スケールに追従しますが、Active状態は独立します。
     */
    void SetParentTransformOnly(GameObject* parent, bool keepWorldPosition = false) {
        if (keepWorldPosition && parent) {
            DirectX::XMFLOAT3 worldPos = GetWorldPosition();
            DirectX::XMFLOAT3 worldRot = angle_;
            DirectX::XMFLOAT3 worldScale = scale_;

            DetachFromParent();

            parent_ = parent;
            hierarchy_type_ = HierarchyType::TransformOnly;
            parent_->children_.push_back(this);

            DirectX::XMMATRIX parentWorldInv = DirectX::XMMatrixInverse(nullptr, parent->GetWorldTransformM());
            DirectX::XMVECTOR worldPosV = DirectX::XMLoadFloat3(&worldPos);
            DirectX::XMVECTOR localPosV = DirectX::XMVector3TransformCoord(worldPosV, parentWorldInv);
            DirectX::XMStoreFloat3(&position_, localPosV);
        }
        else {
            DetachFromParent();

            parent_ = parent;
            hierarchy_type_ = HierarchyType::TransformOnly;

            if (parent_) {
                parent_->children_.push_back(this);
            }
        }

        UpdateTransform();
    }

    /**
     * @brief 親子関係を設定（完全な親子関係）
     * @param parent 親オブジェクト
     * @param keepWorldPosition trueの場合、現在のワールド座標を維持
     *
     * 親の移動・回転・スケール・Active状態すべてに追従します。
     */
    void SetParent(GameObject* parent, bool keepWorldPosition = false) {
        if (keepWorldPosition && parent) {
            DirectX::XMFLOAT3 worldPos = GetWorldPosition();
            DirectX::XMFLOAT3 worldRot = angle_;
            DirectX::XMFLOAT3 worldScale = scale_;

            DetachFromParent();

            parent_ = parent;
            hierarchy_type_ = HierarchyType::Full;
            parent_->children_.push_back(this);

            DirectX::XMMATRIX parentWorldInv = DirectX::XMMatrixInverse(nullptr, parent->GetWorldTransformM());
            DirectX::XMVECTOR worldPosV = DirectX::XMLoadFloat3(&worldPos);
            DirectX::XMVECTOR localPosV = DirectX::XMVector3TransformCoord(worldPosV, parentWorldInv);
            DirectX::XMStoreFloat3(&position_, localPosV);
        }
        else {
            DetachFromParent();

            parent_ = parent;
            hierarchy_type_ = HierarchyType::Full;

            if (parent_) {
                parent_->children_.push_back(this);
            }
        }

        UpdateTransform();
    }

    /**
     * @brief 親オブジェクトから切り離す
     */
    void DetachFromParent() {
        if (parent_) {
            auto& siblings = parent_->children_;
            siblings.erase(
                std::remove(siblings.begin(), siblings.end(), this),
                siblings.end()
            );
            parent_ = nullptr;
        }
        hierarchy_type_ = HierarchyType::None;
    }

    /**
     * @brief 親オブジェクトを取得
     * @return 親オブジェクトのポインタ（親がいない場合はnullptr）
     */
    GameObject* GetParent() const { return parent_; }

    /**
     * @brief 子オブジェクトのリストを取得
     * @return 子オブジェクトのベクター
     */
    const std::vector<GameObject*>& GetChildren() const { return children_; }

    /**
     * @brief 階層内でのアクティブ状態を取得
     * @return 階層全体でアクティブな場合true
     *
     * 完全な親子関係の場合、親が非アクティブなら子も非アクティブとみなされます。
     */
    bool IsActiveInHierarchy() const {
        if (!active_) return false;

        if (hierarchy_type_ == HierarchyType::Full && parent_) {
            return parent_->IsActiveInHierarchy();
        }

        return true;
    }

    /**
     * @brief ワールド座標を取得
     * @return ワールド座標（親の影響を含む）
     */
    DirectX::XMFLOAT3 GetWorldPosition() const {
        DirectX::XMFLOAT3 worldPos;
        DirectX::XMMATRIX world = GetWorldTransformM();
        DirectX::XMStoreFloat3(&worldPos, world.r[3]);
        return worldPos;
    }

    /**
     * @brief ワールド変換行列を取得
     * @return ワールド変換行列（親の影響を含む）
     */
    DirectX::XMFLOAT4X4 GetWorldTransform() const {
        DirectX::XMFLOAT4X4 worldTransform;
        DirectX::XMStoreFloat4x4(&worldTransform, GetWorldTransformM());
        return worldTransform;
    }

    /**
     * @brief ワールド変換行列を取得（XMMATRIX形式）
     * @return ワールド変換行列（親の影響を含む）
     */
    DirectX::XMMATRIX GetWorldTransformM() const {
        DirectX::XMMATRIX localTransform = DirectX::XMLoadFloat4x4(&transform_);

        if (parent_ && hierarchy_type_ != HierarchyType::None) {
            DirectX::XMMATRIX parentWorld = parent_->GetWorldTransformM();
            return localTransform * parentWorld;
        }

        return localTransform;
    }

    /**
     * @brief 位置を設定（後方互換性のため、デフォルトでローカル座標として扱う） 親がいないならワールド座標
     * @param pos 位置
     */
    void SetPosition(const DirectX::XMFLOAT3& pos) { SetLocalPosition(pos); }

    /**
     * @brief 位置を設定
     * @param x X座標
     * @param y Y座標
     * @param z Z座標
     */
    void SetPosition(float x, float y, float z) { SetLocalPosition(x, y, z); }

    /**
     * @brief 位置を設定（ベクトル形式）
     * @param v 位置ベクトル
     */
    void SetPosition(DirectX::FXMVECTOR v) {
        DirectX::XMFLOAT3 pos;
        DirectX::XMStoreFloat3(&pos, v);
        SetLocalPosition(pos);
    }

    /**
     * @brief ローカル座標を取得
     * @return ローカル座標（親からの相対位置）
     */
    const DirectX::XMFLOAT3& GetLocalPosition() const { return position_; }

    /**
     * @brief ローカル座標を設定
     * @param pos ローカル座標
     */
    void SetLocalPosition(const DirectX::XMFLOAT3& pos) {
        position_ = pos;
        UpdateTransform();
    }

    /**
     * @brief ローカル座標を設定
     * @param x X座標
     * @param y Y座標
     * @param z Z座標
     */
    void SetLocalPosition(float x, float y, float z) {
        position_ = { x, y, z };
        UpdateTransform();
    }

    /**
     * @brief ワールド座標を設定
     * @param worldPos ワールド座標
     *
     * 親がいる場合は自動的にローカル座標に変換されます。
     */
    void SetWorldPosition(const DirectX::XMFLOAT3& worldPos) {
        if (parent_ && hierarchy_type_ != HierarchyType::None) {
            DirectX::XMMATRIX parentWorldInv = DirectX::XMMatrixInverse(nullptr, parent_->GetWorldTransformM());
            DirectX::XMVECTOR worldPosV = DirectX::XMLoadFloat3(&worldPos);
            DirectX::XMVECTOR localPosV = DirectX::XMVector3TransformCoord(worldPosV, parentWorldInv);
            DirectX::XMStoreFloat3(&position_, localPosV);
        }
        else {
            position_ = worldPos;
        }
        UpdateTransform();
    }

    /**
     * @brief ワールド座標を設定
     * @param x X座標
     * @param y Y座標
     * @param z Z座標
     */
    void SetWorldPosition(float x, float y, float z) {
        SetWorldPosition(DirectX::XMFLOAT3(x, y, z));
    }

    /**
     * @brief 位置を取得
     * @return 位置（ローカル座標）
     */
    const DirectX::XMFLOAT3& GetPosition() const { return position_; }

    /**
     * @brief 位置を取得（ベクトル形式）
     * @return 位置ベクトル
     */
    DirectX::XMVECTOR GetPositionV() const { return DirectX::XMLoadFloat3(&position_); }

    /**
     * @brief 回転角を設定
     * @param ang 回転角（ラジアン）
     */
    void SetAngle(const DirectX::XMFLOAT3& ang) { angle_ = ang; UpdateTransform(); }

    /**
     * @brief 回転角を設定
     * @param x X軸回転（ラジアン）
     * @param y Y軸回転（ラジアン）
     * @param z Z軸回転（ラジアン）
     */
    void SetAngle(float x, float y, float z) { angle_ = { x, y, z }; UpdateTransform(); }

    /**
     * @brief 回転角を設定（ベクトル形式）
     * @param v 回転角ベクトル
     */
    void SetAngle(DirectX::FXMVECTOR v) { DirectX::XMStoreFloat3(&angle_, v); UpdateTransform(); }

    /**
     * @brief 回転角を取得
     * @return 回転角（ラジアン）
     */
    const DirectX::XMFLOAT3& GetAngle() const { return angle_; }

    /**
     * @brief 回転角を取得（ベクトル形式）
     * @return 回転角ベクトル
     */
    DirectX::XMVECTOR GetAngleV() const { return DirectX::XMLoadFloat3(&angle_); }

    /**
     * @brief 回転角を設定（度数法）
     * @param deg 回転角（度）
     */
    void SetAngleDegree(const DirectX::XMFLOAT3& deg) {
        angle_.x = DirectX::XMConvertToRadians(deg.x);
        angle_.y = DirectX::XMConvertToRadians(deg.y);
        angle_.z = DirectX::XMConvertToRadians(deg.z);
        UpdateTransform();
    }

    /**
     * @brief 回転角を設定（度数法）
     * @param x X軸回転（度）
     * @param y Y軸回転（度）
     * @param z Z軸回転（度）
     */
    void SetAngleDegree(float x, float y, float z) {
        angle_.x = DirectX::XMConvertToRadians(x);
        angle_.y = DirectX::XMConvertToRadians(y);
        angle_.z = DirectX::XMConvertToRadians(z);
        UpdateTransform();
    }

    /**
     * @brief 回転角を取得（度数法）
     * @return 回転角（度）
     */
    DirectX::XMFLOAT3 GetAngleDegree() const {
        return {
          DirectX::XMConvertToDegrees(angle_.x),
          DirectX::XMConvertToDegrees(angle_.y),
          DirectX::XMConvertToDegrees(angle_.z)
        };
    }

    /**
     * @brief スケールを設定
     * @param scl スケール
     */
    void SetScale(const DirectX::XMFLOAT3& scl) { scale_ = scl; UpdateTransform(); }

    /**
     * @brief スケールを設定
     * @param x X軸スケール
     * @param y Y軸スケール
     * @param z Z軸スケール
     */
    void SetScale(float x, float y, float z) { scale_ = { x, y, z }; UpdateTransform(); }

    /**
     * @brief スケールを設定（均等）
     * @param uniform 均等スケール値
     */
    void SetScale(float uniform) { scale_ = { uniform, uniform, uniform }; UpdateTransform(); }

    /**
     * @brief スケールを設定（ベクトル形式）
     * @param v スケールベクトル
     */
    void SetScale(DirectX::FXMVECTOR v) { DirectX::XMStoreFloat3(&scale_, v); UpdateTransform(); }

    /**
     * @brief スケールを取得
     * @return スケール
     */
    const DirectX::XMFLOAT3& GetScale() const { return scale_; }

    /**
     * @brief スケールを取得（ベクトル形式）
     * @return スケールベクトル
     */
    DirectX::XMVECTOR GetScaleV() const { return DirectX::XMLoadFloat3(&scale_); }

    /**
     * @brief 速度を設定
     * @param vel 速度
     */
    void SetVelocity(const DirectX::XMFLOAT3& vel) { velocity_ = vel; }

    /**
     * @brief 速度を設定
     * @param x X軸速度
     * @param y Y軸速度
     * @param z Z軸速度
     */
    void SetVelocity(float x, float y, float z) { velocity_ = { x, y, z }; }

    /**
     * @brief 速度を設定（ベクトル形式）
     * @param v 速度ベクトル
     */
    void SetVelocity(DirectX::FXMVECTOR v) { DirectX::XMStoreFloat3(&velocity_, v); }

    /**
     * @brief 速度を取得
     * @return 速度
     */
    const DirectX::XMFLOAT3& GetVelocity() const { return velocity_; }

    /**
     * @brief 速度を取得（ベクトル形式）
     * @return 速度ベクトル
     */
    DirectX::XMVECTOR GetVelocityV() const { return DirectX::XMLoadFloat3(&velocity_); }

    /**
     * @brief 速度を加算
     * @param vel 加算する速度
     */
    void AddVelocity(const DirectX::XMFLOAT3& vel) {
        velocity_.x += vel.x;
        velocity_.y += vel.y;
        velocity_.z += vel.z;
    }

    /**
     * @brief 速度を加算
     * @param x X軸速度
     * @param y Y軸速度
     * @param z Z軸速度
     */
    void AddVelocity(float x, float y, float z) {
        velocity_.x += x;
        velocity_.y += y;
        velocity_.z += z;
    }

    /**
     * @brief 速度を加算（ベクトル形式）
     * @param v 速度ベクトル
     */
    void AddVelocity(DirectX::FXMVECTOR v) {
        DirectX::XMVECTOR current = DirectX::XMLoadFloat3(&velocity_);
        DirectX::XMVECTOR result = DirectX::XMVectorAdd(current, v);
        DirectX::XMStoreFloat3(&velocity_, result);
    }

    /**
     * @brief モデルをファイルから設定
     * @param filepath モデルファイルのパス
     */
    void SetModel(const char* filepath) {
        model_ = ResourceManager::Instance().LoadModel(filepath);
        UpdateTransform();
    }

    /**
     * @brief モデルを設定
     * @param model モデルオブジェクトの共有ポインタ
     */
    void SetModel(const std::shared_ptr<Model>& model) {
        model_ = model;
        UpdateTransform();
    }

    /**
     * @brief モデルを取得
     * @return モデルオブジェクトの共有ポインタ
     */
    std::shared_ptr<Model> GetModel() const { return model_; }

    /**
     * @brief アクティブ状態を設定
     * @param active アクティブ状態
     */
    void SetActive(bool active) { active_ = active; }

    /**
     * @brief アクティブ状態を取得
     * @return アクティブな場合true
     */
    bool IsActive() const { return active_; }

    /**
     * @brief オブジェクトを破棄
     *
     * 完全な親子関係の子オブジェクトも再帰的に破棄されます。
     * TransformOnlyの子は親子関係のみ解除されます。
     */
    void Destroy() {
        active_ = false;

        std::vector<GameObject*> childrenCopy = children_;
        for (GameObject* child : childrenCopy) {
            if (child) {
                if (child->hierarchy_type_ == HierarchyType::Full) {
                    child->Destroy();
                }
                else {
                    child->DetachFromParent();
                }
            }
        }
    }

    /**
     * @brief ローカル変換行列を取得
     * @return ローカル変換行列
     */
    const DirectX::XMFLOAT4X4& GetTransform() const { return transform_; }

    /**
     * @brief ローカル変換行列を取得（XMMATRIX形式）
     * @return ローカル変換行列
     */
    DirectX::XMMATRIX GetTransformM() const { return DirectX::XMLoadFloat4x4(&transform_); }

    /**
     * @brief 前方向ベクトルを取得
     * @return 正規化された前方向ベクトル（ワールド空間）
     */
    DirectX::XMVECTOR GetForward() const {
        DirectX::XMMATRIX m = GetWorldTransformM();
        return DirectX::XMVector3Normalize(m.r[2]);
    }

    /**
     * @brief 右方向ベクトルを取得
     * @return 正規化された右方向ベクトル（ワールド空間）
     */
    DirectX::XMVECTOR GetRight() const {
        DirectX::XMMATRIX m = GetWorldTransformM();
        return DirectX::XMVector3Normalize(m.r[0]);
    }

    /**
     * @brief 上方向ベクトルを取得
     * @return 正規化された上方向ベクトル（ワールド空間）
     */
    DirectX::XMVECTOR GetUp() const {
        DirectX::XMMATRIX m = GetWorldTransformM();
        return DirectX::XMVector3Normalize(m.r[1]);
    }

    /**
     * @brief 前方向ベクトルを取得（XMFLOAT3版）
     * @return 正規化された前方向ベクトル（ワールド空間）
     */
    DirectX::XMFLOAT3 GetForwardFloat3() const {
        DirectX::XMMATRIX m = GetWorldTransformM();
        DirectX::XMVECTOR v = DirectX::XMVector3Normalize(m.r[2]);
        DirectX::XMFLOAT3 out;
        DirectX::XMStoreFloat3(&out, v);
        return out;
    }

    /**
     * @brief 右方向ベクトルを取得（XMFLOAT3版）
     * @return 正規化された右方向ベクトル（ワールド空間）
     */
    DirectX::XMFLOAT3 GetRightFloat3() const {
        DirectX::XMMATRIX m = GetWorldTransformM();
        DirectX::XMVECTOR v = DirectX::XMVector3Normalize(m.r[0]);
        DirectX::XMFLOAT3 out;
        DirectX::XMStoreFloat3(&out, v);
        return out;
    }

    /**
     * @brief 上方向ベクトルを取得（XMFLOAT3版）
     * @return 正規化された上方向ベクトル（ワールド空間）
     */
    DirectX::XMFLOAT3 GetUpFloat3() const {
        DirectX::XMMATRIX m = GetWorldTransformM();
        DirectX::XMVECTOR v = DirectX::XMVector3Normalize(m.r[1]);
        DirectX::XMFLOAT3 out;
        DirectX::XMStoreFloat3(&out, v);
        return out;
    }


    /**
     * @brief 位置を移動
     * @param offset 移動量
     */
    void Move(const DirectX::XMFLOAT3& offset) {
        position_.x += offset.x;
        position_.y += offset.y;
        position_.z += offset.z;
        UpdateTransform();
    }

    /**
     * @brief 位置を移動
     * @param x X軸移動量
     * @param y Y軸移動量
     * @param z Z軸移動量
     */
    void Move(float x, float y, float z) {
        position_.x += x;
        position_.y += y;
        position_.z += z;
        UpdateTransform();
    }

    /**
     * @brief 位置を移動（ベクトル形式）
     * @param v 移動量ベクトル
     */
    void Move(DirectX::FXMVECTOR v) {
        DirectX::XMVECTOR current = DirectX::XMLoadFloat3(&position_);
        DirectX::XMVECTOR result = DirectX::XMVectorAdd(current, v);
        DirectX::XMStoreFloat3(&position_, result);
        UpdateTransform();
    }

    /**
     * @brief 前方向に移動
     * @param distance 移動距離
     */
    void MoveForward(float distance) {
        DirectX::XMVECTOR forward = GetForward();
        DirectX::XMVECTOR offset = DirectX::XMVectorScale(forward, distance);
        Move(offset);
    }

    /**
     * @brief 右方向に移動
     * @param distance 移動距離
     */
    void MoveRight(float distance) {
        DirectX::XMVECTOR right = GetRight();
        DirectX::XMVECTOR offset = DirectX::XMVectorScale(right, distance);
        Move(offset);
    }

    /**
     * @brief 上方向に移動
     * @param distance 移動距離
     */
    void MoveUp(float distance) {
        DirectX::XMVECTOR up = GetUp();
        DirectX::XMVECTOR offset = DirectX::XMVectorScale(up, distance);
        Move(offset);
    }

    /**
     * @brief 回転を加算
     * @param delta 回転量（ラジアン）
     */
    void Rotate(const DirectX::XMFLOAT3& delta) {
        angle_.x += delta.x;
        angle_.y += delta.y;
        angle_.z += delta.z;
        UpdateTransform();
    }

    /**
     * @brief 回転を加算
     * @param x X軸回転量（ラジアン）
     * @param y Y軸回転量（ラジアン）
     * @param z Z軸回転量（ラジアン）
     */
    void Rotate(float x, float y, float z) {
        angle_.x += x;
        angle_.y += y;
        angle_.z += z;
        UpdateTransform();
    }

    /**
     * @brief 回転を加算（度数法）
     * @param x X軸回転量（度）
     * @param y Y軸回転量（度）
     * @param z Z軸回転量（度）
     */
    void RotateDegree(float x, float y, float z) {
        angle_.x += DirectX::XMConvertToRadians(x);
        angle_.y += DirectX::XMConvertToRadians(y);
        angle_.z += DirectX::XMConvertToRadians(z);
        UpdateTransform();
    }

    /**
     * @brief 指定座標を向く
     * @param target 向く対象の座標
     */
    void LookAt(const DirectX::XMFLOAT3& target) {
        DirectX::XMVECTOR pos = DirectX::XMLoadFloat3(&position_);
        DirectX::XMVECTOR tar = DirectX::XMLoadFloat3(&target);
        DirectX::XMVECTOR dir = DirectX::XMVectorSubtract(tar, pos);

        float x = DirectX::XMVectorGetX(dir);
        float z = DirectX::XMVectorGetZ(dir);
        angle_.y = atan2f(x, z);
        UpdateTransform();
    }

    /**
     * @brief 指定座標を向く（ベクトル形式）
     * @param target 向く対象の座標ベクトル
     */
    void LookAt(DirectX::FXMVECTOR target) {
        DirectX::XMFLOAT3 t;
        DirectX::XMStoreFloat3(&t, target);
        LookAt(t);
    }

    /**
     * @brief 他のオブジェクトとの距離を取得
     * @param other 対象オブジェクト
     * @return 距離（ワールド座標基準）
     */
    float GetDistanceTo(const GameObject* other) const {
        if (!other) return 0.0f;
        DirectX::XMFLOAT3 worldPos1 = GetWorldPosition();
        DirectX::XMFLOAT3 worldPos2 = other->GetWorldPosition();
        DirectX::XMVECTOR p1 = DirectX::XMLoadFloat3(&worldPos1);
        DirectX::XMVECTOR p2 = DirectX::XMLoadFloat3(&worldPos2);
        DirectX::XMVECTOR diff = DirectX::XMVectorSubtract(p2, p1);
        DirectX::XMVECTOR length = DirectX::XMVector3Length(diff);
        return DirectX::XMVectorGetX(length);
    }

    /**
     * @brief 指定座標との距離を取得
     * @param point 対象座標
     * @return 距離
     */
    float GetDistanceTo(const DirectX::XMFLOAT3& point) const {
        DirectX::XMFLOAT3 worldPos = GetWorldPosition();
        DirectX::XMVECTOR p1 = DirectX::XMLoadFloat3(&worldPos);
        DirectX::XMVECTOR p2 = DirectX::XMLoadFloat3(&point);
        DirectX::XMVECTOR diff = DirectX::XMVectorSubtract(p2, p1);
        DirectX::XMVECTOR length = DirectX::XMVector3Length(diff);
        return DirectX::XMVectorGetX(length);
    }

    /**
     * @brief 他のオブジェクトとの距離の2乗を取得
     * @param other 対象オブジェクト
     * @return 距離の2乗（計算コスト削減用）
     */
    float GetDistanceSquaredTo(const GameObject* other) const {
        if (!other) return 0.0f;
        DirectX::XMFLOAT3 worldPos1 = GetWorldPosition();
        DirectX::XMFLOAT3 worldPos2 = other->GetWorldPosition();
        float dx = worldPos2.x - worldPos1.x;
        float dy = worldPos2.y - worldPos1.y;
        float dz = worldPos2.z - worldPos1.z;
        return dx * dx + dy * dy + dz * dz;
    }

    /**
     * @brief 他のオブジェクトへの方向ベクトルを取得
     * @param other 対象オブジェクト
     * @return 正規化された方向ベクトル
     */
    DirectX::XMVECTOR GetDirectionTo(const GameObject* other) const {
        if (!other) return DirectX::XMVectorZero();
        DirectX::XMFLOAT3 worldPos1 = GetWorldPosition();
        DirectX::XMFLOAT3 worldPos2 = other->GetWorldPosition();
        DirectX::XMVECTOR p1 = DirectX::XMLoadFloat3(&worldPos1);
        DirectX::XMVECTOR p2 = DirectX::XMLoadFloat3(&worldPos2);
        DirectX::XMVECTOR dir = DirectX::XMVectorSubtract(p2, p1);
        return DirectX::XMVector3Normalize(dir);
    }

    /**
     * @brief 指定座標への方向ベクトルを取得
     * @param point 対象座標
     * @return 正規化された方向ベクトル
     */
    DirectX::XMVECTOR GetDirectionTo(const DirectX::XMFLOAT3& point) const {
        DirectX::XMFLOAT3 worldPos = GetWorldPosition();
        DirectX::XMVECTOR p1 = DirectX::XMLoadFloat3(&worldPos);
        DirectX::XMVECTOR p2 = DirectX::XMLoadFloat3(&point);
        DirectX::XMVECTOR dir = DirectX::XMVectorSubtract(p2, p1);
        return DirectX::XMVector3Normalize(dir);
    }

    /**
     * @brief 他のオブジェクトが範囲内にいるか判定
     * @param other 対象オブジェクト
     * @param range 判定範囲
     * @return 範囲内の場合true
     */
    bool IsInRange(const GameObject* other, float range) const {
        if (!other) return false;
        return GetDistanceSquaredTo(other) <= (range * range);
    }

    /**
     * @brief 変換行列を直接設定
     * @param transform 変換行列
     */
    void SetTransform(const DirectX::XMFLOAT4X4& transform) {
        transform_ = transform;
        if (model_) {
            DirectX::XMFLOAT4X4 worldTransform = GetWorldTransform();
            model_->UpdateTransform(worldTransform);
        }
    }

    /**
     * @brief 力積を適用
     * @param impulse 力積
     */
    void ApplyImpulse(const DirectX::XMFLOAT3& impulse) {
        AddVelocity(impulse);
    }

    /**
     * @brief 力積を適用（ベクトル形式）
     * @param impulse 力積ベクトル
     */
    void ApplyImpulse(DirectX::FXMVECTOR impulse) {
        AddVelocity(impulse);
    }

    /**
     * @brief 移動を停止
     *
     * 速度をゼロに設定します。
     */
    void StopMovement() {
        velocity_ = { 0.0f, 0.0f, 0.0f };
    }

    /**
     * @brief 現在の速さを取得
     * @return 速さ（速度ベクトルの大きさ）
     */
    float GetSpeed() const {
        DirectX::XMVECTOR v = DirectX::XMLoadFloat3(&velocity_);
        DirectX::XMVECTOR length = DirectX::XMVector3Length(v);
        return DirectX::XMVectorGetX(length);
    }

    /**
     * @brief 速さを設定
     * @param speed 速さ
     *
     * 速度の方向は維持したまま大きさのみ変更します。
     */
    void SetSpeed(float speed) {
        DirectX::XMVECTOR v = DirectX::XMLoadFloat3(&velocity_);
        DirectX::XMVECTOR normalized = DirectX::XMVector3Normalize(v);
        DirectX::XMVECTOR result = DirectX::XMVectorScale(normalized, speed);
        DirectX::XMStoreFloat3(&velocity_, result);
    }

    /**
     * @brief 経過時間を取得
     * @return オブジェクト生成からの経過時間（秒）
     */
    float GetElapsedTime() const { return elapsed_time_; }

    /**
     * @brief 経過時間をリセット
     */
    void ResetElapsedTime() { elapsed_time_ = 0.0f; }

    /**
     * @brief 経過時間を設定
     * @param time 経過時間（秒）
     */
    void SetElapsedTime(float time) { elapsed_time_ = time; }

protected:
    /**
     * @brief 変換行列を更新
     *
     * 位置、回転、スケールから変換行列を計算し、モデルと子オブジェクトに適用します。
     */
    void UpdateTransform() {
        DirectX::XMMATRIX S = DirectX::XMMatrixScaling(scale_.x, scale_.y, scale_.z);
        DirectX::XMMATRIX R = DirectX::XMMatrixRotationRollPitchYaw(angle_.x, angle_.y, angle_.z);
        DirectX::XMMATRIX T = DirectX::XMMatrixTranslation(position_.x, position_.y, position_.z);
        DirectX::XMMATRIX W = S * R * T;
        DirectX::XMStoreFloat4x4(&transform_, W);

        if (model_) {
            DirectX::XMFLOAT4X4 worldTransform = GetWorldTransform();
            model_->UpdateTransform(worldTransform);
        }

        for (GameObject* child : children_) {
            if (child) {
                child->UpdateTransform();
            }
        }
    }

	Tag tag_;                           ///< オブジェクトのタグ
    DirectX::XMFLOAT3 position_;        ///< ローカル座標
    DirectX::XMFLOAT3 angle_;           ///< 回転角（ラジアン）
    DirectX::XMFLOAT3 scale_;           ///< スケール
    DirectX::XMFLOAT3 velocity_;        ///< 速度
    DirectX::XMFLOAT4X4 transform_;     ///< ローカル変換行列
    float elapsed_time_;                ///< 経過時間
    bool active_;                       ///< アクティブ状態
    std::shared_ptr<Model> model_;      ///< モデルオブジェクト

    GameObject* parent_;                       ///< 親オブジェクト
    std::vector<GameObject*> children_;        ///< 子オブジェクトリスト
    HierarchyType hierarchy_type_;             ///< 親子関係のタイプ
};

#endif