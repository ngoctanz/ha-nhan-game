# Hà Nhân

**Hà Nhân** là game phiêu lưu 2D mang màu sắc hài hước, đặt trong một thế giới
làng quê giả tưởng. Người chơi theo chân Hà Nhân qua các tình huống đời thường,
hội thoại, nhiệm vụ và những trận đánh được kể bằng nét vẽ tay đơn giản, biểu cảm
cường điệu và âm thanh meme.

Dự án tập trung vào cách kể chuyện theo chapter. Mỗi màn chơi là một đơn vị nội
dung độc lập nhưng dùng chung nhân vật, hội thoại, nhiệm vụ, tiến trình và giao
diện. Game hỗ trợ desktop và Android từ cùng một codebase C++.

## Gameplay

Người chơi điều khiển Hà Nhân di chuyển trong từng khu vực, tương tác trực tiếp
với NPC hoặc vật thể và hoàn thành mục tiêu hiện tại. Nội dung được triển khai qua
các chapter như phòng ngủ, khu vực trước nhà, Thôn Trúc, nhà hàng xóm ban đêm và
cổng làng.

Vòng lặp gameplay chính gồm:

- khám phá màn chơi và tương tác với actor trong thế giới;
- theo dõi nhiệm vụ và tiến trình cốt truyện;
- hội thoại có biểu cảm khuôn mặt, pose nhân vật và hiệu ứng âm thanh;
- thu thập, quản lý vật phẩm và kỹ năng qua menu Hệ Thống;
- thực hiện các hành động như đi, chạy, rón rén, ném và đá bay;
- tham gia các tình huống chiến đấu hoặc sự kiện được điều khiển bằng state machine.

Trên desktop, input đến từ bàn phím. Trên Android, người chơi giữ các nút cho
chuyển động và chạm trực tiếp vào actor hoặc vật phẩm để tương tác. Gameplay chỉ
đọc trạng thái đã chuẩn hóa từ `GameInput`, vì vậy level và actor không phụ thuộc
trực tiếp vào thiết bị nhập liệu.

## Cách game vận hành

`Game` sở hữu vòng lặp chính và các hệ thống dùng chung. `LevelManager` quản lý
level hiện tại, thực hiện chuyển màn và giữ `GameProgress` xuyên suốt các chapter.
Mỗi class kế thừa `Level` tự nạp nội dung, cập nhật actor và vẽ khu vực của mình.

```text
Game
├── LevelManager
│   ├── GameProgress
│   └── Level hiện tại
│       ├── actor và vật thể
│       ├── dialogue và quest
│       └── event của chapter
├── GameInput
├── BackgroundMusic / SoundEffects
└── Ui / SystemMenu
```

### Level và actor

Level đóng vai trò điều phối thay vì sửa trực tiếp trạng thái bên trong nhân vật.
Một actor nhận lệnh public, tự chạy timer và state machine trong `Update()`, sau đó
trả event khi hành vi hoàn tất. Level dùng event đó để cập nhật nhiệm vụ, mở hội
thoại hoặc chuyển sang bước tiếp theo của chapter.

Cách tổ chức này được dùng cho Hà Nhân, Chó Hệ Thống, Trưởng thôn, gà, hàng xóm
và nhóm lưu manh. Logic chuyển động thật có thể dùng animation nhiều frame; pose
diễn xuất trong hội thoại luôn là một ảnh tĩnh được giữ trong suốt câu thoại.

### Hội thoại và biểu cảm

Mỗi `DialogueLine` chứa người nói, nội dung, ID khuôn mặt, pose tùy chọn và sound
effect tùy chọn. Khi dialogue chuyển dòng, revision tăng và level đồng bộ lại actor
đúng một lần. Pose của dòng cũ được xóa hoặc thay bằng pose của dòng mới; game
không tự đoán hành động từ tên người nói.

Khuôn mặt được tách khỏi body và lấy từ `FaceLibrary`. `FaceRenderer` ghép mặt vào
anchor đã khai báo cho từng body, xử lý độc lập hướng tự nhiên của body và face.
Nhờ vậy cùng một body có thể tái sử dụng nhiều biểu cảm mà không cần tạo thêm
texture nhân vật hoàn chỉnh.

### Nhiệm vụ và tiến trình

`Quest` mô tả mục tiêu cục bộ của màn chơi. `GameProgress` giữ dữ liệu cần tồn tại
khi chuyển chapter, bao gồm trạng thái cốt truyện và các tài nguyên người chơi đã
nhận. `ItemCatalog`, `SkillCatalog`, `CombatSystem` và `SystemMenu` cung cấp lớp dữ
liệu và giao diện cho vật phẩm, kỹ năng, lựa chọn Hệ Thống và chiến đấu.

### Asset và render

Game dùng hệ tọa độ logic `1280x720`. Level có thể rộng hơn viewport và camera
cuộn theo người chơi. Tọa độ actor được tính từ điểm bàn chân chạm đất; metadata
của từng sprite xác định vị trí body, khuôn mặt và bóng.

Texture được tải một lần qua `TextureAsset`. Nhạc nền dài được stream bằng
`BackgroundMusic`, còn sound effect ngắn được giải mã khi load và tái sử dụng qua
`SoundEffects`. Trên Android, asset âm thanh được đọc từ gói APK vào bộ nhớ trước
khi tạo stream vì chúng không tồn tại dưới dạng đường dẫn file thông thường.

## Công nghệ

| Thành phần | Công nghệ |
|---|---|
| Ngôn ngữ | C++20 |
| Game framework | raylib 5.5 |
| Build desktop | CMake 3.20+, FetchContent |
| Android | Gradle 8.10.2, Android Gradle Plugin, NativeActivity |
| Định dạng asset | PNG, GIF, MP3, TTF, FLA |
| Công cụ asset | Python |
| Nền tảng | Linux, Windows qua MinGW, Android |

raylib được lấy tự động trong bước configure CMake. Trên desktop, project tạo
executable `funny-game`; trên Android, cùng danh sách source được build thành
shared library `libmain.so` và đóng gói trong APK.

## Cấu trúc repository

```text
.
├── android/                 cấu hình Gradle, manifest và Activity Android
├── assets/
│   ├── characters/         body, animation và metadata nhân vật
│   ├── environment/        background và foreground của level
│   ├── faces/              thư viện biểu cảm tái sử dụng
│   ├── music/              nhạc nền dạng stream
│   ├── sounds/             sound effect ngắn
│   ├── ui/                 nút mobile và icon vật phẩm
│   └── source/             source artwork phục vụ chỉnh sửa
├── include/game/           interface và khai báo hệ thống
├── src/game/               implementation gameplay
├── tools/                  script chuẩn bị asset
├── CMakeLists.txt          target desktop và Android native
└── AGENTS.md               quy ước bắt buộc khi sửa dự án
```

Các module gameplay chính:

- `Game`, `LevelManager`, `Level`: vòng đời game và chuyển chapter;
- `GameProgress`, `Quest`: tiến trình dài hạn và mục tiêu màn chơi;
- `Dialogue`, `CharacterAction`: câu thoại và pose diễn xuất;
- `FaceLibrary`, `FaceRenderer`: nạp và ghép biểu cảm;
- `Player`, `SystemDog` và các actor NPC: state, movement và event;
- `GameInput`: lớp input thống nhất desktop/mobile;
- `Ui`, `SystemMenu`: HUD, dialogue, inventory và kỹ năng;
- `BackgroundMusic`, `SoundEffects`: audio stream và SFX;
- `CombatSystem`: luật và trạng thái chiến đấu.

## Nền tảng và trạng thái

- Desktop dùng CMake và yêu cầu compiler hỗ trợ C++20.
- Windows có toolchain MinGW riêng và link static runtime.
- Android dùng NativeActivity, giao diện cảm ứng và đóng gói tập asset runtime.
- Fullscreen Android chưa ổn định trên một số thiết bị; trạng thái được theo dõi
  tại [issue #2](https://github.com/ngoctanz/ha-nhan-game/issues/2).

Project hiện ở giai đoạn phát triển. Nội dung chapter, cân bằng gameplay và asset
có thể tiếp tục thay đổi.

## Phát triển

Quy tắc về phong cách mỹ thuật, pose hội thoại, hướng nhìn, face anchor, mốc bàn
chân, input mobile và quản lý texture được định nghĩa trong
[`AGENTS.md`](AGENTS.md). Đây là tài liệu chuẩn cần tuân thủ trước khi thay đổi
gameplay, nhân vật hoặc asset.

Lệnh build tham khảo được giữ ngắn gọn:

```bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

Chi tiết riêng cho gói Android nằm trong [`android/README.md`](android/README.md).
