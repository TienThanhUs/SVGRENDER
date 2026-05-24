#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>

using namespace sf;
using namespace std;

// Struct màu gradient
struct GradientStop
{
    float offset;
    Color color;
};

// Struct Gradient hỗ trợ cả Linear, Radial và Transform
struct Gradient
{
    string id;
    string type; // "linear" hoặc "radial"
    vector<GradientStop> stops;

    // Linear
    float x1 = 0, y1 = 0, x2 = 1, y2 = 0;
    // Radial
    float cx = 0.5f, cy = 0.5f, r = 0.5f;
    float fx = 0.5f, fy = 0.5f;

    string units = "objectBoundingBox";
    Transform transform;
};


// struct luu thong tin viewBox trong the <svg>
struct viewBox
{
    float x = 0, y = 0, width = 0, height = 0; // toa do va kich thuoc viewBox
    void setViewBoxAttribute(const string &value); // thiet lap thuoc tinh tu string
};

// struct luu thong tin transform cua tung the shape trong file svg
struct multi_transform
{
    vector<string> types; // loai transform (translate, rotate, scale...)
    vector<float> values; // gia tri transform tuong ung
};

// struct luu cac thong so transform ban dau, dung de xu ly bien doi theo tin hieu tu ban phim
struct realtime_transform
{
    float zoomLevel; // ti le zoom hien tai
    float rotationAngle; // goc xoay hien tai
    realtime_transform(); // constructor mac dinh
};
