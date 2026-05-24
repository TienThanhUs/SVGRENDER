#include "Base.h"
#include "SVGParse.h"
#include <cstdint>
#include <regex>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <iostream>
#include <vector>

using namespace std;
using namespace sf;

string trim(string str)
{
    str.erase(remove_if(str.begin(), str.end(), ::isspace), str.end());
    return str;
}

bool check(char a)
{
    if (a <= 'Z' && a >= 'A')
        return true;
    if (a <= 'z' && a >= 'a')
        return true;
    if (a <= '9' && a >= '0')
        return true;
    if (a == '(' || a == ')')
        return true;
    return false;
}

void remove_space(string &s)
{
    for (int i = 1; i < s.length() - 1; i++)
    {
        if (!check(s[i]))
        {
            if (s[i - 1] <= '9' && s[i - 1] >= '0' &&
                ((s[i + 1] <= '9' && s[i + 1] >= '0') || s[i + 1] == '-' || s[i + 1] == '.') &&
                s[i] != '.')
            {
                s[i] = ',';
                continue;
            }
            else if (s[i] != '.' && s[i] != '-')
            {
                s.erase(i, 1);
                i--;
            }
        }
    }
}

float clarifyFloat(string s)
{
    if (s.empty())
        return 0.0f;
    if (s[0] == '.')
        s.insert(0, "0");
    if (s.length() > 1 && s[0] == '-' && s[1] == '.')
        s.insert(1, "0");
    if (s.back() == '%')
    {
        return stof(s.substr(0, s.length() - 1)) / 100;
    }
    try
    {
        return stof(s);
    }
    catch (...)
    {
        return 0.0f;
    }
}

Color read_RGB(string value)
{
    Color colour = Color::Black;
    if (value.empty())
        return colour;

    if (value.find("rgb") != string::npos)
    {
        int r = 0, g = 0, b = 0;
        size_t start = value.find("(") + 1;
        size_t end = value.find(")");
        if (start < end)
        {
            string content = value.substr(start, end - start);
            replace(content.begin(), content.end(), ',', ' ');
            stringstream ss(content);
            ss >> r >> g >> b;
        }
        return Color((uint8_t)min(r, 255), (uint8_t)min(g, 255), (uint8_t)min(b, 255));
    }

    if (value[0] == '#')
    {
        string hexStr = value.substr(1);
        if (hexStr.size() == 3)
        {
            string r(2, hexStr[0]);
            string g(2, hexStr[1]);
            string b(2, hexStr[2]);
            hexStr = r + g + b;
        }
        unsigned int hexValue = 0;
        stringstream ss;
        ss << hex << hexStr;
        ss >> hexValue;

        return Color((hexValue >> 16) & 0xFF, (hexValue >> 8) & 0xFF, hexValue & 0xFF);
    }

    return Color::Black;
}

vector<Vector2f> read_points(string value)
{
    vector<Vector2f> points;
    value = regex_replace(value, regex("^\\s+|\\s+$"), "");
    stringstream ss(value);
    string pointStr;

    while (getline(ss, pointStr, ' '))
    {
        Vector2f p;
        if (pointStr.find(',') == string::npos)
        {
            p.x = clarifyFloat(pointStr);
            if (getline(ss, pointStr, ' '))
                p.y = clarifyFloat(pointStr);
        }
        else
        {
            replace(pointStr.begin(), pointStr.end(), ',', ' ');
            stringstream pointStream(pointStr);
            string coord;
            pointStream >> coord;
            p.x = clarifyFloat(coord);
            pointStream >> coord;
            p.y = clarifyFloat(coord);
        }
        points.push_back(p);
    }
    return points;
}

void read_transform(string value, multi_transform &tr)
{
    remove_space(value);
    size_t pos = 0;
    while (pos < value.length())
    {
        if (value[pos] == ',' || value[pos] == ' ')
        {
            pos++;
            continue;
        }

        if (value.compare(pos, 5, "scale") == 0)
        {
            tr.types.push_back("scale");
            pos += 5;
            size_t start = value.find('(', pos);
            size_t end = value.find(')', start);
            string c = value.substr(start + 1, end - start - 1);
            stringstream ss(c);
            string x, y;
            getline(ss, x, ',');
            getline(ss, y, ',');
            tr.values.push_back(clarifyFloat(x));
            tr.values.push_back(y.empty() ? clarifyFloat(x) : clarifyFloat(y));
            pos = end + 1;
        }
        else if (value.compare(pos, 6, "rotate") == 0)
        {
            tr.types.push_back("rotate");
            pos += 6;
            size_t start = value.find('(', pos);
            size_t end = value.find(')', start);
            tr.values.push_back(clarifyFloat(value.substr(start + 1, end - start - 1)));
            pos = end + 1;
        }
        else if (value.compare(pos, 9, "translate") == 0)
        {
            tr.types.push_back("translate");
            pos += 9;
            size_t start = value.find('(', pos);
            size_t end = value.find(')', start);
            stringstream ss(value.substr(start + 1, end - start - 1));
            string x, y;
            getline(ss, x, ',');
            getline(ss, y, ',');
            tr.values.push_back(clarifyFloat(x));
            tr.values.push_back(y.empty() ? 0.0f : clarifyFloat(y));
            pos = end + 1;
        }
        else
            pos++;
    }
}

Transform createSFMLTransform(const multi_transform &trans)
{
    Transform transform = Transform::Identity;
    size_t idx = 0;
    for (const string &type : trans.types)
    {
        if (type == "rotate")
        {
            if (idx < trans.values.size())
                transform.rotate(sf::degrees(trans.values[idx++]));
        }
        else if (type == "scale")
        {
            if (idx + 1 < trans.values.size())
            {
                transform.scale({trans.values[idx], trans.values[idx + 1]});
                idx += 2;
            }
        }
        else if (type == "translate")
        {
            if (idx + 1 < trans.values.size())
            {
                transform.translate({trans.values[idx], trans.values[idx + 1]});
                idx += 2;
            }
        }
    }
    return transform;
}

Transform parseTransformString(string s)
{
    Transform t = Transform::Identity;
    for (char &c : s)
    {
        if (c == ',' || c == '(' || c == ')')
            c = ' ';
    }

    stringstream ss(s);
    string token;
    while (ss >> token)
    {
        if (token.find("matrix") != string::npos)
        {
            float a, b, c, d, e, f;
            ss >> a >> b >> c >> d >> e >> f;
            t.combine(Transform(a, c, e, b, d, f, 0, 0, 1));
        }
        else if (token.find("scale") != string::npos)
        {
            float sx, sy;
            ss >> sx;
            if (!(ss >> sy))
                sy = sx;
            t.scale({sx, sy});
        }
        else if (token.find("rotate") != string::npos)
        {
            float angle, cx = 0, cy = 0;
            ss >> angle;
            if (ss >> cx >> cy)
            {
                t.rotate(sf::degrees(angle), {cx, cy});
            }
            else
            {
                t.rotate(sf::degrees(angle));
            }
        }
        else if (token.find("translate") != string::npos)
        {
            float tx, ty = 0;
            ss >> tx;
            if (ss >> ty)
            {
            };
            t.translate({tx, ty});
        }
    }
    return t;
}

void viewBox::setViewBoxAttribute(const string &value)
{
    stringstream ss(value);
    ss >> x >> y >> width >> height;
}

realtime_transform::realtime_transform()
{
    zoomLevel = 1.0f;
    rotationAngle = 0.0f;
}

vector<char> getFile(const string &filename)
{
    ifstream file(filename, ios::binary | ios::ate);
    if (!file)
        return {};
    size_t size = file.tellg();
    file.seekg(0);
    vector<char> buffer(size + 1);
    file.read(buffer.data(), size);
    buffer[size] = '\0';
    return buffer;
}

void read_file(SVGParse &parse)
{
    ifstream file(parse.filename);
    if (!file.is_open())
    {
        cout << "Error: Cannot open file" << endl;
        exit(-1);
    }

    vector<char> buffer = getFile(parse.filename);
    xml_document<> doc;
    try
    {
        doc.parse<0>(&buffer[0]);
    }
    catch (...)
    {
        cout << "XML Parse error" << endl;
        return;
    }

    xml_node<> *root = doc.first_node("svg");
    if (!root)
        return;

    parse.read_style(root);
    parse.read_defs(root);

    for (xml_attribute<> *attr = root->first_attribute(); attr; attr = attr->next_attribute())
    {
        string name = attr->name();
        string val = attr->value();
        if (name == "viewBox")
            parse.vb.setViewBoxAttribute(val);
        else if (name == "width")
            parse.frame.first = clarifyFloat(val);
        else if (name == "height")
            parse.frame.second = clarifyFloat(val);
    }
    if (parse.frame.first <= 0)
        parse.frame.first = 1000;
    if (parse.frame.second <= 0)
        parse.frame.second = 700;
    if (parse.vb.width <= 0)
    {
        parse.vb.width = parse.frame.first;
        parse.vb.height = parse.frame.second;
    }

    group g;
    g.traversal_group(root, parse.shapes, parse.globalStyles);
}

string normalize_path_data(const string &input)
{
    string res = input;
    replace(res.begin(), res.end(), ',', ' ');
    return res;
}