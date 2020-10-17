#include <bits/stdc++.h>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/Main.hpp>
#include <SFML/System.hpp>

using namespace std;

struct Vector {
    // mathematical vector
    double x, y;

    Vector(double x, double y) {
        this->x = x;
        this->y = y;
    }

    Vector(sf::Vertex point) {
        x = point.position.x;
        y = point.position.y;
    }

    Vector operator+(const Vector &o) const {
        return {x+o.x, y+o.y};
    }
    Vector operator-(const Vector &o) const {
        return {x-o.x, y-o.y};
    }
    Vector operator*(const double &t) const {
        return {x*t, y*t};
    }
    double operator^(const Vector &o) const {
        return x*o.y - y*o.x;
    }
    double module() const {
        return sqrt(x*x + y*y);
    }
    double module_squared() const {
        return x*x + y*y;
    }
    int quarter() const {
        if(x > 0 and y >= 0) return 0;
        if(x <= 0 and y > 0) return 1;
        if(x < 0 and y <= 0) return 2;
        return 3;
    }
    void rotate_ccw(double angle) {
        // rotates vector counter clockwise
        double xant = x;
        x = x*cos(angle) - y*sin(angle);
        y = y*cos(angle) + xant*sin(angle);
    }
    void force_module(double new_module) {
        // assign new module maintaing same direction
        double mod = module();
        if(mod > 1e-7) {
            this->x = (this->x*new_module)/mod;
            this->y = (this->y*new_module)/mod;
        }
    }
};

int distance(sf::Vertex point1, sf::Vertex point2) {
    Vector vec1 = Vector(point1);
    Vector vec2 = Vector(point2);

    return (vec2-vec1).module_squared();
}

bool line_line_intersection(pair<sf::Vertex, sf::Vertex> seg1, pair<sf::Vertex, sf::Vertex> seg2, sf::Vertex& inter) {
    // line1 is segment from p to p+r, line2 is segment from q to q+s
    // intersection is p + tr = q + us if 0 <= u, s <= 1

    Vector p = Vector(seg1.first);
    Vector r = Vector(seg1.second.position.x-seg1.first.position.x, seg1.second.position.y-seg1.first.position.y);

    Vector q = Vector(seg2.first);
    Vector s = Vector(seg2.second.position.x-seg2.first.position.x, seg2.second.position.y-seg2.first.position.y);

    if((r^s) < 0) {
        swap(p, q);
        swap(r, s);
    }

    if((r^s) == 0.f) {
        return false;
    }

    double u = ((q-p)^r);
    double t = ((q-p)^s);

    if(u < 0 or u > (r^s) or t < 0 or t > (r^s)) {
        return false;
    }

    Vector tr = r*(t/(r^s));
    inter = sf::Vertex(sf::Vector2f(p.x + tr.x, p.y + tr.y));
    return true;
}

bool convex_line_intersection(sf::ConvexShape convex, pair<sf::Vertex, sf::Vertex> segment, sf::Vertex& inter) {
    // calculate intersection between convex shape and line
    // stores in inter the closest intersection point relative to line[0]
    bool intersection = false;
    int size = convex.getPointCount();

    for(int i = 0; i < size; i++) {
        pair<sf::Vertex, sf::Vertex> convex_line = make_pair(convex.getPoint(i), convex.getPoint((i+1)%size));

        sf::Vertex inter_aux;
        if(line_line_intersection(convex_line, segment, inter_aux)) {
            if(intersection) {
                if(distance(segment.first, inter_aux) < distance(segment.first, inter) ) {
                    swap(inter, inter_aux);
                }
            }
            else {
                inter = inter_aux;
                intersection = true;
            }
        }
    }

    return intersection;
}

void sort_points(sf::Vertex reference, vector<sf::Vertex>& points) {
    // sorts points counter clockwise with respect to the reference
    Vector ref_vector = Vector(reference);

    // comparation function
    auto comp = [ref_vector](sf::Vertex v1, sf::Vertex v2) {
        Vector vec1 = Vector(v1);
        Vector vec2 = Vector(v2);

        vec1 = vec1 - ref_vector;
        vec2 = vec2 - ref_vector;

        int quarter1 = vec1.quarter();
        int quarter2 = vec2.quarter();

        if(quarter1 == quarter2)
            return (vec1 ^ vec2) > 0;

        return quarter1 < quarter2;
    };

    sort(points.begin(), points.end(), comp);
}

vector<sf::Vertex> get_points(vector<sf::ConvexShape> convexes) {
    // return all vertexes from the convex shapes
    vector<sf::Vertex> points;

    for(sf::ConvexShape convex : convexes) {
        int size = convex.getPointCount();
        for(int i = 0; i < size; i++) {
            points.push_back(convex.getPoint(i));
        }
    }

    return points;
}

vector<pair<sf::Vertex, sf::Vertex>> get_all_segments(sf::Vertex reference, vector<sf::Vertex> points) {
    // returns segments from reference to every point in points plus the segments +- a small angle
    vector<pair<sf::Vertex, sf::Vertex>> segments;
    Vector ref_vector = Vector(reference);

    for(sf::Vertex point : points) {
        segments.push_back(make_pair(reference, point));

        Vector point_vector = Vector(point.position.x-ref_vector.x, point.position.y-ref_vector.y);
        point_vector.force_module(10000);

        // jittered segments
        Vector plus_vector = point_vector;
        plus_vector.rotate_ccw(0.001);
        plus_vector = plus_vector + ref_vector;

        Vector minus_vector = point_vector;
        minus_vector.rotate_ccw(-0.001);
        minus_vector = minus_vector + ref_vector;


        pair<sf::Vertex, sf::Vertex> plus_segment = make_pair(reference, sf::Vertex(sf::Vector2f(plus_vector.x, plus_vector.y)));
        segments.push_back(plus_segment);
        pair<sf::Vertex, sf::Vertex> minus_segment = make_pair(reference, sf::Vertex(sf::Vector2f(minus_vector.x, minus_vector.y)));
        segments.push_back(minus_segment);
    }

    return segments;
}

int main() {
    sf::ContextSettings settings;
    settings.antialiasingLevel = 8;
    sf::RenderWindow window(sf::VideoMode(800, 600), "Window", sf::Style::Default, settings);


    // shapes for test
    sf::ConvexShape convex, window_convex, convex2, convex3;
    convex.setPointCount(5);
    convex.setPoint(0, sf::Vector2f(300.f, 300.f));
    convex.setPoint(1, sf::Vector2f(450.f, 310.f));
    convex.setPoint(2, sf::Vector2f(420.f, 390.f));
    convex.setPoint(3, sf::Vector2f(330.f, 400.f));
    convex.setPoint(4, sf::Vector2f(300.f, 350.f));
    convex.setFillColor(sf::Color::Blue);

    convex2.setPointCount(3);
    convex2.setPoint(0, sf::Vector2f(50.f, 50.f));
    convex2.setPoint(1, sf::Vector2f(50.f, 200.f));
    convex2.setPoint(2, sf::Vector2f(180.f, 170.f));
    convex2.setFillColor(sf::Color::Blue);

    convex3.setPointCount(3);
    convex3.setPoint(0, sf::Vector2f(700.f, 300.f));
    convex3.setPoint(1, sf::Vector2f(700.f, 470.f));
    convex3.setPoint(2, sf::Vector2f(711.f, 470.f));
    convex3.setFillColor(sf::Color::Blue);

    // window
    window_convex.setPointCount(4);
    window_convex.setPoint(0, sf::Vector2f(0.f, 0.f));
    window_convex.setPoint(1, sf::Vector2f(0.f, 600.f));
    window_convex.setPoint(2, sf::Vector2f(800.f, 600.f));
    window_convex.setPoint(3, sf::Vector2f(800.f, 0.f));

    vector<sf::ConvexShape> convexes;
    convexes.push_back(convex);
    convexes.push_back(convex2);
    convexes.push_back(convex3);
    convexes.push_back(window_convex);

    int convexes_size = convexes.size();

    // run the program as long as the window is open
    while(window.isOpen()) {
        // check all the window's events that were triggered since the last iteration of the loop
        sf::Event event;
        while(window.pollEvent(event)) {
            // "close requested" event: we close the window
            if(event.type == sf::Event::Closed)
                window.close();
        }
        window.clear(sf::Color::Black);

        sf::Vector2i mouse_position = sf::Mouse::getPosition(window);
        sf::Vertex mouse_vertex = sf::Vertex(sf::Vector2f(mouse_position.x, mouse_position.y));
        sf::VertexArray area(sf::TrianglesFan); // visibility area
        area.append(mouse_vertex); // shape center

        // get points from convex shapes
        vector<sf::Vertex> points = get_points(convexes);

        // segments from mouse to every point in points
        vector<pair<sf::Vertex, sf::Vertex>> segments = get_all_segments(mouse_vertex, points);

        // calculate intersection points
        vector<sf::Vertex> intersection_points;
        for(pair<sf::Vertex, sf::Vertex> ray : segments) {
            sf::Vertex inter, inter_aux;
            bool has_intersection = false;

            for(sf::ConvexShape convex : convexes) {
                if(convex_line_intersection(convex, ray, inter_aux)) {
                    if(!has_intersection or (distance(ray.first, inter_aux) < distance(ray.first, inter)) ) {
                        swap(inter_aux, inter);
                    }
                    has_intersection = true;
                }
            }

            if(has_intersection) {
                intersection_points.push_back(inter);
            }
        }
        sort_points(mouse_vertex, intersection_points);

        // add points to visibility area
        for(sf::Vertex point : intersection_points) {
            // uncomment to see lines
            // sf::Vertex vertices[] =
            // {
            //     mouse_vertex,
            //     point
            // };
            // window.draw(vertices, 2, sf::Lines);
            // cout << "point = " << point.position.x << " " << point.position.y << endl;
            area.append(point);
        }
        area.append(intersection_points[0]);

        // print stuff
        for(int i = 0; i < convexes_size-1; i++) {
            auto convex = convexes[i];
            window.draw(convex);
        }
        window.draw(area);
        window.display();
    }

    return 0;
}
