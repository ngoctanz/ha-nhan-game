# Kiến trúc gameplay

## Luồng sự kiện

```text
Dialogue đổi dòng
  -> revision tăng
  -> BambooVillageLevel::SyncDialogueActors()
  -> ParseCharacterAction()
  -> Player::TriggerAction()
  -> CharacterActionPlayer giữ đúng một pose tĩnh trong suốt câu
  -> dòng kế tiếp xóa hoặc thay pose

Người chơi tương tác với gà
  -> Chicken::StartEscape()
  -> Chicken tự chạy state machine trong Update()
  -> ChickenEvent::Vanished
  -> màn chơi mở hội thoại kết quả
```

Màn chơi chỉ điều phối sự kiện. Thời gian, frame, chuyển trạng thái và vị trí
của actor nằm trong module actor tương ứng; không sửa trực tiếp state của actor
từ level.

## Thêm hành động nhân vật

1. Thêm tên vào `CharacterAction` và `ParseCharacterAction()`.
2. Nạp mỗi pose độc nhất đúng một lần trong `CharacterActionPlayer::Load()`.
3. Ánh xạ action trực tiếp tới đúng một pose trong `Trigger()`.
4. Ghi tên pose vào `DialogueLine::action`. Để trống nghĩa là idle.

Không tạo một animation chứa bản sao texture cho mỗi đoạn thoại.

## Thêm actor hoặc sự kiện

- Actor nhận lệnh bằng hàm (`StartEscape`, `TriggerAction`, ...).
- `Update()` trả enum event khi hành vi kết thúc.
- Level nhận event và thay đổi quest/hội thoại.
- `Draw()` chỉ vẽ state hiện tại; không tải file, không tạo texture và không đổi quest.

## Quy tắc hiệu năng/mobile

- Asset GPU được tải một lần qua `TextureAsset` và hủy bằng cùng utility.
- Pose thoại chỉ giữ enum/index hiện tại, không cấp phát khi đổi pose.
- Chuỗi chữ HUD nhiệm vụ được cache và chỉ dựng lại khi state thay đổi.
- Không tạo/cắt/resize ảnh trong `Update()` hoặc `Draw()`.
- Game vẫn cần render liên tục khi có chuyển động; tối ưu bằng cách tránh công việc
  nặng và cấp phát mỗi frame, thay vì bỏ frame khiến input/animation giật.
