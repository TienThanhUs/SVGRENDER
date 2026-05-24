#include <SFML/Graphics.hpp>
#include <iostream>
#include "SVGParse.h"
#include "SVGRender.h"

using namespace sf;
using namespace std;

void handleKeyboardEvent(const Event::KeyPressed &keyEvent, realtime_transform &rtt)
{
    switch (keyEvent.code)
    {
    case Keyboard::Key::R:
        rtt.zoomLevel = 1.0f;
        rtt.rotationAngle = 0.0f;
        cout << "View reset" << endl;
        break;

    case Keyboard::Key::Equal:
        rtt.zoomLevel *= 1.5f;
        cout << "Zoom IN: " << rtt.zoomLevel << endl;
        break;

    case Keyboard::Key::Hyphen:
        rtt.zoomLevel /= 1.5f;
        cout << "Zoom OUT: " << rtt.zoomLevel << endl;
        break;

    case Keyboard::Key::Left:
        rtt.rotationAngle += 15.0f;
        cout << "Rotation LEFT: " << rtt.rotationAngle << " degrees" << endl;
        break;

    case Keyboard::Key::Right:
        rtt.rotationAngle -= 15.0f;
        cout << "Rotation RIGHT: " << rtt.rotationAngle << " degrees" << endl;
        break;

    default:
        break;
    }
}

void SVGRender::render(SVGParse &parsed)
{
    sf::ContextSettings settings;
    settings.antiAliasingLevel = 8;
    realtime_transform rtt;
    RenderWindow window(VideoMode({parsed.frame.first, parsed.frame.second}), parsed.filename);
    int index = 1;
    for (auto *shape : parsed.shapes)
    {
        cout << index++ << ". " << shape->typeOfShape << endl;
    }
    while (window.isOpen())
    {
        while (optional<Event> event = window.pollEvent())
        {
            if (event->is<Event::Closed>())
            {
                window.close();
            }

            if (auto *keyEvent = event->getIf<Event::KeyPressed>())
            {
                if (keyEvent->code == Keyboard::Key::Escape)
                    window.close();
                handleKeyboardEvent(*keyEvent, rtt);
            }
        }

        window.clear(Color::White);

        View view = window.getDefaultView();
        view.setRotation(degrees(rtt.rotationAngle));
        view.setCenter(Vector2f(parsed.vb.x + parsed.vb.width / 2.f, parsed.vb.y + parsed.vb.height / 2.f));
        view.setSize(Vector2f(parsed.vb.width, parsed.vb.height) / rtt.zoomLevel);

        window.setView(view);

        for (auto *shape : parsed.shapes)
        {
            shape->draw(window, parsed.gradients);
        }
        window.setView(window.getDefaultView());
        window.display();
    }
}

SVGRender::SVGRender(SVGParse &parsed)
{
    render(parsed);
}