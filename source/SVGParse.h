#pragma once
#include <vector>
#include <string>
#include <map>
#include "rapidxml.hpp"
#include "Shape.h"
#include "Types.h"

// Lớp SVGParse lưu trữ dữ liệu phân tích từ file SVG
class SVGParse
{
public:
    string filename;                        // Tên file SVG
    pair<unsigned int, unsigned int> frame; // Kích thước cửa sổ render
    viewBox vb;                             // Thông tin viewBox SVG
    vector<shape *> shapes;                 // Danh sách các shape đã parse

    map<string, string> globalStyles; // Style toàn cục từ CSS

    map<string, Gradient> gradients; // Danh sách gradient được định nghĩa

    SVGParse(const string &); // Khởi tạo parser với tên file SVG
    ~SVGParse();              // Giải phóng tài nguyên

    void read_style(rapidxml::xml_node<> *root); // Đọc thẻ <style> trong SVG
    void read_defs(rapidxml::xml_node<> *root);  // Đọc thẻ <defs> (gradient, ...)
    void loadSVG();                              // Load và parse toàn bộ file SVG
    void clear();                                // Xóa dữ liệu đã parse

    void parseCssContent(string content); // Parse nội dung CSS trong SVG
};

// Cấu trúc đại diện cho thẻ <g> (group) trong SVG
struct group
{
    map<string, string> attributes; // Thuộc tính kế thừa của group

    // Duyệt cây XML group và tạo các shape con
    void traversal_group(rapidxml::xml_node<> *root,
                         vector<shape *> &shapes,
                         const map<string, string> &styles);
};
