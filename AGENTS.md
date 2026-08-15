# Quy tắc bắt buộc của dự án Hà Nhân

Phạm vi: toàn bộ repository. Mọi agent sửa gameplay, nhân vật hoặc asset phải
đọc và tuân thủ file này trước khi làm việc.

## 1. Phong cách mỹ thuật

- Nhân vật dùng nét vẽ tay tếu, hơi vụng, màu phẳng và ít đổ bóng.
- Không tự nâng độ chi tiết, không biến asset thành tranh game bóng bẩy.
- Con chó Hệ Thống là chuẩn tham chiếu về độ hài và mức độ đơn giản.
- Body nên để trống khuôn mặt để tái sử dụng `FaceLibrary`.

## 2. Một câu thoại chỉ dùng tối đa một pose tĩnh

- `DialogueLine::action` để trống: nhân vật giữ idle.
- Nếu có action, giá trị phải là đúng tên pose:
  - `hands_folded`
  - `point`
  - `present_low`
  - `present_open`
  - `wave`
- Pose được giữ nguyên trong toàn bộ câu thoại và được xóa khi dialogue chuyển
  dòng. Không phát chuỗi pose, không lặp, không tự đoán pose từ tên người nói.
- Alias `rant` và `explain` chỉ tồn tại để tương thích dữ liệu cũ; nội dung mới
  không được dùng chúng.

Ví dụ khai báo:

```cpp
{"HÀ NHÂN", "Ngươi giải thích đi!", "player:verified_angry_surprise", "point"}
{"HÀ NHÂN", "...Ta không còn gì để nói.", "player:verified_dazed_blank_stare", ""}
```

Luồng gọi chuẩn:

```cpp
// Chỉ gọi khi Dialogue::Revision() thay đổi.
player.TriggerAction(ParseCharacterAction(dialogue.Current().action));
// Khi chuyển sang dòng không có action, ParseCharacterAction trả None và pose bị xóa.
```

Khi thêm pose mới: thêm enum + parser + nạp texture độc nhất một lần trong
`CharacterActionPlayer`. Không tạo animation riêng chứa bản sao cùng texture.

## 2A. Quy tắc background màn chơi

- Background làng phải theo cùng phong cách vẽ tay đơn giản, màu phẳng và hơi
  vụng; không dùng tranh concept/anime quá bóng bẩy.
- Màn Thôn Trúc dùng ảnh đúng `2400x720`, trùng `WorldWidth` và `ScreenHeight`;
  không kéo một ảnh sai tỉ lệ trong runtime.
- Đường đi phải liên tục và ít vật cản quanh `GroundY = 622`.
- Nhà cửa phải phủ đều trái–giữa–phải để camera cuộn tới đâu vẫn đọc là làng.
- Núi, trời và tre chỉ là lớp phụ; không được chiếm chỗ của nhà hoặc đường chơi.
- Không đặt người, NPC hay chữ trực tiếp vào background.
- Nếu cần đưa `GroundY` vào giữa đường, được dịch background lên một offset Y
  hằng và nối dải hở bên dưới bằng một dải đất gradient có nét đường
  cố định cùng bảng màu; không kéo giãn sai tỉ lệ PNG.

## 3. Quy tắc hướng nhìn dùng chung

- Mọi world character có hướng phải chứa `CharacterFacing`.
- NPC nhìn người đang nói chuyện bằng:

```cpp
npc.FaceToward(npcPosition.x, speakerPosition.x);
```

- NPC đứng yên không được gọi `FaceToward()` trong `Update()` mỗi frame. Chỉ
  gọi khi người chơi bấm tương tác thành công, ngay trước khi mở dialogue; NPC
  giữ nguyên hướng đó tới lần tương tác kế tiếp.

- Khai báo riêng hướng tự nhiên của body và face. Hai asset có thể khác nhau
  (ví dụ body Trưởng thôn gốc nhìn trái, face gốc nhìn phải).
- Vẽ body bằng `SourceForFacing(texture, facing, bodyArtworkFacesRight)`.
- Tách hai cờ lật khi ghép mặt:

```cpp
const bool bodyFlipped = facing.IsFlipped(bodyArtworkFacesRight);
const bool faceFlipped = facing.IsFlipped(faceArtworkFacesRight);
DrawFaceOverlay(face, bodyDestination, anchor, bodyFlipped, faceFlipped);
```

- Không tự viết lại phép `source.width = -source.width` trong từng NPC.
- Không dùng một cờ `flipped` chung nếu hướng tự nhiên của body và face khác nhau.
  `bodyFlipped` điều khiển canvas/anchor; `faceFlipped` chỉ điều khiển nét mặt.
- Đổi hướng NPC chỉ lật hình, không đổi slot đội hình hay dịch actor qua người khác.
- Chó Hệ Thống đứng phía sau hướng di chuyển của Hà Nhân. Chỉ đổi bên và
  đổi hướng sau khi người chơi thực sự đi đủ ngưỡng quãng đường; bấm/tap
  đảo hướng ngắn không được làm chó nhảy qua người.
- Nếu ảnh gốc nhìn trái, truyền `artworkFacesRight = false` cho utility.
- Dùng dead-zone trong `FaceToward` để NPC không rung trái/phải khi hai tâm gần nhau.

## 4. Ghép mặt vào body

- `FaceAnchor` được khai báo theo hướng tự nhiên của artwork gốc.
- Tâm X phải nằm giữa vùng đầu thật, không nằm giữa toàn canvas nếu canvas có
  gậy, tóc hoặc phụ kiện lệch bên.
- Không sửa `centerX` để giả lập hướng nhìn. `centerX` là tâm giải phẫu của đầu;
  độ tiến về phía trước phải khai báo riêng bằng `forwardOffsetX`.
- `forwardOffsetX` là số có dấu theo canvas artwork chưa lật. Ví dụ artwork gốc
  nhìn phải dùng giá trị dương; khi nhân vật quay trái, `DrawFaceOverlay` tự
  mirror cả tâm và offset sang trái.
- Luôn kiểm tra đủ hai hướng trái/phải. Không chấp nhận mặt đúng một hướng nhưng
  lệch ra sau khi lật.
- Body đã có mặt cũ: dùng `maskOriginal = true`.
- Animation trích từ GIF đã có khuôn mặt hoàn chỉnh (ví dụ `GoofyWalk`) phải giữ
  nguyên mặt trong frame và bỏ qua `DrawFaceOverlay`; không chồng thêm mặt thứ hai.
- Body có vùng mặt trống sạch: dùng `maskOriginal = false` để tránh mảng vá màu.
- Anchor ngoài thế giới và anchor portrait UI có thể khác kích thước, nhưng phải
  cùng tâm giải phẫu và cùng quy ước hướng.
- Không sửa trực tiếp file face để bù lỗi anchor; sửa metadata/anchor của body.

## 4A. Mốc bàn chân và tiếp đất

- Tọa độ actor là điểm bàn chân chạm đất, không phải đáy canvas PNG.
- Mỗi bộ sprite phải khai báo `normalizedFootY` đo một lần từ asset và dùng
  `GroundedDestination`; không bù padding trong suốt bằng số dịch Y ngẫu nhiên.
- Bóng được vẽ trước body, tâm bóng chỉ thấp hơn mốc chân khoảng 2 px để
  giày đè lên bóng; không để một dải nền sáng giữa chân và bóng.
- Không quét alpha hay tạo texture trong `Update()`/`Draw()`; mốc chân là metadata hằng.

## 5. Actor, action và event

- Level chỉ gửi lệnh public như `StartEscape`, `TriggerAction`, `FaceToward`.
- Actor tự quản lý timer, state, vị trí và pose trong `Update()`.
- Khi hành vi hoàn tất, actor trả enum event; level nhận event để đổi quest hoặc
  mở dialogue. Không cho level sửa field nội bộ của actor.
- Animation vận động thật (đi/chạy/rón rén) có thể nhiều frame. Pose diễn xuất
  trong một câu thoại không phải animation và phải tuân thủ mục 2.

## 6. Hiệu năng và mobile

- Tải asset qua `TextureAsset`; không load/crop/resize trong `Update()` hoặc `Draw()`.
- Mỗi pose độc nhất chỉ có một texture GPU. Chuỗi/state chỉ giữ index hoặc enum.
- Không cấp phát vector/string mỗi frame. Cache HUD khi quest thay đổi.
- Không tạo texture tạm khi ghép mặt; body và face được vẽ từ texture đã nạp.
- Game vẫn render theo frame để input và chuyển động mượt. Tối ưu công việc trong
  frame, không bỏ render tùy tiện làm animation giật.
- Gameplay chỉ đọc `GameInput`; actor, dialogue và level không tự đọc phím hay touch
  trực tiếp. Việc đổi cách điều khiển phải sửa tập trung trong `InputSystem`.
- Nút mobile là trạng thái giữ (`Held`) cho đi/chạy/rón rén/tấu hài; tương tác và
  chuyển câu thoại là cạnh nhấn (`Pressed`) để giữ ngón không kích hoạt lặp.
- Touch nằm trên nút điều khiển phải được đánh dấu consumed trước khi kiểm tra actor.
  Touch actor phải đổi từ screen-space sang world-space qua camera hiện tại.
- Giữ hệ tọa độ logic `1280x720`; raylib chịu trách nhiệm scale touch về hệ này.
- Nút `NHẸ` chỉ phát trạng thái `sneakHeld`. Sự kiện cần đi nhẹ về sau kiểm tra trạng
  thái/chuyển động này, không tạo thêm phím hoặc nhánh điều khiển riêng trong level.
- Mobile không có nút Nói/Tương tác: chạm trực tiếp actor hoặc vật phẩm để kích hoạt.
  Không thêm lại nút tương tác toàn cục nếu actor đã có hitbox touch.
- APK chỉ đóng gói asset thật sự dùng ở runtime; không đưa contact sheet, source GIF,
  file export thừa hoặc toàn bộ thư mục làm việc vào gói cài đặt.
- Nhạc dài dùng `BackgroundMusic` và `UpdateMusicStream()` mỗi frame; trên Android
  phải nạp bằng `LoadFileData` + `LoadMusicStreamFromMemory` vì asset trong APK không
  phải đường dẫn file hệ thống. Giữ buffer nén tới sau khi unload stream.
- Meme SFX ngắn dùng `SoundEffects`, giải mã một lần lúc load và tái sử dụng `Sound`.
  `DialogueLine::sound` phải khai báo rõ (`huh`, `doit`, `cao_ni_ma`) và chỉ phát khi
  `Dialogue::Revision()` đổi; không đoán từ tên mặt và không gọi `PlaySound` mỗi frame.

## 7. Checklist trước khi bàn giao

1. Build Release thành công.
2. Chạy game ít nhất 5 giây không thoát sớm.
3. Kiểm tra action lúc bắt đầu câu và sau khi đổi sang câu kế tiếp.
4. Đứng bên trái rồi bên phải NPC; body và mặt phải cùng quay về người chơi.
5. Mặt không nhỏ, không bị cắt, không lệch ra phía sau và không nghiêng ngược.
6. Actor chuyển đủ state trước khi phát event kết thúc.
7. Không có texture cùng đường dẫn bị nạp lặp cho các pose thoại.
