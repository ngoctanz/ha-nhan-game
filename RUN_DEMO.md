# Demo Thôn Trúc

Demo cốt truyện đầu tiên của **Hà Nhân Xuyên Không**.

## Nội dung

- Màn chơi đi ngang tại Thôn Trúc.
- Nhân vật dùng 80 frame PNG xuất từ Adobe Animate.
- Camera bám theo người chơi.
- Hội thoại mở màn và NPC Trưởng thôn.
- Chuỗi nhiệm vụ: nhận nhiệm vụ, đuổi Gà Linh Khí, nhặt vật phẩm và trả nhiệm vụ.
- Giao diện mục tiêu, thông báo hệ thống và đánh dấu tương tác.

## Điều khiển

- `A/D` hoặc `←/→`: di chuyển.
- `Shift`: chạy nhanh.
- `E` hoặc `Space`: tương tác/tiếp tục hội thoại.
- `Esc`: thoát game.

## Cấu trúc mở rộng

Xem `ARCHITECTURE.md` để thêm level, NPC, emotion, animation và quest mới.

## Build

Project dùng C++20, CMake và raylib 5.5.

```powershell
cmake -S . -B build-win -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build-win
./build-win/funny-game.exe
```

Chạy executable với thư mục hiện tại là thư mục gốc của project để game tìm thấy `assets/`.
