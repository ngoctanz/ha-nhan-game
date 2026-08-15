# Hà Nhân

Asset được tổ chức theo hai trục: **cảm xúc → hành động**.

```text
emotions/
└── angry/
    ├── actions/
    │   ├── idle/
    │   ├── walk/
    │   ├── sit/
    │   └── gesture/
    └── source_frames/
```

Ngoài bộ Animate cũ, Hà Nhân hiện có thêm:

- `actions/sneak/`: sáu frame đi rón rén, giữ `Ctrl` khi di chuyển.
- `actions/goofy_walk/`: 33 frame trích từ `ha-nhan-chibi.gif`, dành cho event về sau;
  người chơi không thể tự kích hoạt từ input hoặc mobile UI.
- `../../faces/boy_faces/`: thư viện 50 biểu cảm nam, mỗi biểu cảm có 6 frame dùng chung cho hội thoại và các model mặt trống.

Các khóa portrait hội thoại có dạng `player:<expression>`, ví dụ
`player:angry`, `player:stunned_disbelief`, `player:uneasy`.
Danh sách đầy đủ nằm trong `assets/faces/boy_faces/male_face_expression_map.md`.

Khuôn mặt không còn được dùng như một portrait rời. Game vẽ thân nhân vật trước,
sau đó `FaceRenderer` đè biểu cảm hiện tại trực tiếp lên vùng đầu của sprite.

- `source_frames/` giữ nguyên toàn bộ 80 PNG đã xuất từ Animate, kể cả frame lặp và các tên đã đổi thủ công.
- `actions/` chỉ chứa asset đã phân loại để game sử dụng.
- Khuôn mặt hiện tại được đánh dấu là emotion `angry`.
- `player.json` là catalog tên animation, FPS và thứ tự frame.

## Kết quả audit

80 PNG nguồn chỉ có 17 hình độc nhất. Phần lớn là frame giữ pose:

- 0001–0005: idle weight shift.
- 0006–0016: hands folded.
- 0017–0029: present low.
- 0030–0044: present open.
- 0045–0059: point.
- 0060–0065: wave.
- 0066–0069: sit transition.
- 0070–0071: sit.
- 0072–0080: walk cycle chín frame.
