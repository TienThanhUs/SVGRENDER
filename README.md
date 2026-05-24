# SVGRENDER

Một chương trình mạnh mẽ được viết bằng C++ cho phép đọc (parse) và kết xuất (render) hiển thị các file đồ họa vector dạng SVG (Scalable Vector Graphics).

## 1. Code làm gì?
Chương trình này hoạt động như một SVG Viewer nội bộ. Nó phân tích cú pháp (parse) nội dung của một file `.svg` (sử dụng thư viện `rapidxml`) và sau đó hiển thị (render) các khối hình học, đường nét và màu sắc có trong file SVG đó ra một cửa sổ ứng dụng trên máy tính thông qua thư viện đồ họa **SFML**. Ngoài ra, chương trình cho phép người dùng tương tác trực tiếp bằng bàn phím để phóng to, thu nhỏ và xoay bản vẽ một cách linh hoạt.

## 2. Input và Output là gì?
- **Input:** Đường dẫn tới một file định dạng vector `.svg` (Ví dụ: `svg-01.svg` hoặc ổ đĩa chứa file gốc).
- **Output:** Một cửa sổ đồ họa hiển thị kết quả trực quan của file SVG cấu thành từ các hình khối (shapes), đường đi (paths), chữ (texts)... và màu sắc tương ứng nằm trong file cung cấp, cùng với khả năng chịu tương tác.

## 3. Cách sử dụng

### 3.1. Yêu cầu hệ thống (Prerequisites)
- Trình biên dịch C++ hỗ trợ **C++20** (như `g++`).
- Đã cài đặt thư viện đồ họa **SFML 3.0.2** (Trong file Makefile hiện đang được cấu hình đường dẫn cho macOS qua Homebrew).
- Lệnh biên dịch `make`.

### 3.2. Biên dịch (Build)
Di chuyển vào thư mục `source/` của dự án và chạy Makefile:
```bash
cd source
make
```
Sau khi biên dịch thành công, sẽ sinh ra một file thực thi mang tên `svg_reader`.

### 3.3. Chạy chương trình
Để chạy chương trình, bạn gõ lệnh:
```bash
make run
# hoặc chạy trực tiếp bằng lệnh: ./svg_reader
```
Khi này, Terminal sẽ yêu cầu bạn nhập tên hoặc đường dẫn tới file SVG:
```text
Nhap ten file can render (svg-xx.svg): <Nhập đường dẫn file SVG của bạn và ấn Enter>
```

### 3.4. Dùng bàn phím để điều khiển (Keyboard Controls)
Khi cửa sổ SFML hiện ra, bạn có thể điều hướng góc nhìn bằng các phím sau:
- `R`: Trở về trạng thái ban đầu (Reset view với zoom = 1.0, không xoay).
- `=` (Phím Bằng): Phóng to lên 1.5 lần (Zoom IN).
- `-` (Phím Trừ): Thu nhỏ đi 1.5 lần (Zoom OUT).
- `->` (Mũi tên Phải): Xoay góc nhìn sang phải 15 độ.
- `<-` (Mũi tên Trái): Xoay góc nhìn sang trái 15 độ.

---
*Dự án sử dụng `rapidxml` để parse XML, `earcut` để xử lý mảng lưới đa giác và `SFML` để dựng hình 2D.*
