#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include <map>
#include "Types.h"
#include "earcut.hpp"

using namespace std;
using namespace sf;

// Lớp cơ sở trừu tượng cho mọi hình SVG
class shape
{
public:
    virtual ~shape() = default;

    // Vẽ shape lên cửa sổ SFML, có hỗ trợ gradient
    virtual void draw(RenderWindow &window, const map<string, Gradient> &gradients) = 0;

    Color fill_color = Color::Black;   // Màu tô fill
    string fill_id;                    // ID gradient dùng cho fill
    Color stroke_color = Color::Black; // Màu viền stroke

    bool has_fill = true; // Có vẽ fill hay không

    float fill_opacity = 1.0f;   // Độ trong suốt fill
    float stroke_opacity = 1.0f; // Độ trong suốt stroke
    float stroke_width = 0.0f;   // Độ dày stroke
    multi_transform trans;       // Transform SVG áp dụng cho shape
    string typeOfShape;          // Loại shape (line, rect, path, ...)
};

// Shape SVG <line>
class line : public shape
{
public:
    Vector2f start, end; // Điểm bắt đầu và kết thúc
    void draw(RenderWindow &window, const map<string, Gradient> &gradients) override;
};

// Shape SVG <rect>
class rectangle : public shape
{
public:
    Vector2f start;              // Góc trên-trái
    float width = 0, height = 0; // Chiều rộng và chiều cao
    void draw(RenderWindow &window, const map<string, Gradient> &gradients) override;
};

// Shape SVG <circle>
class circle : public shape
{
public:
    Vector2f center; // Tâm hình tròn
    float r = 0;     // Bán kính
    void draw(RenderWindow &window, const map<string, Gradient> &gradients) override;
};

// Shape SVG <ellipse>
class ellipse : public shape
{
public:
    Vector2f start;       // Tâm ellipse
    float rx = 0, ry = 0; // Bán trục x và y
    void draw(RenderWindow &window, const map<string, Gradient> &gradients) override;
};

// Shape SVG <polygon>
class polygon : public shape
{
public:
    vector<Vector2f> p; // Danh sách các đỉnh
    void draw(RenderWindow &window, const map<string, Gradient> &gradients) override;
};

// Shape SVG <polyline>
class polyline : public shape
{
public:
    vector<Vector2f> p;  // Danh sách các điểm
    bool closed = false; // Có khép kín hay không
    void draw(RenderWindow &window, const map<string, Gradient> &gradients) override;
};

// Shape SVG <text>
class text : public shape
{
public:
    Vector2f start;       // Vị trí bắt đầu text
    float dx = 0, dy = 0; // Dịch chuyển text
    float font_size = 12; // Cỡ chữ
    string font_family;   // Font chữ
    string font_style;    // Kiểu chữ (normal, italic)
    string font_weight;   // Độ đậm chữ
    string text_anchor;   // Canh lề text
    string text_;         // Nội dung text

    void draw(RenderWindow &window, const map<string, Gradient> &gradients) override;
};

// Shape SVG <path>
class path : public shape
{
public:
    string d;                // Chuỗi path data SVG
    multi_transform trans;   // Transform riêng của path
    bool has_stroke = false; // Có vẽ stroke hay không

    // Cấu trúc lưu lệnh path (M, L, C, ...)
    struct PathCommand
    {
        char type;            // Loại lệnh path
        vector<float> values; // Tham số của lệnh
    };

    // Parse chuỗi d thành danh sách lệnh path
    vector<PathCommand> parsePathData() const;

    // Render path từ các lệnh đã parse
    void renderPath(RenderWindow &window, const vector<PathCommand> &commands,
                    const map<string, Gradient> &gradients) const;

    // Lấy màu fill có áp dụng opacity
    Color getFillColor() const
    {
        return Color(fill_color.r, fill_color.g, fill_color.b,
                     static_cast<uint8_t>(fill_opacity * 255));
    }

    // Lấy màu stroke có áp dụng opacity
    Color getStrokeColor() const
    {
        return Color(stroke_color.r, stroke_color.g, stroke_color.b,
                     static_cast<uint8_t>(stroke_opacity * 255));
    }

    // Vẽ path SVG
    void draw(RenderWindow &window, const map<string, Gradient> &gradients) override;
};
