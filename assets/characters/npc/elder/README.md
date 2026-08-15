# Trưởng thôn

Model thử nghiệm dựa trên nhân vật thứ hai từ phải sang trong ảnh tham chiếu.

## Nhận diện cố định

- Khăn đỏ che kín mắt.
- Hoa vàng bên phải khăn.
- Tóc búi cao và râu nhọn.
- Áo ngoài nâu, áo trong màu be, đai tối màu.
- Gậy gỗ cong.

## Asset

- `elder-expression-sheet.png`: sheet toàn thân và tám biểu cảm, nền alpha.
- `elder-walk-sheet.png`: sheet nguồn tám pha bước đi, nền alpha.
- `walk/`: tám frame đã tách, cùng canvas 300×460 và baseline `y = 432`.
- `expressions/`: model toàn thân trung tính và tám portrait biểu cảm.
- `elder-walk-preview.gif`: bản xem nhanh chu kỳ bước đi ở khoảng 9 FPS.
- `elder.json`: manifest để game nạp animation và expression.

Các file `*-chroma.png` là bản nguồn nền xanh, được giữ lại để chỉnh matte nếu cần.
