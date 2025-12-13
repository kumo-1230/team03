
//クラス名はPascalCase
class ExampleClass {
    public:
        static constexpr int kMaxHealth = 100;
        // 定数はconstexpr
        // 定数名はk + PascalCase

        Example(int health = kMaxHealth) : health_(health) {}
        ~Example() = default;
        void SetHealth(int health) { health_ = health; }// 関数名はPascalCase
        const int GetHealth() const { return health_; }

    private:
        int health_;// private/protected メンバは末尾に_をつける それ以外の変数はつけない
};

// enumも定数名だからk + PascalCase
enum class ListExample {
    kDefault,
    kPlayer,
    kEnemy,
};

int main(){
    ExampleClass exam;
    int player_health = 20;// 変数名はsnake_case
    exam.SetHealth(player_health);
}


// const参照で返す（コピー回避）
const std::string& name() const { return name_; }

// 関数名のDoFunc() 
// つまり動詞→名詞は遵守
Window() // でウィンドウが開くかどうかはわかりづらい
OpenWindow() // これで開くという意味が伝わる

// Get/Set以外の説明がつくなら、Get/Setを使わない
// 例 ウィンドウが有効かどうかを取得する関数
GetWindowMode() // X
IsWindowEnable() // ◯
//return boolはIsを使う
//ただしCollision判定がメインで、Collisionの結果がboolで返る場合などは Isはいらない

//型が明確な場合はautoを使っていい
MyClassManager& mng = MyClassManager::GetInstance(); // 冗長
auto& mng = MyClassManager::GetInstance();