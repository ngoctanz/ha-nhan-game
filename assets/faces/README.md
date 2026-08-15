# Exported face library

Thư viện hiện gồm **49 biểu cảm độc nhất**. Bản export ban đầu chứa 100 PNG vì mỗi
biểu cảm có hai file giống nhau từng pixel (`0001` và `0002`).

- `expressions/`: bộ dùng cho game, tên `snake_case` theo nội dung biểu cảm.
- `source_originals/`: toàn bộ 100 file export gốc, không chỉnh sửa và không xóa.
- `_catalog/`: contact sheet để duyệt hình.
- `catalog.csv`: ánh xạ `source_id`, tên chuẩn và nhóm cảm xúc.
- `priority_faces.csv`: nghĩa diễn xuất do tác giả quy định cho các mặt quan trọng; ưu tiên file này khi viết thoại và nhiệm vụ.

Tên file trong `expressions/` khớp cột `name` trong `catalog.csv`.

### Quy tắc tên đã xác nhận

Các mặt đã được tác giả xác nhận nghĩa dùng mẫu
`verified_<ý_nghĩa_chính>_<dấu_hiệu_hình_ảnh>`. Tên cũ được giữ ở cột
`legacy_name` trong `priority_faces.csv`, nên có thể truy ngược mà không nhầm
với asset chưa được duyệt.

## Cách game sử dụng

Nhân vật được render theo hai lớp: **body → face overlay**. `FaceRenderer` phủ
vùng mặt cũ bên trong đầu rồi đặt biểu cảm từ thư viện này lên trên. Mỗi nhóm
animation có một face anchor riêng, vì vậy biểu cảm vẫn bám đúng đầu khi đứng,
đi thường, rón rén hoặc đi cà khịa.
