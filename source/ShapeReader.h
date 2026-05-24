#pragma once
#include <vector>
#include "Shape.h"
#include "rapidxml.hpp"

using namespace rapidxml;

void read_line(const string &name, const string &value, line *line);           // doc line
void read_rectangle(const string &name, const string &value, rectangle *rect); // doc hinh chu nhat
void read_ellipse(const string &name, const string &value, ellipse *elli);     // doc ellips
void read_circle(const string &name, const string &value, circle *cir);        // doc hinh tron
void read_polygon(const string &name, const string &value, polygon *poly);     // doc polygon
void read_polyline(const string &name, const string &value, polyline *poly);   // doc polyline
void read_text(const string &name, const string &value, text *tex);            // doc text
void read_path(const string &name, const string &value, path *pth);            // doc path