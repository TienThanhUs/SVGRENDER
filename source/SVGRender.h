#pragma once
#include "SVGParse.h"

// class nhan du lieu da phan tich tu file svg va thuc hien render ra cua so
class SVGRender
{
public:
    SVGRender(SVGParse &);   // ham khoi tao nhan tham so la SVGParse, khi khoi tao ham se thuc hien render
    void render(SVGParse &); // ve ra cua so
};
