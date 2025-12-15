#ifndef UI_EVENT_H_
#define UI_EVENT_H_

class UiButton;

// UIイベントの種類
enum class UiEventType {
    Click,
    PressBegin,
    PressEnd,
    HoverBegin,
    HoverEnd
};

// UIイベント
struct UiEvent {
    UiEventType type;
    UiButton* button;
    int action_id;

    UiEvent(UiEventType t, UiButton* btn, int id)
        : type(t), button(btn), action_id(id) {
    }
};

// メニュー背景モード
namespace MenuBackgroundMode {
    constexpr int kBackgroundVisible = 0;
    constexpr int kBackgroundHidden = 1;
}

#endif // UI_EVENT_H_