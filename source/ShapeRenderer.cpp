#include "Shape.h"
#include <SFML/Graphics.hpp>
#include <cmath>
#include <algorithm>
#include <iostream>
#include <vector>
#include <list>
#include "Base.h"
#include "earcut.hpp"

using namespace std;
using namespace sf;

namespace mapbox
{
    namespace util
    {
        template <>
        struct nth<0, sf::Vector2f>
        {
            static auto get(const sf::Vector2f &t) { return t.x; }
        };
        template <>
        struct nth<1, sf::Vector2f>
        {
            static auto get(const sf::Vector2f &t) { return t.y; }
        };
    }
}

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

Color getGradientColor(Vector2f pos, const Gradient &grad, const FloatRect &bounds)
{
    if (grad.stops.empty())
        return Color::Black;

    Vector2f p = pos;
    if (grad.units == "objectBoundingBox")
    {
        if (bounds.size.x > 0)
            p.x = (pos.x - bounds.position.x) / bounds.size.x;
        if (bounds.size.y > 0)
            p.y = (pos.y - bounds.position.y) / bounds.size.y;
    }

    p = grad.transform.getInverse().transformPoint(p);
    float t = 0;
    if (grad.type == "linear")
    {
        Vector2f start(grad.x1, grad.y1);
        Vector2f end(grad.x2, grad.y2);
        Vector2f V = end - start;
        float lenSq = V.x * V.x + V.y * V.y;
        if (lenSq > 0.00001f)
        {
            Vector2f W = p - start;
            t = (W.x * V.x + W.y * V.y) / lenSq;
        }
    }
    else
    {
        Vector2f center(grad.cx, grad.cy);
        float dist = sqrt(pow(p.x - center.x, 2) + pow(p.y - center.y, 2));
        if (grad.r > 0)
            t = dist / grad.r;
    }

    t = std::max(0.0f, std::min(1.0f, t));

    if (t <= grad.stops.front().offset)
        return grad.stops.front().color;
    if (t >= grad.stops.back().offset)
        return grad.stops.back().color;

    GradientStop s1 = grad.stops.front();
    GradientStop s2 = grad.stops.back();

    for (size_t i = 0; i < grad.stops.size() - 1; i++)
    {
        if (t >= grad.stops[i].offset && t <= grad.stops[i + 1].offset)
        {
            s1 = grad.stops[i];
            s2 = grad.stops[i + 1];
            break;
        }
    }

    float local_t = (s2.offset == s1.offset) ? 0 : (t - s1.offset) / (s2.offset - s1.offset);

    auto lerp = [](uint8_t a, uint8_t b, float t) -> uint8_t
    {
        float val = a + (b - a) * t;
        return static_cast<uint8_t>(std::max(0.0f, std::min(255.0f, val)));
    };

    return Color(
        lerp(s1.color.r, s2.color.r, local_t),
        lerp(s1.color.g, s2.color.g, local_t),
        lerp(s1.color.b, s2.color.b, local_t),
        lerp(s1.color.a, s2.color.a, local_t));
}

vector<Vector2f> getArcPoints(Vector2f currentPos, float rx, float ry, float rotation,
                              bool largeArc, bool sweep, float x, float y)
{
    vector<Vector2f> points;
    if (rx == 0 || ry == 0)
    {
        points.push_back(Vector2f(x, y));
        return points;
    }
    rx = abs(rx);
    ry = abs(ry);
    float phi = deg2rad(rotation);
    float dx = (currentPos.x - x) / 2.0f;
    float dy = (currentPos.y - y) / 2.0f;
    float x1p = cos(phi) * dx + sin(phi) * dy;
    float y1p = -sin(phi) * dx + cos(phi) * dy;
    float lambda = (x1p * x1p) / (rx * rx) + (y1p * y1p) / (ry * ry);
    if (lambda > 1.0f)
    {
        float sqRoot = sqrt(lambda);
        rx *= sqRoot;
        ry *= sqRoot;
    }
    float factor = (rx * rx * ry * ry - rx * rx * y1p * y1p - ry * ry * x1p * x1p) / (rx * rx * y1p * y1p + ry * ry * x1p * x1p);
    if (factor < 0)
        factor = 0;
    float sqFactor = sqrt(factor);
    if (largeArc == sweep)
        sqFactor = -sqFactor;
    float cxp = sqFactor * (rx * y1p / ry);
    float cyp = sqFactor * -(ry * x1p / rx);
    float cx = cos(phi) * cxp - sin(phi) * cyp + (currentPos.x + x) / 2.0f;
    float cy = sin(phi) * cxp + cos(phi) * cyp + (currentPos.y + y) / 2.0f;
    auto angle = [](float ux, float uy, float vx, float vy)
    {
        float dot = ux * vx + uy * vy;
        float len = sqrt(ux * ux + uy * uy) * sqrt(vx * vx + vy * vy);
        float ang = acos(max(-1.0f, min(1.0f, dot / len)));
        if (ux * vy - uy * vx < 0)
            ang = -ang;
        return ang;
    };
    Vector2f vp1((x1p - cxp) / rx, (y1p - cyp) / ry);
    Vector2f vp2((-x1p - cxp) / rx, (-y1p - cyp) / ry);
    float theta1 = angle(1, 0, vp1.x, vp1.y);
    float dTheta = angle(vp1.x, vp1.y, vp2.x, vp2.y);
    if (!sweep && dTheta > 0)
        dTheta -= 2 * M_PI;
    else if (sweep && dTheta < 0)
        dTheta += 2 * M_PI;
    int segments = max(15, int(abs(dTheta) * rx));
    for (int i = 1; i <= segments; i++)
    {
        float t = float(i) / segments;
        float theta = theta1 + t * dTheta;
        float px = rx * cos(theta);
        float py = ry * sin(theta);
        points.push_back(Vector2f(cos(phi) * px - sin(phi) * py + cx, sin(phi) * px + cos(phi) * py + cy));
    }
    if (!points.empty())
        points.back() = Vector2f(x, y);
    return points;
}

void rectangle::draw(RenderWindow &window, const map<string, Gradient> &gradients)
{
    Transform transform = createSFMLTransform(trans);
    if (fill_opacity > 0)
    {
        auto it = gradients.find(fill_id);
        if (!fill_id.empty() && it != gradients.end())
        {
            VertexArray quad(PrimitiveType::TriangleStrip, 4);
            Vector2f p0(start.x, start.y), p1(start.x + width, start.y), p2(start.x, start.y + height), p3(start.x + width, start.y + height);
            quad[0].position = p0;
            quad[1].position = p1;
            quad[2].position = p2;
            quad[3].position = p3;
            FloatRect bounds({start.x, start.y}, {width, height});
            quad[0].color = getGradientColor(p0, it->second, bounds);
            quad[1].color = getGradientColor(p1, it->second, bounds);
            quad[2].color = getGradientColor(p2, it->second, bounds);
            quad[3].color = getGradientColor(p3, it->second, bounds);
            window.draw(quad, transform);
        }
        else
        {
            RectangleShape rect(Vector2f(width, height));
            rect.setPosition(start);
            rect.setFillColor(Color(fill_color.r, fill_color.g, fill_color.b, static_cast<uint8_t>(fill_opacity * 255)));
            window.draw(rect, transform);
        }
    }
    if (stroke_opacity > 0 && stroke_width > 0)
    {
        RectangleShape rect(Vector2f(width, height));
        rect.setPosition(start);
        rect.setFillColor(Color::Transparent);
        rect.setOutlineColor(Color(stroke_color.r, stroke_color.g, stroke_color.b, static_cast<uint8_t>(stroke_opacity * 255)));
        rect.setOutlineThickness(stroke_width);
        window.draw(rect, transform);
    }
}
void circle::draw(RenderWindow &window, const map<string, Gradient> &gradients)
{
    Transform transform = createSFMLTransform(trans);
    if (fill_opacity > 0)
    {
        auto it = gradients.find(fill_id);
        if (!fill_id.empty() && it != gradients.end())
        {
            const int pts = 60;
            VertexArray fan(PrimitiveType::TriangleFan, pts + 2);
            fan[0].position = center;
            FloatRect bounds({center.x - r, center.y - r}, {r * 2, r * 2});
            fan[0].color = getGradientColor(center, it->second, bounds);
            for (int i = 0; i <= pts; ++i)
            {
                float a = i * 2 * M_PI / pts;
                Vector2f p(center.x + r * cos(a), center.y + r * sin(a));
                fan[i + 1].position = p;
                fan[i + 1].color = getGradientColor(p, it->second, bounds);
            }
            window.draw(fan, transform);
        }
        else
        {
            CircleShape s(r);
            s.setPosition(Vector2f(center.x - r, center.y - r));
            s.setFillColor(Color(fill_color.r, fill_color.g, fill_color.b, static_cast<uint8_t>(fill_opacity * 255)));
            window.draw(s, transform);
        }
    }
    if (stroke_opacity > 0 && stroke_width > 0)
    {
        CircleShape s(r);
        s.setPosition(Vector2f(center.x - r, center.y - r));
        s.setFillColor(Color::Transparent);
        s.setOutlineColor(Color(stroke_color.r, stroke_color.g, stroke_color.b, static_cast<uint8_t>(stroke_opacity * 255)));
        s.setOutlineThickness(stroke_width);
        window.draw(s, transform);
    }
}
void line::draw(RenderWindow &window, const map<string, Gradient> &gradients)
{
    Transform transform = createSFMLTransform(trans);
    if (stroke_opacity > 0 && stroke_width > 0)
    {
        float dx = end.x - start.x, dy = end.y - start.y;
        float len = sqrt(dx * dx + dy * dy);
        if (len > 0)
        {
            RectangleShape r(Vector2f(len, stroke_width));
            r.setPosition(Vector2f(start.x, start.y - stroke_width / 2));
            r.setRotation(sf::radians(atan2(dy, dx)));
            r.setFillColor(Color(stroke_color.r, stroke_color.g, stroke_color.b, static_cast<uint8_t>(stroke_opacity * 255)));
            window.draw(r, transform);
        }
    }
}
void ellipse::draw(RenderWindow &window, const map<string, Gradient> &gradients)
{
    Transform transform = createSFMLTransform(trans);
    if (fill_opacity > 0)
    {
        auto it = gradients.find(fill_id);
        if (!fill_id.empty() && it != gradients.end())
        {
            const int pts = 60;
            VertexArray fan(PrimitiveType::TriangleFan, pts + 2);
            fan[0].position = start;
            FloatRect bounds({start.x - rx, start.y - ry}, {rx * 2, ry * 2});
            fan[0].color = getGradientColor(start, it->second, bounds);
            for (int i = 0; i <= pts; ++i)
            {
                float a = i * 2 * M_PI / pts;
                Vector2f p(start.x + rx * cos(a), start.y + ry * sin(a));
                fan[i + 1].position = p;
                fan[i + 1].color = getGradientColor(p, it->second, bounds);
            }
            window.draw(fan, transform);
        }
        else
        {
            CircleShape s(rx);
            s.setScale(Vector2f(1.0f, ry / rx));
            s.setPosition(Vector2f(start.x - rx, start.y - ry));
            s.setFillColor(Color(fill_color.r, fill_color.g, fill_color.b, static_cast<uint8_t>(fill_opacity * 255)));
            window.draw(s, transform);
        }
    }
    if (stroke_opacity > 0 && stroke_width > 0)
    {
        CircleShape s(rx);
        s.setScale(Vector2f(1.0f, ry / rx));
        s.setPosition(Vector2f(start.x - rx, start.y - ry));
        s.setFillColor(Color::Transparent);
        s.setOutlineColor(Color(stroke_color.r, stroke_color.g, stroke_color.b, static_cast<uint8_t>(stroke_opacity * 255)));
        s.setOutlineThickness(stroke_width);
        window.draw(s, transform);
    }
}
void polygon::draw(RenderWindow &window, const map<string, Gradient> &gradients)
{
    if (p.empty())
        return;
    Transform transform = createSFMLTransform(trans);
    if (fill_opacity > 0)
    {
        auto it = gradients.find(fill_id);
        if (!fill_id.empty() && it != gradients.end())
        {
            float minX = p[0].x, maxX = p[0].x, minY = p[0].y, maxY = p[0].y;
            for (const auto &pt : p)
            {
                if (pt.x < minX)
                    minX = pt.x;
                if (pt.x > maxX)
                    maxX = pt.x;
                if (pt.y < minY)
                    minY = pt.y;
                if (pt.y > maxY)
                    maxY = pt.y;
            }
            FloatRect bounds({minX, minY}, {maxX - minX, maxY - minY});
            VertexArray poly(PrimitiveType::TriangleFan, p.size());
            for (size_t i = 0; i < p.size(); i++)
            {
                poly[i].position = p[i];
                poly[i].color = getGradientColor(p[i], it->second, bounds);
            }
            window.draw(poly, transform);
        }
        else
        {
            ConvexShape s;
            s.setPointCount(p.size());
            for (size_t i = 0; i < p.size(); i++)
                s.setPoint(i, p[i]);
            s.setFillColor(Color(fill_color.r, fill_color.g, fill_color.b, static_cast<uint8_t>(fill_opacity * 255)));
            window.draw(s, transform);
        }
    }
    if (stroke_opacity > 0 && stroke_width > 0)
    {
        ConvexShape s;
        s.setPointCount(p.size());
        for (size_t i = 0; i < p.size(); i++)
            s.setPoint(i, p[i]);
        s.setFillColor(Color::Transparent);
        s.setOutlineColor(Color(stroke_color.r, stroke_color.g, stroke_color.b, static_cast<uint8_t>(stroke_opacity * 255)));
        s.setOutlineThickness(stroke_width);
        window.draw(s, transform);
    }
}
void polyline::draw(sf::RenderWindow &window, const map<string, Gradient> &gradients)
{
    if (p.size() < 2)
        return;
    sf::Color stroke(stroke_color.r, stroke_color.g, stroke_color.b, static_cast<std::uint8_t>(stroke_opacity * 255.f));
    sf::VertexArray lines(sf::PrimitiveType::Lines);
    for (std::size_t i = 0; i + 1 < p.size(); ++i)
    {
        lines.append(sf::Vertex(p[i], stroke));
        lines.append(sf::Vertex(p[i + 1], stroke));
    }
    if (closed)
    {
        lines.append(sf::Vertex(p.back(), stroke));
        lines.append(sf::Vertex(p.front(), stroke));
    }
    window.draw(lines, createSFMLTransform(trans));
}
void text::draw(RenderWindow &window, const map<string, Gradient> &gradients)
{
    if (text_.empty())
        return;
    std::string processedText = text_;
    for (char &c : processedText)
        if (c == '\n')
            c = ' ';
    sf::Font font;
    bool loaded = false;
    vector<string> fonts = {"./Arial.ttf", "/System/Library/Fonts/Supplemental/Arial.ttf", "C:/Windows/Fonts/Arial.ttf"};
    for (auto &path : fonts)
    {
        if (font.openFromFile(path))
        {
            loaded = true;
            break;
        }
    }
    if (!loaded && !font.openFromFile("./Arial.ttf"))
        return;
    sf::Text sfText(font, processedText);
    sfText.setCharacterSize(static_cast<unsigned int>(font_size));
    sfText.setFillColor(sf::Color(fill_color.r, fill_color.g, fill_color.b, static_cast<uint8_t>(fill_opacity * 255)));
    if (stroke_width > 0 && stroke_opacity > 0)
    {
        sfText.setOutlineColor(sf::Color(stroke_color.r, stroke_color.g, stroke_color.b, static_cast<uint8_t>(stroke_opacity * 255)));
        sfText.setOutlineThickness(stroke_width);
    }
    sfText.setPosition(Vector2f(start.x + dx, start.y + dy));
    window.draw(sfText, createSFMLTransform(trans));
}

bool isPointInsidePolygon(const Vector2f &pt, const vector<Vector2f> &polygon)
{
    if (polygon.size() < 3)
        return false;
    bool inside = false;
    size_t n = polygon.size();
    for (size_t i = 0, j = n - 1; i < n; j = i++)
    {
        if (((polygon[i].y > pt.y) != (polygon[j].y > pt.y)) &&
            (pt.x < (polygon[j].x - polygon[i].x) * (pt.y - polygon[i].y) / (polygon[j].y - polygon[i].y) + polygon[i].x))
        {
            inside = !inside;
        }
    }
    return inside;
}

bool isPolyInsidePoly(const vector<Vector2f> &inner, const vector<Vector2f> &outer)
{
    if (inner.empty() || outer.empty())
        return false;
    return isPointInsidePolygon(inner[0], outer);
}

void path::draw(RenderWindow &window, const map<string, Gradient> &gradients)
{
    try
    {
        auto commands = parsePathData();
        renderPath(window, commands, gradients);
    }
    catch (...)
    {
    }
}

void path::renderPath(sf::RenderWindow &window, const std::vector<PathCommand> &commands, const map<string, Gradient> &gradients) const
{
    using Vec2 = sf::Vector2f;

    auto cubicBezierPoints = [](const Vec2 &p0, const Vec2 &p1, const Vec2 &p2, const Vec2 &p3, int segments = 20)
    {
        std::vector<Vec2> pts;
        for (int i = 1; i <= segments; ++i)
        {
            float t = float(i) / segments, u = 1.f - t;

            float b0 = u * u * u, b1 = 3 * u * u * t, b2 = 3 * u * t * t, b3 = t * t * t;
            pts.push_back({b0 * p0.x + b1 * p1.x + b2 * p2.x + b3 * p3.x,
                           b0 * p0.y + b1 * p1.y + b2 * p2.y + b3 * p3.y});
        }
        return pts;
    };

    auto quadraticBezierPoints = [](const Vec2 &p0, const Vec2 &p1, const Vec2 &p2, int segments = 20)
    {
        std::vector<Vec2> pts;
        for (int i = 1; i <= segments; ++i)
        {
            float t = float(i) / segments, u = 1.f - t;

            pts.push_back({u * u * p0.x + 2 * u * t * p1.x + t * t * p2.x,
                           u * u * p0.y + 2 * u * t * p1.y + t * t * p2.y});
        }
        return pts;
    };

    auto reflect = [&](Vec2 c, Vec2 pivot)
    { return Vec2(2 * pivot.x - c.x, 2 * pivot.y - c.y); };

    std::vector<std::vector<Vec2>> subpaths;
    std::vector<Vec2> currentPath;
    Vec2 currentPos(0, 0), startPos(0, 0), lastC2(0, 0);
    bool hasCurrentSubpath = false;
    bool hasPrevCubic = false;
    bool hasPrevQuad = false;

    for (const auto &cmd : commands)
    {
        char T = std::toupper(cmd.type);
        bool rel = std::islower(cmd.type);
        const auto &v = cmd.values;

        switch (T)
        {
        case 'M':
            if (!currentPath.empty())
            {
                subpaths.push_back(currentPath);
                currentPath.clear();
            }
            if (v.size() >= 2)
            {
                currentPos = rel ? Vec2(currentPos.x + v[0], currentPos.y + v[1]) : Vec2(v[0], v[1]);
                startPos = currentPos;
                currentPath.push_back(currentPos);
                hasCurrentSubpath = true;
                hasPrevCubic = false;
                hasPrevQuad = false;
                for (size_t i = 2; i + 1 < v.size(); i += 2)
                {
                    currentPos = rel ? Vec2(currentPos.x + v[i], currentPos.y + v[i + 1]) : Vec2(v[i], v[i + 1]);
                    currentPath.push_back(currentPos);
                }
            }
            break;
        case 'L':
            if (!hasCurrentSubpath)
                break;
            for (size_t i = 0; i + 1 < v.size(); i += 2)
            {
                currentPos = rel ? Vec2(currentPos.x + v[i], currentPos.y + v[i + 1]) : Vec2(v[i], v[i + 1]);
                currentPath.push_back(currentPos);
            }
            hasPrevCubic = false;
            hasPrevQuad = false;
            break;
        case 'H':
            if (!hasCurrentSubpath)
                break;
            for (float val : v)
            {
                currentPos = rel ? Vec2(currentPos.x + val, currentPos.y) : Vec2(val, currentPos.y);
                currentPath.push_back(currentPos);
            }
            hasPrevCubic = false;
            hasPrevQuad = false;
            break;
        case 'V':
            if (!hasCurrentSubpath)
                break;
            for (float val : v)
            {
                currentPos = rel ? Vec2(currentPos.x, currentPos.y + val) : Vec2(currentPos.x, val);
                currentPath.push_back(currentPos);
            }
            hasPrevCubic = false;
            hasPrevQuad = false;
            break;
        case 'C':
            if (!hasCurrentSubpath)
                break;
            for (size_t i = 0; i + 5 < v.size(); i += 6)
            {
                Vec2 c1 = rel ? Vec2(currentPos.x + v[i], currentPos.y + v[i + 1]) : Vec2(v[i], v[i + 1]);
                Vec2 c2 = rel ? Vec2(currentPos.x + v[i + 2], currentPos.y + v[i + 3]) : Vec2(v[i + 2], v[i + 3]);
                Vec2 end = rel ? Vec2(currentPos.x + v[i + 4], currentPos.y + v[i + 5]) : Vec2(v[i + 4], v[i + 5]);
                auto pts = cubicBezierPoints(currentPos, c1, c2, end);
                currentPath.insert(currentPath.end(), pts.begin(), pts.end());
                currentPos = end;
                lastC2 = c2;
                hasPrevCubic = true;
                hasPrevQuad = false;
            }
            break;
        case 'S':
            if (!hasCurrentSubpath)
                break;
            for (size_t i = 0; i + 3 < v.size(); i += 4)
            {
                Vec2 c1 = hasPrevCubic ? reflect(lastC2, currentPos) : currentPos;
                Vec2 c2 = rel ? Vec2(currentPos.x + v[i], currentPos.y + v[i + 1]) : Vec2(v[i], v[i + 1]);
                Vec2 end = rel ? Vec2(currentPos.x + v[i + 2], currentPos.y + v[i + 3]) : Vec2(v[i + 2], v[i + 3]);
                auto pts = cubicBezierPoints(currentPos, c1, c2, end);
                currentPath.insert(currentPath.end(), pts.begin(), pts.end());
                currentPos = end;
                lastC2 = c2;
                hasPrevCubic = true;
                hasPrevQuad = false;
            }
            break;

        case 'Q':
            if (!hasCurrentSubpath)
                break;
            for (size_t i = 0; i + 3 < v.size(); i += 4)
            {
                Vec2 c1 = rel ? Vec2(currentPos.x + v[i], currentPos.y + v[i + 1]) : Vec2(v[i], v[i + 1]);
                Vec2 end = rel ? Vec2(currentPos.x + v[i + 2], currentPos.y + v[i + 3]) : Vec2(v[i + 2], v[i + 3]);
                auto pts = quadraticBezierPoints(currentPos, c1, end);
                currentPath.insert(currentPath.end(), pts.begin(), pts.end());
                currentPos = end;
                lastC2 = c1;
                hasPrevQuad = true;
                hasPrevCubic = false;
            }
            break;

        case 'T':
            if (!hasCurrentSubpath)
                break;
            for (size_t i = 0; i + 1 < v.size(); i += 2)
            {

                Vec2 c1 = hasPrevQuad ? reflect(lastC2, currentPos) : currentPos;
                Vec2 end = rel ? Vec2(currentPos.x + v[i], currentPos.y + v[i + 1]) : Vec2(v[i], v[i + 1]);
                auto pts = quadraticBezierPoints(currentPos, c1, end);
                currentPath.insert(currentPath.end(), pts.begin(), pts.end());
                currentPos = end;
                lastC2 = c1;
                hasPrevQuad = true;
                hasPrevCubic = false;
            }
            break;

        case 'A':
            if (!hasCurrentSubpath)
                break;
            for (size_t i = 0; i + 6 < v.size(); i += 7)
            {
                float rx = v[i], ry = v[i + 1], rot = v[i + 2];
                bool large = (v[i + 3] > 0.5f), sweep = (v[i + 4] > 0.5f);
                Vec2 end = rel ? Vec2(currentPos.x + v[i + 5], currentPos.y + v[i + 6]) : Vec2(v[i + 5], v[i + 6]);
                auto pts = getArcPoints(currentPos, rx, ry, rot, large, sweep, end.x, end.y);
                currentPath.insert(currentPath.end(), pts.begin(), pts.end());
                currentPos = end;
                hasPrevCubic = false;
                hasPrevQuad = false;
            }
            break;
        case 'Z':
            if (!hasCurrentSubpath)
                break;
            if (currentPath.back() != startPos)
                currentPath.push_back(startPos);
            subpaths.push_back(currentPath);
            currentPath.clear();
            currentPos = startPos;
            hasCurrentSubpath = false;
            hasPrevCubic = false;
            hasPrevQuad = false;
            break;
        }
    }
    if (!currentPath.empty())
        subpaths.push_back(currentPath);

    if (has_fill && getFillColor().a > 0)
    {
        sf::Color fillColor = getFillColor();
        sf::Transform transform = createSFMLTransform(trans);

        vector<int> nestingLevel(subpaths.size(), 0);
        for (size_t i = 0; i < subpaths.size(); ++i)
        {
            if (subpaths[i].size() < 3)
                continue;
            for (size_t j = 0; j < subpaths.size(); ++j)
            {
                if (i == j)
                    continue;
                if (subpaths[j].size() < 3)
                    continue;
                if (isPolyInsidePoly(subpaths[i], subpaths[j]))
                    nestingLevel[i]++;
            }
        }

        struct RenderGroup
        {
            size_t solidIndex;
            vector<size_t> holeIndices;
        };
        vector<RenderGroup> groups;

        for (size_t i = 0; i < subpaths.size(); ++i)
        {
            if (subpaths[i].size() < 3)
                continue;
            if (nestingLevel[i] % 2 == 0)
                groups.push_back({i, {}});
        }
        for (size_t i = 0; i < subpaths.size(); ++i)
        {
            if (subpaths[i].size() < 3)
                continue;
            if (nestingLevel[i] % 2 != 0)
            {
                int parentId = -1;
                for (size_t g = 0; g < groups.size(); ++g)
                {
                    size_t sIdx = groups[g].solidIndex;
                    if (isPolyInsidePoly(subpaths[i], subpaths[sIdx]))
                    {
                        if (parentId == -1 || nestingLevel[sIdx] > nestingLevel[groups[parentId].solidIndex])
                        {
                            parentId = g;
                        }
                    }
                }
                if (parentId != -1)
                    groups[parentId].holeIndices.push_back(i);
            }
        }

        float minX = 0, maxX = 0, minY = 0, maxY = 0;
        bool first = true;
        if (!fill_id.empty())
        {
            for (const auto &sp : subpaths)
                for (const auto &p : sp)
                {
                    if (first)
                    {
                        minX = maxX = p.x;
                        minY = maxY = p.y;
                        first = false;
                    }
                    else
                    {
                        if (p.x < minX)
                            minX = p.x;
                        if (p.x > maxX)
                            maxX = p.x;
                        if (p.y < minY)
                            minY = p.y;
                        if (p.y > maxY)
                            maxY = p.y;
                    }
                }
        }
        FloatRect globalBounds({minX, minY}, {maxX - minX, maxY - minY});
        const Gradient *activeGrad = (!fill_id.empty() && gradients.count(fill_id)) ? &gradients.at(fill_id) : nullptr;

        for (const auto &group : groups)
        {
            std::vector<std::vector<Vec2>> polygonInput;
            polygonInput.push_back(subpaths[group.solidIndex]);
            for (size_t hIdx : group.holeIndices)
                polygonInput.push_back(subpaths[hIdx]);
            std::vector<uint32_t> indices = mapbox::earcut<uint32_t>(polygonInput);
            std::vector<Vec2> flatPoints;
            for (const auto &ring : polygonInput)
                flatPoints.insert(flatPoints.end(), ring.begin(), ring.end());

            if (!indices.empty())
            {
                sf::VertexArray tri(sf::PrimitiveType::Triangles, indices.size());
                for (size_t k = 0; k < indices.size(); ++k)
                {
                    tri[k].position = flatPoints[indices[k]];
                    if (activeGrad)
                        tri[k].color = getGradientColor(tri[k].position, *activeGrad, globalBounds);
                    else
                        tri[k].color = fillColor;
                }
                window.draw(tri, sf::RenderStates(transform));
            }
        }
    }

    if (has_stroke && stroke_width > 0.1f)
    {
        sf::Color strokeColor = getStrokeColor();
        sf::Transform transform = createSFMLTransform(trans);
        for (const auto &p : subpaths)
        {
            for (size_t i = 0; i + 1 < p.size(); ++i)
            {
                sf::VertexArray line(sf::PrimitiveType::Lines, 2);
                line[0].position = p[i];
                line[0].color = strokeColor;
                line[1].position = p[i + 1];
                line[1].color = strokeColor;
                window.draw(line, sf::RenderStates(transform));
            }
        }
    }
}

vector<path::PathCommand> path::parsePathData() const
{
    vector<PathCommand> cmds;
    size_t i = 0;
    auto &data = d;
    auto skipWhitespace = [&]()
    { while (i < data.size() && (isspace(data[i]) || data[i] == ',')) i++; };
    auto readNumber = [&]() -> float
    {
        skipWhitespace();
        if (i >= data.size())
            throw std::runtime_error("Unexpected end");
        size_t start = i;
        if (data[i] == '+' || data[i] == '-')
            i++;
        bool hasDigit = false;
        while (i < data.size() && isdigit(data[i]))
        {
            i++;
            hasDigit = true;
        }
        if (i < data.size() && data[i] == '.')
        {
            i++;
            while (i < data.size() && isdigit(data[i]))
            {
                i++;
                hasDigit = true;
            }
        }
        if (hasDigit && i < data.size() && (data[i] == 'e' || data[i] == 'E'))
        {
            i++;
            if (i < data.size() && (data[i] == '+' || data[i] == '-'))
                i++;
            while (i < data.size() && isdigit(data[i]))
                i++;
        }
        if (!hasDigit)
            throw std::runtime_error("Invalid number");
        return clarifyFloat(data.substr(start, i - start));
    };
    auto readFlag = [&]() -> float
    {
        skipWhitespace();
        if (i >= data.size())
            throw std::runtime_error("Unexpected end");
        if (data[i] == '0' || data[i] == '1')
        {
            return (data[i++] == '1') ? 1.0f : 0.0f;
        }
        throw std::runtime_error("Invalid flag");
    };
    while (i < data.size())
    {
        skipWhitespace();
        if (i >= data.size())
            break;
        char type = data[i++];
        PathCommand cmd;
        cmd.type = type;
        while (i < data.size())
        {
            skipWhitespace();
            if (i < data.size() && isalpha(data[i]) && data[i] != 'e' && data[i] != 'E')
                break;
            if (toupper(type) == 'Z')
                break;
            if (toupper(type) == 'A')
            {
                cmd.values.push_back(readNumber());
                cmd.values.push_back(readNumber());
                cmd.values.push_back(readNumber());
                cmd.values.push_back(readFlag());
                cmd.values.push_back(readFlag());
                cmd.values.push_back(readNumber());
                cmd.values.push_back(readNumber());
            }
            else
            {
                int bs = 0;
                switch (toupper(type))
                {
                case 'M':
                case 'L':
                case 'T':
                    bs = 2;
                    break;
                case 'H':
                case 'V':
                    bs = 1;
                    break;
                case 'C':
                    bs = 6;
                    break;
                case 'S':
                case 'Q':
                    bs = 4;
                    break;
                }
                for (int k = 0; k < bs; k++)
                    cmd.values.push_back(readNumber());
            }
        }
        cmds.push_back(cmd);
    }
    return cmds;
}
