#include <iostream>
#include <string>
#include <vector>
#include <regex>
#include <algorithm>
#include <SFML/Graphics.hpp>
#include "rapidxml.hpp"
#include "Shape.h"
#include "SVGParse.h"
#include "Base.h"
#include "Types.h"
#include "ShapeReader.h"

using namespace std;
using namespace sf;
using namespace rapidxml;

void SVGParse::parseCssContent(string content)
{
    content.erase(std::remove(content.begin(), content.end(), '\n'), content.end());
    content.erase(std::remove(content.begin(), content.end(), '\r'), content.end());

    std::regex cssRegex(R"(\.([\w-]+)\s*\{([^}]+)\})");
    std::smatch match;

    string::const_iterator searchStart(content.cbegin());
    while (std::regex_search(searchStart, content.cend(), match, cssRegex))
    {
        if (match.size() == 3)
        {
            string className = match[1].str();
            string styleBody = trim(match[2].str());

            this->globalStyles[className] = styleBody;
        }
        searchStart = match.suffix().first;
    }
}

SVGParse::SVGParse(const string &filename)
{
    this->filename = filename;
    this->loadSVG();
}

SVGParse::~SVGParse()
{
    clear();
}

void SVGParse::loadSVG()
{
    clear();

    read_file(*this);

    if (shapes.empty())
    {
        cout << "Warning: No shapes loaded from SVG file or parsing failed." << endl;
    }
    else
    {
        cout << "Loaded " << shapes.size() << " shapes." << endl;
        cout << "Found " << globalStyles.size() << " CSS classes." << endl;
        cout << "Found " << gradients.size() << " gradients." << endl;
    }
}

void SVGParse::clear()
{
    for (auto *shape : shapes)
    {
        delete shape;
    }
    shapes.clear();
    gradients.clear();
    globalStyles.clear();
}

void SVGParse::read_style(xml_node<> *root)
{
    xml_node<> *styleNode = root->first_node("style");
    if (styleNode && styleNode->value())
    {
        parseCssContent(styleNode->value());
    }

    xml_node<> *defs = root->first_node("defs");
    if (defs)
    {
        xml_node<> *defsStyle = defs->first_node("style");
        if (defsStyle && defsStyle->value())
        {
            parseCssContent(defsStyle->value());
        }
    }
}
void SVGParse::read_defs(xml_node<> *root)
{
    xml_node<> *defs = root->first_node("defs");
    if (!defs)
        return;
    for (xml_node<> *node = defs->first_node(); node; node = node->next_sibling())
    {
        string name = node->name();

        if (name == "linearGradient" || name == "radialGradient")
        {
            Gradient grad;
            grad.type = (name == "linearGradient") ? "linear" : "radial";

            xml_attribute<> *attr = node->first_attribute("id");
            if (attr)
                grad.id = attr->value();

            attr = node->first_attribute("xlink:href");
            if (attr)
            {
                string refId = attr->value();
                if (refId.size() > 1 && refId[0] == '#')
                    refId = refId.substr(1);
                if (gradients.count(refId))
                {
                    grad.stops = gradients[refId].stops;
                }
            }

            attr = node->first_attribute("gradientUnits");
            if (attr)
                grad.units = attr->value();

            attr = node->first_attribute("gradientTransform");
            if (attr)
                grad.transform = parseTransformString(attr->value());

            if (grad.type == "linear")
            {
                attr = node->first_attribute("x1");
                if (attr)
                    grad.x1 = clarifyFloat(attr->value());
                attr = node->first_attribute("y1");
                if (attr)
                    grad.y1 = clarifyFloat(attr->value());
                attr = node->first_attribute("x2");
                if (attr)
                    grad.x2 = clarifyFloat(attr->value());
                attr = node->first_attribute("y2");
                if (attr)
                    grad.y2 = clarifyFloat(attr->value());
            }
            else
            {
                attr = node->first_attribute("cx");
                grad.cx = attr ? clarifyFloat(attr->value()) : 0.5f;
                attr = node->first_attribute("cy");
                grad.cy = attr ? clarifyFloat(attr->value()) : 0.5f;
                attr = node->first_attribute("r");
                grad.r = attr ? clarifyFloat(attr->value()) : 0.5f;
                attr = node->first_attribute("fx");
                grad.fx = attr ? clarifyFloat(attr->value()) : grad.cx;
                attr = node->first_attribute("fy");
                grad.fy = attr ? clarifyFloat(attr->value()) : grad.cy;
            }

            if (node->first_node("stop"))
            {
                grad.stops.clear();
                for (xml_node<> *stopNode = node->first_node("stop"); stopNode; stopNode = stopNode->next_sibling("stop"))
                {
                    GradientStop stop;
                    attr = stopNode->first_attribute("offset");
                    if (attr)
                        stop.offset = clarifyFloat(attr->value());

                    attr = stopNode->first_attribute("stop-color");
                    if (attr)
                        stop.color = read_RGB(attr->value());

                    attr = stopNode->first_attribute("stop-opacity");
                    float opacity = attr ? stof(attr->value()) : 1.0f;

                    attr = stopNode->first_attribute("style");
                    if (attr)
                    {
                        string style = attr->value();
                        size_t cPos = style.find("stop-color");
                        if (cPos != string::npos)
                        {
                            size_t start = style.find(":", cPos) + 1;
                            size_t end = style.find(";", start);
                            if (end == string::npos)
                                end = style.length();
                            stop.color = read_RGB(trim(style.substr(start, end - start)));
                        }
                        size_t oPos = style.find("stop-opacity");
                        if (oPos != string::npos)
                        {
                            size_t start = style.find(":", oPos) + 1;
                            size_t end = style.find(";", start);
                            if (end == string::npos)
                                end = style.length();
                            opacity = stof(style.substr(start, end - start));
                        }
                    }
                    stop.color.a = static_cast<uint8_t>(opacity * 255);
                    grad.stops.push_back(stop);
                }
            }
            this->gradients[grad.id] = grad;
        }
    }
}

void group::traversal_group(rapidxml::xml_node<> *root, vector<shape *> &shapes, const map<string, string> &styles)
{
    xml_node<> *node = root;
    if (!node)
        return;

    for (xml_node<> *child = node->first_node(); child; child = child->next_sibling())
    {
        string name = child->name();

        string styleFromClass = "";
        xml_attribute<> *classAttr = child->first_attribute("class");
        if (classAttr)
        {
            string clsName = classAttr->value();
            if (styles.count(clsName))
            {
                styleFromClass = styles.at(clsName);
            }
        }

        auto applyAttributes = [&](shape *s, void (*reader)(const string &, const string &, void *))
        {
            for (const auto &attr : attributes)
            {

                if (s->typeOfShape == "line")
                    read_line(attr.first, attr.second, (line *)s);
                else if (s->typeOfShape == "rect")
                    read_rectangle(attr.first, attr.second, (rectangle *)s);
                else if (s->typeOfShape == "circle")
                    read_circle(attr.first, attr.second, (circle *)s);
                else if (s->typeOfShape == "ellipse")
                    read_ellipse(attr.first, attr.second, (ellipse *)s);
                else if (s->typeOfShape == "polygon")
                    read_polygon(attr.first, attr.second, (polygon *)s);
                else if (s->typeOfShape == "polyline")
                    read_polyline(attr.first, attr.second, (polyline *)s);
                else if (s->typeOfShape == "text")
                    read_text(attr.first, attr.second, (text *)s);
                else if (s->typeOfShape == "path")
                    read_path(attr.first, attr.second, (path *)s);
            }

            if (!styleFromClass.empty())
            {
                if (s->typeOfShape == "line")
                    read_line("style", styleFromClass, (line *)s);
                else if (s->typeOfShape == "rect")
                    read_rectangle("style", styleFromClass, (rectangle *)s);
                else if (s->typeOfShape == "circle")
                    read_circle("style", styleFromClass, (circle *)s);
                else if (s->typeOfShape == "ellipse")
                    read_ellipse("style", styleFromClass, (ellipse *)s);
                else if (s->typeOfShape == "polygon")
                    read_polygon("style", styleFromClass, (polygon *)s);
                else if (s->typeOfShape == "polyline")
                    read_polyline("style", styleFromClass, (polyline *)s);
                else if (s->typeOfShape == "text")
                    read_text("style", styleFromClass, (text *)s);
                else if (s->typeOfShape == "path")
                    read_path("style", styleFromClass, (path *)s);
            }

            for (xml_attribute<> *attr = child->first_attribute(); attr; attr = attr->next_attribute())
            {
                if (s->typeOfShape == "line")
                    read_line(attr->name(), attr->value(), (line *)s);
                else if (s->typeOfShape == "rect")
                    read_rectangle(attr->name(), attr->value(), (rectangle *)s);
                else if (s->typeOfShape == "circle")
                    read_circle(attr->name(), attr->value(), (circle *)s);
                else if (s->typeOfShape == "ellipse")
                    read_ellipse(attr->name(), attr->value(), (ellipse *)s);
                else if (s->typeOfShape == "polygon")
                    read_polygon(attr->name(), attr->value(), (polygon *)s);
                else if (s->typeOfShape == "polyline")
                    read_polyline(attr->name(), attr->value(), (polyline *)s);
                else if (s->typeOfShape == "text")
                    read_text(attr->name(), attr->value(), (text *)s);
                else if (s->typeOfShape == "path")
                    read_path(attr->name(), attr->value(), (path *)s);
            }
        };

        if (name == "line")
        {
            line *lin = new line();
            lin->typeOfShape = "line";
            applyAttributes(lin, nullptr);
            shapes.push_back(lin);
        }
        else if (name == "rect")
        {
            rectangle *rect = new rectangle();
            rect->typeOfShape = "rect";
            applyAttributes(rect, nullptr);
            shapes.push_back(rect);
        }
        else if (name == "ellipse")
        {
            ellipse *elli = new ellipse();
            elli->typeOfShape = "ellipse";
            applyAttributes(elli, nullptr);
            shapes.push_back(elli);
        }
        else if (name == "circle")
        {
            circle *cir = new circle();
            cir->typeOfShape = "circle";
            applyAttributes(cir, nullptr);
            shapes.push_back(cir);
        }
        else if (name == "polygon")
        {
            polygon *polyg = new polygon();
            polyg->typeOfShape = "polygon";
            applyAttributes(polyg, nullptr);
            shapes.push_back(polyg);
        }
        else if (name == "polyline")
        {
            polyline *polyl = new polyline();
            polyl->typeOfShape = "polyline";
            applyAttributes(polyl, nullptr);
            shapes.push_back(polyl);
        }
        else if (name == "text")
        {
            text *tex = new text();
            tex->typeOfShape = "text";
            applyAttributes(tex, nullptr);
            if (child->value())
                tex->text_ = child->value();
            shapes.push_back(tex);
        }
        else if (name == "path")
        {
            path *pth = new path();
            pth->typeOfShape = "path";
            applyAttributes(pth, nullptr);
            shapes.push_back(pth);
        }
        else if (name == "g")
        {
            group new_group;

            new_group.attributes = this->attributes;

            for (xml_attribute<> *attri = child->first_attribute(); attri; attri = attri->next_attribute())
            {
                string attr_name = attri->name();
                string attr_value = attri->value();

                if (attr_name == "transform" && new_group.attributes.count("transform"))
                {
                    new_group.attributes[attr_name] += " " + attr_value;
                }
                else
                {
                    new_group.attributes[attr_name] = attr_value;
                }
            }

            new_group.traversal_group(child, shapes, styles);
        }
    }
}
