#include <iostream>
#include <string>
#include "SVGParse.h"
#include "SVGRender.h"

using namespace std;
// Chuong trinh su dung SFML 3.0.2

// *************** Keyboard Controls Menu *************
// R: Reset view (zoom = 1.0, rotation = 0)
// = (Equal): Zoom IN (scale view up by 1.5x)
// - (Hyphen): Zoom OUT (scale view down by 1.5x)
// ->: Rotate view right by 15 degrees
// <-   : Rotate view left by 15 degrees
// ****************************************************

int main()
{
    string fileName;
    cout << "Nhap ten file can render (svg-xx.svg): ";
    cin >> fileName;
    SVGParse parse(fileName);
    SVGRender render(parse);
    return 0;
}