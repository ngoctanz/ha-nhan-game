# Animated face library

Thư viện runtime có **79 biểu cảm**, gồm **50 bộ nam** và **29 bộ nữ**. Mỗi biểu
cảm có đúng 6 frame đã chuẩn hóa:

```text
faces/
├── boy_faces/
│   ├── male_face_expression_map.md
│   └── 01_confused/frame_01.png ... frame_06.png
└── girl_faces/
    ├── face_expression_map.md
    └── 1_nervous_smile/frame_01.png ... frame_06.png
```

Phần số ở đầu folder chỉ phục vụ sắp xếp/catalog. Khóa gọi trong game là phần
`snake_case` phía sau, ví dụ `confused`, `old_man`, `nervous_smile` hoặc
`playful_wink`. Hai file `*_expression_map.md` mô tả ý nghĩa diễn xuất của từng
khóa và là nguồn tham chiếu khi viết dialogue.

## API runtime

`FaceLibrary::Load()` nạp mỗi texture đúng một lần. Mỗi actor giữ một
`FaceAnimator`, không giữ đường dẫn và không tự nạp texture:

```cpp
FaceAnimator face;

// Chỉ reset frame khi FaceId thực sự đổi.
face.SetExpression(faces, FaceGender::Male, "angry");
face.Update(deltaTime);
DrawFaceOverlay(face.Current(faces), destination, anchor,
                bodyFlipped, faceFlipped);
```

Đối với nữ, chỉ đổi gender và dùng tên trong catalog nữ:

```cpp
face.SetExpression(faces, FaceGender::Female, "concerned");
```

Dialogue của Hà Nhân dùng khóa `player:<expression>`, ví dụ `player:angry`.
Chỉ tên sạch trong hai catalog mới được hỗ trợ; bộ tên và asset cũ đã được xóa.

Khi nhân vật đang nói, animation mặt chạy 8 fps. Ngoài dialogue mặt giữ tĩnh;
trong dialogue chỉ `FaceAnimator` của đúng người đang nói được `Update()`. Asset
hiện có ba frame giữ cho mỗi key pose, nên nhịp đổi nét mặt vẫn rõ. Toàn bộ
lookup tên diễn ra khi đổi biểu cảm; `Draw()` chỉ lấy frame bằng `FaceId`.
