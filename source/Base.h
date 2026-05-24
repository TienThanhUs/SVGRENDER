#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include <vector>
#include <map>
#include <cmath>
#include "Types.h"
#include "SVGParse.h"

using namespace std;
using namespace sf;
using namespace rapidxml;

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

string trim(string str);                                     // xoa toan bo khoang trang khong co y nghia
bool check(char a);                                          // kiem tra ky tu co hop le khong
void remove_space(string &s);                                // xoa cac khoang trang de chuan hoa chuoi transform
float clarifyFloat(string s);                                // chuan hoa so
Color read_RGB(string value);                                // doc mau trong cac shape
vector<Vector2f> read_points(string value);                  // luu cac diem vao vector<vector2f> (su dung de doc polygon polyline)
void read_transform(string value, multi_transform &tr);      // doc transform trong svg
Transform createSFMLTransform(const multi_transform &trans); // Nhan vao mot mutil_transform va tra ve mot doi tuong sf::transform
vector<char> getFile(const string &);                        // doc file va tra ve vector<char> chua toan bo noi dung file
void read_file(SVGParse &);                                  // Doc va phan tich du lieu trong file sau do luu vao cau truc du lieu SVGParse
string normalize_path_data(const string &d);                 // Chuẩn hóa chuỗi SVG path data để dễ parse
Transform parseTransformString(string s);                    // Parse chuỗi transform SVG và trả về sf::Transform

// Chuyen doi tu do (degrees) sang radian (float)
inline float deg2rad(float deg)
{
    return deg * static_cast<float>(M_PI) / 180.f;
}