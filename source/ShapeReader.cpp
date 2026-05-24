#include <sstream>
#include <iostream>
#include "ShapeReader.h"
#include "Base.h"

using namespace std;

void parse_fill_attribute(const string &value, shape *shp, float &opacity, Color &color)
{
    if (value.find("url(") != string::npos)
    {
        size_t start = value.find("#") + 1;
        size_t end = value.find(")");
        if (start != string::npos && end != string::npos)
        {
            shp->fill_id = value.substr(start, end - start);
            opacity = 1.0f;
        }
    }
    else if (value == "none" || value == "transparent")
    {
        opacity = 0;
    }
    else
    {
        color = read_RGB(value);
    }
}

void read_line(const string &name, const string &value, line *line)
{
    if (name == "stroke-opacity")
    {
        line->stroke_opacity = stof(value);
    }
    else if (name == "stroke")
    {
        if (value == "none" || value == "transparent")
        {
            line->stroke_opacity = 0;
        }
        else
        {
            line->stroke_color = read_RGB(value);
            if (line->stroke_width == 0)
                line->stroke_width = 1;
        }
    }
    else if (name == "x1")
        line->start.x = stof(value);
    else if (name == "y1")
        line->start.y = stof(value);
    else if (name == "x2")
        line->end.x = stof(value);
    else if (name == "y2")
        line->end.y = stof(value);
    else if (name == "stroke-width")
        line->stroke_width = stof(value);
    else if (name == "transform")
        read_transform(value, line->trans);
    else if (name == "style")
    {
        istringstream iss(trim(value));
        string tmp;
        while (getline(iss, tmp, ';'))
        {
            size_t colonPos = tmp.find(':');
            if (colonPos != string::npos)
            {
                read_line(trim(tmp.substr(0, colonPos)), trim(tmp.substr(colonPos + 1)), line);
            }
        }
    }
}

void read_rectangle(const string &name, const string &value, rectangle *rect)
{
    if (name == "fill-opacity")
        rect->fill_opacity = stof(value);
    else if (name == "stroke-opacity")
        rect->stroke_opacity = stof(value);
    else if (name == "fill")
    {
        parse_fill_attribute(value, rect, rect->fill_opacity, rect->fill_color);
    }
    else if (name == "stroke")
    {
        if (value == "none" || value == "transparent")
            rect->stroke_opacity = 0;
        else
        {
            rect->stroke_color = read_RGB(value);
            if (rect->stroke_width == 0)
                rect->stroke_width = 1;
        }
    }
    else if (name == "x")
        rect->start.x = stof(value);
    else if (name == "y")
        rect->start.y = stof(value);
    else if (name == "width")
    {
        if (!value.empty() && value.back() == 't')
            rect->width = stof(value.substr(0, value.length() - 2));
        else
            rect->width = stof(value);
    }
    else if (name == "height")
    {
        if (!value.empty() && value.back() == 't')
            rect->height = stof(value.substr(0, value.length() - 2));
        else
            rect->height = stof(value);
    }
    else if (name == "stroke-width")
        rect->stroke_width = stof(value);
    else if (name == "transform")
        read_transform(value, rect->trans);
    else if (name == "style")
    {
        istringstream iss(trim(value));
        string tmp;
        while (getline(iss, tmp, ';'))
        {
            size_t colonPos = tmp.find(':');
            if (colonPos != string::npos)
            {
                read_rectangle(trim(tmp.substr(0, colonPos)), trim(tmp.substr(colonPos + 1)), rect);
            }
        }
    }
}

void read_ellipse(const string &name, const string &value, ellipse *elli)
{
    if (name == "fill-opacity")
        elli->fill_opacity = stof(value);
    else if (name == "stroke-opacity")
        elli->stroke_opacity = stof(value);
    else if (name == "fill")
    {
        parse_fill_attribute(value, elli, elli->fill_opacity, elli->fill_color);
    }
    else if (name == "stroke")
    {
        if (value == "none" || value == "transparent")
            elli->stroke_opacity = 0;
        else
        {
            elli->stroke_color = read_RGB(value);
            if (elli->stroke_width == 0)
                elli->stroke_width = 1;
        }
    }
    else if (name == "cx")
        elli->start.x = stof(value);
    else if (name == "cy")
        elli->start.y = stof(value);
    else if (name == "rx")
        elli->rx = stof(value);
    else if (name == "ry")
        elli->ry = stof(value);
    else if (name == "stroke-width")
        elli->stroke_width = stof(value);
    else if (name == "transform")
        read_transform(value, elli->trans);
    else if (name == "style")
    {
        istringstream iss(trim(value));
        string tmp;
        while (getline(iss, tmp, ';'))
        {
            size_t colonPos = tmp.find(':');
            if (colonPos != string::npos)
            {
                read_ellipse(trim(tmp.substr(0, colonPos)), trim(tmp.substr(colonPos + 1)), elli);
            }
        }
    }
}

void read_circle(const string &name, const string &value, circle *cir)
{
    if (name == "fill-opacity")
        cir->fill_opacity = stof(value);
    else if (name == "stroke-opacity")
        cir->stroke_opacity = stof(value);
    else if (name == "fill")
    {
        parse_fill_attribute(value, cir, cir->fill_opacity, cir->fill_color);
    }
    else if (name == "stroke")
    {
        if (value == "none" || value == "transparent")
            cir->stroke_opacity = 0;
        else
        {
            cir->stroke_color = read_RGB(value);
            if (cir->stroke_width == 0)
                cir->stroke_width = 1;
        }
    }
    else if (name == "cx")
        cir->center.x = stof(value);
    else if (name == "cy")
        cir->center.y = stof(value);
    else if (name == "r")
        cir->r = stof(value);
    else if (name == "stroke-width")
        cir->stroke_width = stof(value);
    else if (name == "transform")
        read_transform(value, cir->trans);
    else if (name == "style")
    {
        istringstream iss(trim(value));
        string tmp;
        while (getline(iss, tmp, ';'))
        {
            size_t colonPos = tmp.find(':');
            if (colonPos != string::npos)
            {
                read_circle(trim(tmp.substr(0, colonPos)), trim(tmp.substr(colonPos + 1)), cir);
            }
        }
    }
}

void read_polygon(const string &name, const string &value, polygon *poly)
{
    if (name == "fill-opacity")
        poly->fill_opacity = stof(value);
    else if (name == "stroke-opacity")
        poly->stroke_opacity = stof(value);
    else if (name == "fill")
    {
        parse_fill_attribute(value, poly, poly->fill_opacity, poly->fill_color);
    }
    else if (name == "stroke")
    {
        if (value == "none" || value == "transparent")
            poly->stroke_opacity = 0;
        else
        {
            poly->stroke_color = read_RGB(value);
            if (poly->stroke_width == 0)
                poly->stroke_width = 1;
        }
    }
    else if (name == "stroke-width")
        poly->stroke_width = stof(value);
    else if (name == "points")
        poly->p = read_points(value);
    else if (name == "transform")
        read_transform(value, poly->trans);
    else if (name == "style")
    {
        istringstream iss(trim(value));
        string tmp;
        while (getline(iss, tmp, ';'))
        {
            size_t colonPos = tmp.find(':');
            if (colonPos != string::npos)
            {
                read_polygon(trim(tmp.substr(0, colonPos)), trim(tmp.substr(colonPos + 1)), poly);
            }
        }
    }
}

void read_polyline(const string &name, const string &value, polyline *poly)
{
    if (name == "fill-opacity")
        poly->fill_opacity = stof(value);
    else if (name == "stroke-opacity")
        poly->stroke_opacity = stof(value);
    else if (name == "fill")
    {
        parse_fill_attribute(value, poly, poly->fill_opacity, poly->fill_color);
    }
    else if (name == "stroke")
    {
        if (value == "none" || value == "transparent")
            poly->stroke_opacity = 0;
        else
        {
            poly->stroke_color = read_RGB(value);
            if (poly->stroke_width == 0)
                poly->stroke_width = 1;
        }
    }
    else if (name == "stroke-width")
        poly->stroke_width = stof(value);
    else if (name == "points")
        poly->p = read_points(value);
    else if (name == "transform")
        read_transform(value, poly->trans);
    else if (name == "style")
    {
        istringstream iss(trim(value));
        string tmp;
        while (getline(iss, tmp, ';'))
        {
            size_t colonPos = tmp.find(':');
            if (colonPos != string::npos)
            {
                read_polyline(trim(tmp.substr(0, colonPos)), trim(tmp.substr(colonPos + 1)), poly);
            }
        }
    }
}

void read_text(const string &name, const string &value, text *tex)
{
    if (name == "x")
        tex->start.x = stof(value);
    else if (name == "y")
        tex->start.y = stof(value);
    else if (name == "font-size")
        tex->font_size = stof(value);
    else if (name == "fill")
    {
        parse_fill_attribute(value, tex, tex->fill_opacity, tex->fill_color);
    }
    else if (name == "fill-opacity")
        tex->fill_opacity = stof(value);
    else if (name == "stroke")
    {
        if (value == "none" || value == "transparent")
            tex->stroke_opacity = 0;
        else
        {
            tex->stroke_color = read_RGB(value);
            if (tex->stroke_width == 0)
                tex->stroke_width = 1;
        }
    }
    else if (name == "stroke-opacity")
        tex->stroke_opacity = stof(value);
    else if (name == "font-family")
        tex->font_family = value;
    else if (name == "font-style")
        tex->font_style = trim(value);
    else if (name == "font-weight")
        tex->font_weight = trim(value);
    else if (name == "transform")
        read_transform(value, tex->trans);
    else if (name == "dx")
        tex->dx = stof(value);
    else if (name == "dy")
        tex->dy = stof(value);
    else if (name == "text-anchor")
        tex->text_anchor = value;
    else if (name == "style")
    {
        istringstream iss(trim(value));
        string tmp;
        while (getline(iss, tmp, ';'))
        {
            size_t colonPos = tmp.find(':');
            if (colonPos != string::npos)
            {
                read_text(trim(tmp.substr(0, colonPos)), trim(tmp.substr(colonPos + 1)), tex);
            }
        }
    }
}

void read_path(const string &name, const string &value, path *pth)
{
    if (name == "fill-opacity")
    {
        pth->fill_opacity = stof(value);
        if (stof(value) > 0)
            pth->has_fill = true;
    }
    else if (name == "fill")
    {
        if (value.find("url(") != string::npos)
        {
            parse_fill_attribute(value, pth, pth->fill_opacity, pth->fill_color);
            pth->has_fill = true;
        }
        else if (value == "none" || value == "transparent")
        {
            pth->fill_opacity = 0;
            pth->has_fill = false;
        }
        else
        {
            pth->fill_color = read_RGB(value);
            pth->has_fill = true;
        }
    }
    else if (name == "stroke")
    {
        if (value == "none" || value == "transparent")
        {
            pth->stroke_opacity = 0;
            pth->has_stroke = false;
        }
        else
        {
            pth->stroke_color = read_RGB(value);
            pth->has_stroke = true;
            if (pth->stroke_width == 0)
                pth->stroke_width = 1;
        }
    }
    else if (name == "stroke-width")
        pth->stroke_width = stof(value);
    else if (name == "d")
    {
        pth->d = normalize_path_data(value);
    }
    else if (name == "transform")
        read_transform(value, pth->trans);
    else if (name == "style")
    {
        istringstream iss(trim(value));
        string tmp;
        while (getline(iss, tmp, ';'))
        {
            size_t colonPos = tmp.find(':');
            if (colonPos != string::npos)
            {
                read_path(trim(tmp.substr(0, colonPos)), trim(tmp.substr(colonPos + 1)), pth);
            }
        }
    }
}