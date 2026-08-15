# Kiến trúc game

Project được tách theo trách nhiệm để có thể thêm nhiều nhân vật, cảm xúc, nhiệm vụ và màn chơi mà không làm `main.cpp` phình to.

```text
src/main.cpp
  └── Game
      ├── LevelManager
      │   └── Level
      │       └── BambooVillageLevel
      └── Ui

BambooVillageLevel
  ├── Player
  ├── Elder
  ├── Dialogue
  ├── QuestTracker
  └── Animation
```

## Module

- `Game`: game loop chung.
- `LevelManager`: tạo, giữ và chuyển level hiện tại.
- `Level`: interface bắt buộc cho mọi màn chơi.
- `BambooVillageLevel`: nội dung riêng của demo Thôn Trúc.
- `Animation`: tải danh sách frame, crop, resize và chạy animation.
- `Player`: input, movement và animation của Hà Nhân.
- `Elder`: model Trưởng thôn và portrait theo cảm xúc.
- `Dialogue`: tiến trình hội thoại và callback hành động sau câu cuối.
- `QuestTracker`: ID, objective, progress và trạng thái hoàn thành của nhiệm vụ.
- `Ui`: font tiếng Việt, HUD, prompt và hộp thoại.
- `AssetLocator`: tìm asset ổn định khi chạy từ root hoặc thư mục build.

## Thêm màn chơi

1. Tạo class mới kế thừa `Level`.
2. Cài đặt `Load`, `Update`, `Draw`.
3. Thêm ID vào `LevelId`.
4. Thêm một nhánh factory trong `LevelManager::Load`.
5. Thêm file `.cpp` vào `CMakeLists.txt`.

Game loop, UI, dialogue, quest, animation và entity hiện có không cần viết lại.

## Thêm cảm xúc cho Hà Nhân

```text
assets/characters/player/emotions/<emotion>/actions/
├── idle/
├── walk/
├── sit/
└── gesture/
```

Cập nhật `assets/characters/player/player.json` để khai báo FPS và thứ tự frame. Các frame xuất thẳng từ Animate phải nằm trong `source_frames/`, không để game phụ thuộc trực tiếp vào tên export.

## Thêm NPC

Mỗi NPC nên có thư mục riêng:

```text
assets/characters/npc/<npc-id>/
├── expressions/
├── walk/
└── <npc-id>.json
```

Entity NPC chỉ tải asset curated. Sheet nguồn/chroma được giữ để chỉnh sửa nhưng không tải vào runtime.
