
/*
 * MULTI-CHANNEL SIGNED DISTANCE FIELD GENERATOR
 * ---------------------------------------------
 * https://github.com/Chlumsky/msdfgen
 *
 * MIT License
 * 
 * Copyright (c) 2014 - 2024 Viktor Chlumsky
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 */

#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "msdfgen.h"

#include <queue>

#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshadow"
#elif defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable : 4456 4457 4458 6246)
#endif

#ifndef M_PI
#define M_PI 3.1415926535897932384626433832795
#endif

namespace msdfgen {

int solveQuadratic(double x[2], double a, double b, double c) {
    // a == 0 -> linear equation
    if (a == 0 || fabs(b) > 1e12*fabs(a)) {
        // a == 0, b == 0 -> no solution
        if (b == 0) {
            if (c == 0)
                return -1; // 0 == 0
            return 0;
        }
        x[0] = -c/b;
        return 1;
    }
    double dscr = b*b-4*a*c;
    if (dscr > 0) {
        dscr = sqrt(dscr);
        x[0] = (-b+dscr)/(2*a);
        x[1] = (-b-dscr)/(2*a);
        return 2;
    } else if (dscr == 0) {
        x[0] = -b/(2*a);
        return 1;
    } else
        return 0;
}

static int solveCubicNormed(double x[3], double a, double b, double c) {
    double a2 = a*a;
    double q = 1/9.*(a2-3*b);
    double r = 1/54.*(a*(2*a2-9*b)+27*c);
    double r2 = r*r;
    double q3 = q*q*q;
    a *= 1/3.;
    if (r2 < q3) {
        double t = r/sqrt(q3);
        if (t < -1) t = -1;
        if (t > 1) t = 1;
        t = acos(t);
        q = -2*sqrt(q);
        x[0] = q*cos(1/3.*t)-a;
        x[1] = q*cos(1/3.*(t+2*M_PI))-a;
        x[2] = q*cos(1/3.*(t-2*M_PI))-a;
        return 3;
    } else {
        double u = (r < 0 ? 1 : -1)*MSDFGEN_CUBE_ROOT(fabs(r)+sqrt(r2-q3));
        double v = u == 0 ? 0 : q/u;
        x[0] = (u+v)-a;
        if (u == v || fabs(u-v) < 1e-12*fabs(u+v)) {
            x[1] = -.5*(u+v)-a;
            return 2;
        }
        return 1;
    }
}

int solveCubic(double x[3], double a, double b, double c, double d) {
    if (a != 0) {
        double bn = b/a;
        if (fabs(bn) < 1e6) // Above this ratio, the numerical error gets larger than if we treated a as zero
            return solveCubicNormed(x, bn, c/a, d/a);
    }
    return solveQuadratic(x, b, c, d);
}

Projection::Projection() : scale(1), translate(0) { }

Projection::Projection(const Vector2 &scale, const Vector2 &translate) : scale(scale), translate(translate) { }

Point2 Projection::project(const Point2 &coord) const {
    return scale*(coord+translate);
}

Point2 Projection::unproject(const Point2 &coord) const {
    return coord/scale-translate;
}

Vector2 Projection::projectVector(const Vector2 &vector) const {
    return scale*vector;
}

Vector2 Projection::unprojectVector(const Vector2 &vector) const {
    return vector/scale;
}

double Projection::projectX(double x) const {
    return scale.x*(x+translate.x);
}

double Projection::projectY(double y) const {
    return scale.y*(y+translate.y);
}

double Projection::unprojectX(double x) const {
    return x/scale.x-translate.x;
}

double Projection::unprojectY(double y) const {
    return y/scale.y-translate.y;
}

DistanceMapping DistanceMapping::inverse(Range range) {
    double rangeWidth = range.upper-range.lower;
    return DistanceMapping(rangeWidth, range.lower/(rangeWidth ? rangeWidth : 1));
}

DistanceMapping::DistanceMapping() : scale(1), translate(0) { }

DistanceMapping::DistanceMapping(Range range) : scale(1/(range.upper-range.lower)), translate(-range.lower) { }

double DistanceMapping::operator()(double d) const {
    return scale*(d+translate);
}

double DistanceMapping::operator()(Delta d) const {
    return scale*d.value;
}

DistanceMapping DistanceMapping::inverse() const {
    return DistanceMapping(1/scale, -scale*translate);
}

static int compareIntersections(const void *a, const void *b) {
    return sign(reinterpret_cast<const Scanline::Intersection *>(a)->x-reinterpret_cast<const Scanline::Intersection *>(b)->x);
}

bool interpretFillRule(int intersections, FillRule fillRule) {
    switch (fillRule) {
        case FILL_NONZERO:
            return intersections != 0;
        case FILL_ODD:
            return intersections&1;
        case FILL_POSITIVE:
            return intersections > 0;
        case FILL_NEGATIVE:
            return intersections < 0;
    }
    return false;
}

Scanline::Scanline() : lastIndex(0) { }

void Scanline::preprocess() {
    lastIndex = 0;
    if (!intersections.empty()) {
        qsort(&intersections[0], intersections.size(), sizeof(Intersection), compareIntersections);
        int totalDirection = 0;
        for (std::vector<Intersection>::iterator intersection = intersections.begin(); intersection != intersections.end(); ++intersection) {
            totalDirection += intersection->direction;
            intersection->direction = totalDirection;
        }
    }
}

void Scanline::setIntersections(const std::vector<Intersection> &intersections) {
    this->intersections = intersections;
    preprocess();
}

int Scanline::moveTo(double x) const {
    if (intersections.empty())
        return -1;
    int index = lastIndex;
    if (x < intersections[index].x) {
        do {
            if (index == 0) {
                lastIndex = 0;
                return -1;
            }
            --index;
        } while (x < intersections[index].x);
    } else {
        while (index < (int) intersections.size()-1 && x >= intersections[index+1].x)
            ++index;
    }
    lastIndex = index;
    return index;
}

int Scanline::countIntersections(double x) const {
    return moveTo(x)+1;
}

int Scanline::sumIntersections(double x) const {
    int index = moveTo(x);
    if (index >= 0)
        return intersections[index].direction;
    return 0;
}

bool Scanline::filled(double x, FillRule fillRule) const {
    return interpretFillRule(sumIntersections(x), fillRule);
}

}

namespace msdfgen {

EdgeSegment *EdgeSegment::create(Point2 p0, Point2 p1, EdgeColor edgeColor) {
    return new LinearSegment(p0, p1, edgeColor);
}

EdgeSegment *EdgeSegment::create(Point2 p0, Point2 p1, Point2 p2, EdgeColor edgeColor) {
    if (!crossProduct(p1-p0, p2-p1))
        return new LinearSegment(p0, p2, edgeColor);
    return new QuadraticSegment(p0, p1, p2, edgeColor);
}

EdgeSegment *EdgeSegment::create(Point2 p0, Point2 p1, Point2 p2, Point2 p3, EdgeColor edgeColor) {
    Vector2 p12 = p2-p1;
    if (!crossProduct(p1-p0, p12) && !crossProduct(p12, p3-p2))
        return new LinearSegment(p0, p3, edgeColor);
    if ((p12 = 1.5*p1-.5*p0) == 1.5*p2-.5*p3)
        return new QuadraticSegment(p0, p12, p3, edgeColor);
    return new CubicSegment(p0, p1, p2, p3, edgeColor);
}

void EdgeSegment::distanceToPerpendicularDistance(SignedDistance &distance, Point2 origin, double param) const {
    if (param < 0) {
        Vector2 dir = direction(0).normalize();
        Vector2 aq = origin-point(0);
        double ts = dotProduct(aq, dir);
        if (ts < 0) {
            double perpendicularDistance = crossProduct(aq, dir);
            if (fabs(perpendicularDistance) <= fabs(distance.distance)) {
                distance.distance = perpendicularDistance;
                distance.dot = 0;
            }
        }
    } else if (param > 1) {
        Vector2 dir = direction(1).normalize();
        Vector2 bq = origin-point(1);
        double ts = dotProduct(bq, dir);
        if (ts > 0) {
            double perpendicularDistance = crossProduct(bq, dir);
            if (fabs(perpendicularDistance) <= fabs(distance.distance)) {
                distance.distance = perpendicularDistance;
                distance.dot = 0;
            }
        }
    }
}

LinearSegment::LinearSegment(Point2 p0, Point2 p1, EdgeColor edgeColor) : EdgeSegment(edgeColor) {
    p[0] = p0;
    p[1] = p1;
}

QuadraticSegment::QuadraticSegment(Point2 p0, Point2 p1, Point2 p2, EdgeColor edgeColor) : EdgeSegment(edgeColor) {
    p[0] = p0;
    p[1] = p1;
    p[2] = p2;
}

CubicSegment::CubicSegment(Point2 p0, Point2 p1, Point2 p2, Point2 p3, EdgeColor edgeColor) : EdgeSegment(edgeColor) {
    p[0] = p0;
    p[1] = p1;
    p[2] = p2;
    p[3] = p3;
}

LinearSegment *LinearSegment::clone() const {
    return new LinearSegment(p[0], p[1], color);
}

QuadraticSegment *QuadraticSegment::clone() const {
    return new QuadraticSegment(p[0], p[1], p[2], color);
}

CubicSegment *CubicSegment::clone() const {
    return new CubicSegment(p[0], p[1], p[2], p[3], color);
}

int LinearSegment::type() const {
    return (int) EDGE_TYPE;
}

int QuadraticSegment::type() const {
    return (int) EDGE_TYPE;
}

int CubicSegment::type() const {
    return (int) EDGE_TYPE;
}

const Point2 *LinearSegment::controlPoints() const {
    return p;
}

const Point2 *QuadraticSegment::controlPoints() const {
    return p;
}

const Point2 *CubicSegment::controlPoints() const {
    return p;
}

Point2 LinearSegment::point(double param) const {
    return mix(p[0], p[1], param);
}

Point2 QuadraticSegment::point(double param) const {
    return mix(mix(p[0], p[1], param), mix(p[1], p[2], param), param);
}

Point2 CubicSegment::point(double param) const {
    Vector2 p12 = mix(p[1], p[2], param);
    return mix(mix(mix(p[0], p[1], param), p12, param), mix(p12, mix(p[2], p[3], param), param), param);
}

Vector2 LinearSegment::direction(double param) const {
    return p[1]-p[0];
}

Vector2 QuadraticSegment::direction(double param) const {
    Vector2 tangent = mix(p[1]-p[0], p[2]-p[1], param);
    if (!tangent)
        return p[2]-p[0];
    return tangent;
}

Vector2 CubicSegment::direction(double param) const {
    Vector2 tangent = mix(mix(p[1]-p[0], p[2]-p[1], param), mix(p[2]-p[1], p[3]-p[2], param), param);
    if (!tangent) {
        if (param == 0) return p[2]-p[0];
        if (param == 1) return p[3]-p[1];
    }
    return tangent;
}

Vector2 LinearSegment::directionChange(double param) const {
    return Vector2();
}

Vector2 QuadraticSegment::directionChange(double param) const {
    return (p[2]-p[1])-(p[1]-p[0]);
}

Vector2 CubicSegment::directionChange(double param) const {
    return mix((p[2]-p[1])-(p[1]-p[0]), (p[3]-p[2])-(p[2]-p[1]), param);
}

double LinearSegment::length() const {
    return (p[1]-p[0]).length();
}

double QuadraticSegment::length() const {
    Vector2 ab = p[1]-p[0];
    Vector2 br = p[2]-p[1]-ab;
    double abab = dotProduct(ab, ab);
    double abbr = dotProduct(ab, br);
    double brbr = dotProduct(br, br);
    double abLen = sqrt(abab);
    double brLen = sqrt(brbr);
    double crs = crossProduct(ab, br);
    double h = sqrt(abab+abbr+abbr+brbr);
    return (
        brLen*((abbr+brbr)*h-abbr*abLen)+
        crs*crs*log((brLen*h+abbr+brbr)/(brLen*abLen+abbr))
    )/(brbr*brLen);
}

SignedDistance LinearSegment::signedDistance(Point2 origin, double &param) const {
    Vector2 aq = origin-p[0];
    Vector2 ab = p[1]-p[0];
    param = dotProduct(aq, ab)/dotProduct(ab, ab);
    Vector2 eq = p[param > .5]-origin;
    double endpointDistance = eq.length();
    if (param > 0 && param < 1) {
        double orthoDistance = dotProduct(ab.getOrthonormal(false), aq);
        if (fabs(orthoDistance) < endpointDistance)
            return SignedDistance(orthoDistance, 0);
    }
    return SignedDistance(nonZeroSign(crossProduct(aq, ab))*endpointDistance, fabs(dotProduct(ab.normalize(), eq.normalize())));
}

SignedDistance QuadraticSegment::signedDistance(Point2 origin, double &param) const {
    Vector2 ap = origin-p[0];
    Vector2 bp = origin-p[2];
    Vector2 q = 2*(p[1]-p[0]);
    Vector2 r = p[2]-2*p[1]+p[0];
    double aSqD = ap.squaredLength();
    double bSqD = bp.squaredLength();
    double t = quadraticNearPoint(ap, q, r);
    if (t > 0 && t < 1) {
        Vector2 tp = ap-(q+r*t)*t;
        double tSqD = tp.squaredLength();
        if (tSqD < aSqD && tSqD < bSqD) {
            param = t;
            return SignedDistance(nonZeroSign(crossProduct(tp, q+2*r*t))*sqrt(tSqD), 0);
        }
    }
    if (bSqD < aSqD) {
        Vector2 d = q+r+r;
        if (!d)
            d = p[2]-p[0];
        param = dotProduct(bp, d)/d.squaredLength()+1;
        return SignedDistance(nonZeroSign(crossProduct(bp, d))*sqrt(bSqD), dotProduct(bp.normalize(), d.normalize()));
    }
    if (!q)
        q = p[2]-p[0];
    param = dotProduct(ap, q)/q.squaredLength();
    return SignedDistance(nonZeroSign(crossProduct(ap, q))*sqrt(aSqD), -dotProduct(ap.normalize(), q.normalize()));
}

SignedDistance CubicSegment::signedDistance(Point2 origin, double &param) const {
    Vector2 ap = origin-p[0];
    Vector2 bp = origin-p[3];
    Vector2 q = 3*(p[1]-p[0]);
    Vector2 r = 3*(p[2]-p[1])-q;
    Vector2 s = p[3]-3*(p[2]-p[1])-p[0];
    double aSqD = ap.squaredLength();
    double bSqD = bp.squaredLength();
    double tSqD;
    double t = cubicNearPoint(ap, q, r, s, tSqD);
    if (t > 0 && t < 1) {
        if (tSqD < aSqD && tSqD < bSqD) {
            param = t;
            return SignedDistance(nonZeroSign(crossProduct(ap-(q+(r+s*t)*t)*t, q+(r+r+3*s*t)*t))*sqrt(tSqD), 0);
        }
    }
    if (bSqD < aSqD) {
        Vector2 d = q+r+r+3*s;
        if (!d)
            d = p[3]-p[1];
        param = dotProduct(bp, d)/d.squaredLength()+1;
        return SignedDistance(nonZeroSign(crossProduct(bp, d))*sqrt(bSqD), dotProduct(bp.normalize(), d.normalize()));
    }
    if (!q)
        q = p[2]-p[0];
    param = dotProduct(ap, q)/q.squaredLength();
    return SignedDistance(nonZeroSign(crossProduct(ap, q))*sqrt(aSqD), -dotProduct(ap.normalize(), q.normalize()));
}


int LinearSegment::scanlineIntersections(double x[3], int dy[3], double y) const {
    return horizontalScanlineIntersections(x, dy, y);
}

int QuadraticSegment::scanlineIntersections(double x[3], int dy[3], double y) const {
    return horizontalScanlineIntersections(x, dy, y);
}

int CubicSegment::scanlineIntersections(double x[3], int dy[3], double y) const {
    return horizontalScanlineIntersections(x, dy, y);
}

int LinearSegment::horizontalScanlineIntersections(double x[3], int dy[3], double y) const {
    if ((y >= p[0].y && y < p[1].y) || (y >= p[1].y && y < p[0].y)) {
        double param = (y-p[0].y)/(p[1].y-p[0].y);
        x[0] = mix(p[0].x, p[1].x, param);
        dy[0] = sign(p[1].y-p[0].y);
        return 1;
    }
    return 0;
}

int QuadraticSegment::horizontalScanlineIntersections(double x[3], int dy[3], double y) const {
    int total = 0;
    int nextDY = y > p[0].y ? 1 : -1;
    x[total] = p[0].x;
    if (p[0].y == y) {
        if (p[0].y < p[1].y || (p[0].y == p[1].y && p[0].y < p[2].y))
            dy[total++] = 1;
        else
            nextDY = 1;
    }
    {
        Vector2 ab = p[1]-p[0];
        Vector2 br = p[2]-p[1]-ab;
        double t[2];
        int solutions = solveQuadratic(t, br.y, 2*ab.y, p[0].y-y);
        // Sort solutions
        double tmp;
        if (solutions >= 2 && t[0] > t[1])
            tmp = t[0], t[0] = t[1], t[1] = tmp;
        for (int i = 0; i < solutions && total < 2; ++i) {
            if (t[i] >= 0 && t[i] <= 1) {
                x[total] = p[0].x+2*t[i]*ab.x+t[i]*t[i]*br.x;
                if (nextDY*(ab.y+t[i]*br.y) >= 0) {
                    dy[total++] = nextDY;
                    nextDY = -nextDY;
                }
            }
        }
    }
    if (p[2].y == y) {
        if (nextDY > 0 && total > 0) {
            --total;
            nextDY = -1;
        }
        if ((p[2].y < p[1].y || (p[2].y == p[1].y && p[2].y < p[0].y)) && total < 2) {
            x[total] = p[2].x;
            if (nextDY < 0) {
                dy[total++] = -1;
                nextDY = 1;
            }
        }
    }
    if (nextDY != (y >= p[2].y ? 1 : -1)) {
        if (total > 0)
            --total;
        else {
            if (fabs(p[2].y-y) < fabs(p[0].y-y))
                x[total] = p[2].x;
            dy[total++] = nextDY;
        }
    }
    return total;
}

int CubicSegment::horizontalScanlineIntersections(double x[3], int dy[3], double y) const {
    int total = 0;
    int nextDY = y > p[0].y ? 1 : -1;
    x[total] = p[0].x;
    if (p[0].y == y) {
        if (p[0].y < p[1].y || (p[0].y == p[1].y && (p[0].y < p[2].y || (p[0].y == p[2].y && p[0].y < p[3].y))))
            dy[total++] = 1;
        else
            nextDY = 1;
    }
    {
        Vector2 ab = p[1]-p[0];
        Vector2 br = p[2]-p[1]-ab;
        Vector2 as = (p[3]-p[2])-(p[2]-p[1])-br;
        double t[3];
        int solutions = solveCubic(t, as.y, 3*br.y, 3*ab.y, p[0].y-y);
        // Sort solutions
        double tmp;
        if (solutions >= 2) {
            if (t[0] > t[1])
                tmp = t[0], t[0] = t[1], t[1] = tmp;
            if (solutions >= 3 && t[1] > t[2]) {
                tmp = t[1], t[1] = t[2], t[2] = tmp;
                if (t[0] > t[1])
                    tmp = t[0], t[0] = t[1], t[1] = tmp;
            }
        }
        for (int i = 0; i < solutions && total < 3; ++i) {
            if (t[i] >= 0 && t[i] <= 1) {
                x[total] = p[0].x+3*t[i]*ab.x+3*t[i]*t[i]*br.x+t[i]*t[i]*t[i]*as.x;
                if (nextDY*(ab.y+2*t[i]*br.y+t[i]*t[i]*as.y) >= 0) {
                    dy[total++] = nextDY;
                    nextDY = -nextDY;
                }
            }
        }
    }
    if (p[3].y == y) {
        if (nextDY > 0 && total > 0) {
            --total;
            nextDY = -1;
        }
        if ((p[3].y < p[2].y || (p[3].y == p[2].y && (p[3].y < p[1].y || (p[3].y == p[1].y && p[3].y < p[0].y)))) && total < 3) {
            x[total] = p[3].x;
            if (nextDY < 0) {
                dy[total++] = -1;
                nextDY = 1;
            }
        }
    }
    if (nextDY != (y >= p[3].y ? 1 : -1)) {
        if (total > 0)
            --total;
        else {
            if (fabs(p[3].y-y) < fabs(p[0].y-y))
                x[total] = p[3].x;
            dy[total++] = nextDY;
        }
    }
    return total;
}

static void pointBounds(Point2 p, double &l, double &b, double &r, double &t) {
    if (p.x < l) l = p.x;
    if (p.y < b) b = p.y;
    if (p.x > r) r = p.x;
    if (p.y > t) t = p.y;
}

void LinearSegment::bound(double &l, double &b, double &r, double &t) const {
    pointBounds(p[0], l, b, r, t);
    pointBounds(p[1], l, b, r, t);
}

void QuadraticSegment::bound(double &l, double &b, double &r, double &t) const {
    pointBounds(p[0], l, b, r, t);
    pointBounds(p[2], l, b, r, t);
    Vector2 bot = (p[1]-p[0])-(p[2]-p[1]);
    if (bot.x) {
        double param = (p[1].x-p[0].x)/bot.x;
        if (param > 0 && param < 1)
            pointBounds(point(param), l, b, r, t);
    }
    if (bot.y) {
        double param = (p[1].y-p[0].y)/bot.y;
        if (param > 0 && param < 1)
            pointBounds(point(param), l, b, r, t);
    }
}

void CubicSegment::bound(double &l, double &b, double &r, double &t) const {
    pointBounds(p[0], l, b, r, t);
    pointBounds(p[3], l, b, r, t);
    Vector2 a0 = p[1]-p[0];
    Vector2 a1 = 2*(p[2]-p[1]-a0);
    Vector2 a2 = p[3]-3*p[2]+3*p[1]-p[0];
    double params[2];
    int solutions;
    solutions = solveQuadratic(params, a2.x, a1.x, a0.x);
    for (int i = 0; i < solutions; ++i)
        if (params[i] > 0 && params[i] < 1)
            pointBounds(point(params[i]), l, b, r, t);
    solutions = solveQuadratic(params, a2.y, a1.y, a0.y);
    for (int i = 0; i < solutions; ++i)
        if (params[i] > 0 && params[i] < 1)
            pointBounds(point(params[i]), l, b, r, t);
}

void LinearSegment::reverse() {
    Point2 tmp = p[0];
    p[0] = p[1];
    p[1] = tmp;
}

void QuadraticSegment::reverse() {
    Point2 tmp = p[0];
    p[0] = p[2];
    p[2] = tmp;
}

void CubicSegment::reverse() {
    Point2 tmp = p[0];
    p[0] = p[3];
    p[3] = tmp;
    tmp = p[1];
    p[1] = p[2];
    p[2] = tmp;
}

void LinearSegment::moveStartPoint(Point2 to) {
    p[0] = to;
}

void QuadraticSegment::moveStartPoint(Point2 to) {
    Vector2 origSDir = p[0]-p[1];
    Point2 origP1 = p[1];
    p[1] += crossProduct(p[0]-p[1], to-p[0])/crossProduct(p[0]-p[1], p[2]-p[1])*(p[2]-p[1]);
    p[0] = to;
    if (dotProduct(origSDir, p[0]-p[1]) < 0)
        p[1] = origP1;
}

void CubicSegment::moveStartPoint(Point2 to) {
    p[1] += to-p[0];
    p[0] = to;
}

void LinearSegment::moveEndPoint(Point2 to) {
    p[1] = to;
}

void QuadraticSegment::moveEndPoint(Point2 to) {
    Vector2 origEDir = p[2]-p[1];
    Point2 origP1 = p[1];
    p[1] += crossProduct(p[2]-p[1], to-p[2])/crossProduct(p[2]-p[1], p[0]-p[1])*(p[0]-p[1]);
    p[2] = to;
    if (dotProduct(origEDir, p[2]-p[1]) < 0)
        p[1] = origP1;
}

void CubicSegment::moveEndPoint(Point2 to) {
    p[2] += to-p[3];
    p[3] = to;
}

void LinearSegment::splitInThirds(EdgeSegment *&part0, EdgeSegment *&part1, EdgeSegment *&part2) const {
    part0 = new LinearSegment(p[0], point(1/3.), color);
    part1 = new LinearSegment(point(1/3.), point(2/3.), color);
    part2 = new LinearSegment(point(2/3.), p[1], color);
}

void QuadraticSegment::splitInThirds(EdgeSegment *&part0, EdgeSegment *&part1, EdgeSegment *&part2) const {
    part0 = new QuadraticSegment(p[0], mix(p[0], p[1], 1/3.), point(1/3.), color);
    part1 = new QuadraticSegment(point(1/3.), mix(mix(p[0], p[1], 5/9.), mix(p[1], p[2], 4/9.), .5), point(2/3.), color);
    part2 = new QuadraticSegment(point(2/3.), mix(p[1], p[2], 2/3.), p[2], color);
}

void CubicSegment::splitInThirds(EdgeSegment *&part0, EdgeSegment *&part1, EdgeSegment *&part2) const {
    part0 = new CubicSegment(p[0], p[0] == p[1] ? p[0] : mix(p[0], p[1], 1/3.), mix(mix(p[0], p[1], 1/3.), mix(p[1], p[2], 1/3.), 1/3.), point(1/3.), color);
    part1 = new CubicSegment(point(1/3.),
        mix(mix(mix(p[0], p[1], 1/3.), mix(p[1], p[2], 1/3.), 1/3.), mix(mix(p[1], p[2], 1/3.), mix(p[2], p[3], 1/3.), 1/3.), 2/3.),
        mix(mix(mix(p[0], p[1], 2/3.), mix(p[1], p[2], 2/3.), 2/3.), mix(mix(p[1], p[2], 2/3.), mix(p[2], p[3], 2/3.), 2/3.), 1/3.),
        point(2/3.), color);
    part2 = new CubicSegment(point(2/3.), mix(mix(p[1], p[2], 2/3.), mix(p[2], p[3], 2/3.), 2/3.), p[2] == p[3] ? p[3] : mix(p[2], p[3], 2/3.), p[3], color);
}

EdgeSegment *QuadraticSegment::convertToCubic() const {
    return new CubicSegment(p[0], mix(p[0], p[1], 2/3.), mix(p[1], p[2], 1/3.), p[2], color);
}

void EdgeHolder::swap(EdgeHolder &a, EdgeHolder &b) {
    EdgeSegment *tmp = a.edgeSegment;
    a.edgeSegment = b.edgeSegment;
    b.edgeSegment = tmp;
}

EdgeHolder::EdgeHolder(const EdgeHolder &orig) : edgeSegment(orig.edgeSegment ? orig.edgeSegment->clone() : NULL) { }

EdgeHolder::~EdgeHolder() {
    delete edgeSegment;
}

EdgeHolder &EdgeHolder::operator=(const EdgeHolder &orig) {
    if (this != &orig) {
        delete edgeSegment;
        edgeSegment = orig.edgeSegment ? orig.edgeSegment->clone() : NULL;
    }
    return *this;
}

EdgeSegment &EdgeHolder::operator*() {
    return *edgeSegment;
}

const EdgeSegment &EdgeHolder::operator*() const {
    return *edgeSegment;
}

EdgeSegment *EdgeHolder::operator->() {
    return edgeSegment;
}

const EdgeSegment *EdgeHolder::operator->() const {
    return edgeSegment;
}

EdgeHolder::operator EdgeSegment *() {
    return edgeSegment;
}

EdgeHolder::operator const EdgeSegment *() const {
    return edgeSegment;
}

static double shoelace(const Point2 &a, const Point2 &b) {
    return (b.x-a.x)*(a.y+b.y);
}

void Contour::addEdge(const EdgeHolder &edge) {
    edges.push_back(edge);
}

EdgeHolder &Contour::addEdge() {
    edges.resize(edges.size()+1);
    return edges.back();
}

static void boundPoint(double &l, double &b, double &r, double &t, Point2 p) {
    if (p.x < l) l = p.x;
    if (p.y < b) b = p.y;
    if (p.x > r) r = p.x;
    if (p.y > t) t = p.y;
}

void Contour::bound(double &l, double &b, double &r, double &t) const {
    for (std::vector<EdgeHolder>::const_iterator edge = edges.begin(); edge != edges.end(); ++edge)
        (*edge)->bound(l, b, r, t);
}

void Contour::boundMiters(double &l, double &b, double &r, double &t, double border, double miterLimit, int polarity) const {
    if (edges.empty())
        return;
    Vector2 prevDir = edges.back()->direction(1).normalize(true);
    for (std::vector<EdgeHolder>::const_iterator edge = edges.begin(); edge != edges.end(); ++edge) {
        Vector2 dir = -(*edge)->direction(0).normalize(true);
        if (polarity*crossProduct(prevDir, dir) >= 0) {
            double miterLength = miterLimit;
            double q = .5*(1-dotProduct(prevDir, dir));
            if (q > 0)
                miterLength = min(1/sqrt(q), miterLimit);
            Point2 miter = (*edge)->point(0)+border*miterLength*(prevDir+dir).normalize(true);
            boundPoint(l, b, r, t, miter);
        }
        prevDir = (*edge)->direction(1).normalize(true);
    }
}

int Contour::winding() const {
    if (edges.empty())
        return 0;
    double total = 0;
    if (edges.size() == 1) {
        Point2 a = edges[0]->point(0), b = edges[0]->point(1/3.), c = edges[0]->point(2/3.);
        total += shoelace(a, b);
        total += shoelace(b, c);
        total += shoelace(c, a);
    } else if (edges.size() == 2) {
        Point2 a = edges[0]->point(0), b = edges[0]->point(.5), c = edges[1]->point(0), d = edges[1]->point(.5);
        total += shoelace(a, b);
        total += shoelace(b, c);
        total += shoelace(c, d);
        total += shoelace(d, a);
    } else {
        Point2 prev = edges.back()->point(0);
        for (std::vector<EdgeHolder>::const_iterator edge = edges.begin(); edge != edges.end(); ++edge) {
            Point2 cur = (*edge)->point(0);
            total += shoelace(prev, cur);
            prev = cur;
        }
    }
    return sign(total);
}

void Contour::reverse() {
    for (int i = (int) edges.size()/2; i > 0; --i)
        EdgeHolder::swap(edges[i-1], edges[edges.size()-i]);
    for (std::vector<EdgeHolder>::iterator edge = edges.begin(); edge != edges.end(); ++edge)
        (*edge)->reverse();
}

}

#define DECONVERGE_OVERSHOOT 1.11111111111111111 // moves control points slightly more than necessary to account for floating-point errors

namespace msdfgen {

Shape::Shape() : inverseYAxis(false) { }

void Shape::addContour(const Contour &contour) {
    contours.push_back(contour);
}

Contour &Shape::addContour() {
    contours.resize(contours.size()+1);
    return contours.back();
}

bool Shape::validate() const {
    for (std::vector<Contour>::const_iterator contour = contours.begin(); contour != contours.end(); ++contour) {
        if (!contour->edges.empty()) {
            Point2 corner = contour->edges.back()->point(1);
            for (std::vector<EdgeHolder>::const_iterator edge = contour->edges.begin(); edge != contour->edges.end(); ++edge) {
                if (!*edge)
                    return false;
                if ((*edge)->point(0) != corner)
                    return false;
                corner = (*edge)->point(1);
            }
        }
    }
    return true;
}

static void deconvergeEdge(EdgeHolder &edgeHolder, int param, Vector2 vector) {
    switch (edgeHolder->type()) {
        case (int) QuadraticSegment::EDGE_TYPE:
            edgeHolder = static_cast<const QuadraticSegment *>(&*edgeHolder)->convertToCubic();
            // fallthrough
        case (int) CubicSegment::EDGE_TYPE:
            {
                Point2 *p = static_cast<CubicSegment *>(&*edgeHolder)->p;
                switch (param) {
                    case 0:
                        p[1] += (p[1]-p[0]).length()*vector;
                        break;
                    case 1:
                        p[2] += (p[2]-p[3]).length()*vector;
                        break;
                }
            }
    }
}

void Shape::normalize() {
    for (std::vector<Contour>::iterator contour = contours.begin(); contour != contours.end(); ++contour) {
        if (contour->edges.size() == 1) {
            EdgeSegment *parts[3] = { };
            contour->edges[0]->splitInThirds(parts[0], parts[1], parts[2]);
            contour->edges.clear();
            contour->edges.push_back(EdgeHolder(parts[0]));
            contour->edges.push_back(EdgeHolder(parts[1]));
            contour->edges.push_back(EdgeHolder(parts[2]));
        } else {
            // Push apart convergent edge segments
            EdgeHolder *prevEdge = &contour->edges.back();
            for (std::vector<EdgeHolder>::iterator edge = contour->edges.begin(); edge != contour->edges.end(); ++edge) {
                Vector2 prevDir = (*prevEdge)->direction(1).normalize();
                Vector2 curDir = (*edge)->direction(0).normalize();
                if (dotProduct(prevDir, curDir) < MSDFGEN_CORNER_DOT_EPSILON-1) {
                    double factor = DECONVERGE_OVERSHOOT*sqrt(1-(MSDFGEN_CORNER_DOT_EPSILON-1)*(MSDFGEN_CORNER_DOT_EPSILON-1))/(MSDFGEN_CORNER_DOT_EPSILON-1);
                    Vector2 axis = factor*(curDir-prevDir).normalize();
                    // Determine curve ordering using third-order derivative (t = 0) of crossProduct((*prevEdge)->point(1-t)-p0, (*edge)->point(t)-p0) where p0 is the corner (*edge)->point(0)
                    if (crossProduct((*prevEdge)->directionChange(1), (*edge)->direction(0))+crossProduct((*edge)->directionChange(0), (*prevEdge)->direction(1)) < 0)
                        axis = -axis;
                    deconvergeEdge(*prevEdge, 1, axis.getOrthogonal(true));
                    deconvergeEdge(*edge, 0, axis.getOrthogonal(false));
                }
                prevEdge = &*edge;
            }
        }
    }
}

void Shape::bound(double &l, double &b, double &r, double &t) const {
    for (std::vector<Contour>::const_iterator contour = contours.begin(); contour != contours.end(); ++contour)
        contour->bound(l, b, r, t);
}

void Shape::boundMiters(double &l, double &b, double &r, double &t, double border, double miterLimit, int polarity) const {
    for (std::vector<Contour>::const_iterator contour = contours.begin(); contour != contours.end(); ++contour)
        contour->boundMiters(l, b, r, t, border, miterLimit, polarity);
}

Shape::Bounds Shape::getBounds(double border, double miterLimit, int polarity) const {
    static const double LARGE_VALUE = 1e240;
    Shape::Bounds bounds = { +LARGE_VALUE, +LARGE_VALUE, -LARGE_VALUE, -LARGE_VALUE };
    bound(bounds.l, bounds.b, bounds.r, bounds.t);
    if (border > 0) {
        bounds.l -= border, bounds.b -= border;
        bounds.r += border, bounds.t += border;
        if (miterLimit > 0)
            boundMiters(bounds.l, bounds.b, bounds.r, bounds.t, border, miterLimit, polarity);
    }
    return bounds;
}

void Shape::scanline(Scanline &line, double y) const {
    std::vector<Scanline::Intersection> intersections;
    double x[3];
    int dy[3];
    for (std::vector<Contour>::const_iterator contour = contours.begin(); contour != contours.end(); ++contour) {
        for (std::vector<EdgeHolder>::const_iterator edge = contour->edges.begin(); edge != contour->edges.end(); ++edge) {
            int n = (*edge)->scanlineIntersections(x, dy, y);
            for (int i = 0; i < n; ++i) {
                Scanline::Intersection intersection = { x[i], dy[i] };
                intersections.push_back(intersection);
            }
        }
    }
    line.setIntersections(intersections);
}

int Shape::edgeCount() const {
    int total = 0;
    for (std::vector<Contour>::const_iterator contour = contours.begin(); contour != contours.end(); ++contour)
        total += (int) contour->edges.size();
    return total;
}

struct Intersection {
    double x;
    int direction;
    int contourIndex;

    static int compare(const void *a, const void *b) {
        return sign(reinterpret_cast<const Intersection *>(a)->x-reinterpret_cast<const Intersection *>(b)->x);
    }
};
void Shape::orientContours() {

    const double ratio = .5*(sqrt(5)-1); // an irrational number to minimize chance of intersecting a corner or other point of interest
    std::vector<int> orientations(contours.size());
    std::vector<Intersection> intersections;
    for (int i = 0; i < (int) contours.size(); ++i) {
        if (!orientations[i] && !contours[i].edges.empty()) {
            // Find an Y that crosses the contour
            double y0 = contours[i].edges.front()->point(0).y;
            double y1 = y0;
            for (std::vector<EdgeHolder>::const_iterator edge = contours[i].edges.begin(); edge != contours[i].edges.end() && y0 == y1; ++edge)
                y1 = (*edge)->point(1).y;
            for (std::vector<EdgeHolder>::const_iterator edge = contours[i].edges.begin(); edge != contours[i].edges.end() && y0 == y1; ++edge)
                y1 = (*edge)->point(ratio).y; // in case all endpoints are in a horizontal line
            double y = mix(y0, y1, ratio);
            // Scanline through whole shape at Y
            double x[3];
            int dy[3];
            for (int j = 0; j < (int) contours.size(); ++j) {
                for (std::vector<EdgeHolder>::const_iterator edge = contours[j].edges.begin(); edge != contours[j].edges.end(); ++edge) {
                    int n = (*edge)->scanlineIntersections(x, dy, y);
                    for (int k = 0; k < n; ++k) {
                        Intersection intersection = { x[k], dy[k], j };
                        intersections.push_back(intersection);
                    }
                }
            }
            if (!intersections.empty()) {
                qsort(&intersections[0], intersections.size(), sizeof(Intersection), &Intersection::compare);
                // Disqualify multiple intersections
                for (int j = 1; j < (int) intersections.size(); ++j)
                    if (intersections[j].x == intersections[j-1].x)
                        intersections[j].direction = intersections[j-1].direction = 0;
                // Inspect scanline and deduce orientations of intersected contours
                for (int j = 0; j < (int) intersections.size(); ++j)
                    if (intersections[j].direction)
                        orientations[intersections[j].contourIndex] += 2*((j&1)^(intersections[j].direction > 0))-1;
                intersections.clear();
            }
        }
    }
    // Reverse contours that have the opposite orientation
    for (int i = 0; i < (int) contours.size(); ++i)
        if (orientations[i] < 0)
            contours[i].reverse();
}

/**
 * For each position < n, this function will return -1, 0, or 1,
 * depending on whether the position is closer to the beginning, middle, or end, respectively.
 * It is guaranteed that the output will be balanced in that the total for positions 0 through n-1 will be zero.
 */
static int symmetricalTrichotomy(int position, int n) {
    return int(3+2.875*position/(n-1)-1.4375+.5)-3;
}

static bool isCorner(const Vector2 &aDir, const Vector2 &bDir, double crossThreshold) {
    return dotProduct(aDir, bDir) <= 0 || fabs(crossProduct(aDir, bDir)) > crossThreshold;
}

static double estimateEdgeLength(const EdgeSegment *edge) {
    double len = 0;
    Point2 prev = edge->point(0);
    for (int i = 1; i <= MSDFGEN_EDGE_LENGTH_PRECISION; ++i) {
        Point2 cur = edge->point(1./MSDFGEN_EDGE_LENGTH_PRECISION*i);
        len += (cur-prev).length();
        prev = cur;
    }
    return len;
}

static int seedExtract2(unsigned long long &seed) {
    int v = int(seed)&1;
    seed >>= 1;
    return v;
}

static int seedExtract3(unsigned long long &seed) {
    int v = int(seed%3);
    seed /= 3;
    return v;
}

static EdgeColor initColor(unsigned long long &seed) {
    static const EdgeColor colors[3] = { CYAN, MAGENTA, YELLOW };
    return colors[seedExtract3(seed)];
}

static void switchColor(EdgeColor &color, unsigned long long &seed) {
    int shifted = color<<(1+seedExtract2(seed));
    color = EdgeColor((shifted|shifted>>3)&WHITE);
}

static void switchColor(EdgeColor &color, unsigned long long &seed, EdgeColor banned) {
    EdgeColor combined = EdgeColor(color&banned);
    if (combined == RED || combined == GREEN || combined == BLUE)
        color = EdgeColor(combined^WHITE);
    else
        switchColor(color, seed);
}

void edgeColoringSimple(Shape &shape, double angleThreshold, unsigned long long seed) {
    double crossThreshold = sin(angleThreshold);
    EdgeColor color = initColor(seed);
    std::vector<int> corners;
    for (std::vector<Contour>::iterator contour = shape.contours.begin(); contour != shape.contours.end(); ++contour) {
        if (contour->edges.empty())
            continue;
        { // Identify corners
            corners.clear();
            Vector2 prevDirection = contour->edges.back()->direction(1);
            int index = 0;
            for (std::vector<EdgeHolder>::const_iterator edge = contour->edges.begin(); edge != contour->edges.end(); ++edge, ++index) {
                if (isCorner(prevDirection.normalize(), (*edge)->direction(0).normalize(), crossThreshold))
                    corners.push_back(index);
                prevDirection = (*edge)->direction(1);
            }
        }

        // Smooth contour
        if (corners.empty()) {
            switchColor(color, seed);
            for (std::vector<EdgeHolder>::iterator edge = contour->edges.begin(); edge != contour->edges.end(); ++edge)
                (*edge)->color = color;
        }
        // "Teardrop" case
        else if (corners.size() == 1) {
            EdgeColor colors[3];
            switchColor(color, seed);
            colors[0] = color;
            colors[1] = WHITE;
            switchColor(color, seed);
            colors[2] = color;
            int corner = corners[0];
            if (contour->edges.size() >= 3) {
                int m = (int) contour->edges.size();
                for (int i = 0; i < m; ++i)
                    contour->edges[(corner+i)%m]->color = colors[1+symmetricalTrichotomy(i, m)];
            } else if (contour->edges.size() >= 1) {
                // Less than three edge segments for three colors => edges must be split
                EdgeSegment *parts[7] = { };
                contour->edges[0]->splitInThirds(parts[0+3*corner], parts[1+3*corner], parts[2+3*corner]);
                if (contour->edges.size() >= 2) {
                    contour->edges[1]->splitInThirds(parts[3-3*corner], parts[4-3*corner], parts[5-3*corner]);
                    parts[0]->color = parts[1]->color = colors[0];
                    parts[2]->color = parts[3]->color = colors[1];
                    parts[4]->color = parts[5]->color = colors[2];
                } else {
                    parts[0]->color = colors[0];
                    parts[1]->color = colors[1];
                    parts[2]->color = colors[2];
                }
                contour->edges.clear();
                for (int i = 0; parts[i]; ++i)
                    contour->edges.push_back(EdgeHolder(parts[i]));
            }
        }
        // Multiple corners
        else {
            int cornerCount = (int) corners.size();
            int spline = 0;
            int start = corners[0];
            int m = (int) contour->edges.size();
            switchColor(color, seed);
            EdgeColor initialColor = color;
            for (int i = 0; i < m; ++i) {
                int index = (start+i)%m;
                if (spline+1 < cornerCount && corners[spline+1] == index) {
                    ++spline;
                    switchColor(color, seed, EdgeColor((spline == cornerCount-1)*initialColor));
                }
                contour->edges[index]->color = color;
            }
        }
    }
}

struct EdgeColoringInkTrapCorner {
    int index;
    double prevEdgeLengthEstimate;
    bool minor;
    EdgeColor color;
};

void edgeColoringInkTrap(Shape &shape, double angleThreshold, unsigned long long seed) {
    typedef EdgeColoringInkTrapCorner Corner;
    double crossThreshold = sin(angleThreshold);
    EdgeColor color = initColor(seed);
    std::vector<Corner> corners;
    for (std::vector<Contour>::iterator contour = shape.contours.begin(); contour != shape.contours.end(); ++contour) {
        if (contour->edges.empty())
            continue;
        double splineLength = 0;
        { // Identify corners
            corners.clear();
            Vector2 prevDirection = contour->edges.back()->direction(1);
            int index = 0;
            for (std::vector<EdgeHolder>::const_iterator edge = contour->edges.begin(); edge != contour->edges.end(); ++edge, ++index) {
                if (isCorner(prevDirection.normalize(), (*edge)->direction(0).normalize(), crossThreshold)) {
                    Corner corner = { index, splineLength };
                    corners.push_back(corner);
                    splineLength = 0;
                }
                splineLength += estimateEdgeLength(*edge);
                prevDirection = (*edge)->direction(1);
            }
        }

        // Smooth contour
        if (corners.empty()) {
            switchColor(color, seed);
            for (std::vector<EdgeHolder>::iterator edge = contour->edges.begin(); edge != contour->edges.end(); ++edge)
                (*edge)->color = color;
        }
        // "Teardrop" case
        else if (corners.size() == 1) {
            EdgeColor colors[3];
            switchColor(color, seed);
            colors[0] = color;
            colors[1] = WHITE;
            switchColor(color, seed);
            colors[2] = color;
            int corner = corners[0].index;
            if (contour->edges.size() >= 3) {
                int m = (int) contour->edges.size();
                for (int i = 0; i < m; ++i)
                    contour->edges[(corner+i)%m]->color = colors[1+symmetricalTrichotomy(i, m)];
            } else if (contour->edges.size() >= 1) {
                // Less than three edge segments for three colors => edges must be split
                EdgeSegment *parts[7] = { };
                contour->edges[0]->splitInThirds(parts[0+3*corner], parts[1+3*corner], parts[2+3*corner]);
                if (contour->edges.size() >= 2) {
                    contour->edges[1]->splitInThirds(parts[3-3*corner], parts[4-3*corner], parts[5-3*corner]);
                    parts[0]->color = parts[1]->color = colors[0];
                    parts[2]->color = parts[3]->color = colors[1];
                    parts[4]->color = parts[5]->color = colors[2];
                } else {
                    parts[0]->color = colors[0];
                    parts[1]->color = colors[1];
                    parts[2]->color = colors[2];
                }
                contour->edges.clear();
                for (int i = 0; parts[i]; ++i)
                    contour->edges.push_back(EdgeHolder(parts[i]));
            }
        }
        // Multiple corners
        else {
            int cornerCount = (int) corners.size();
            int majorCornerCount = cornerCount;
            if (cornerCount > 3) {
                corners.begin()->prevEdgeLengthEstimate += splineLength;
                for (int i = 0; i < cornerCount; ++i) {
                    if (
                        corners[i].prevEdgeLengthEstimate > corners[(i+1)%cornerCount].prevEdgeLengthEstimate &&
                        corners[(i+1)%cornerCount].prevEdgeLengthEstimate < corners[(i+2)%cornerCount].prevEdgeLengthEstimate
                    ) {
                        corners[i].minor = true;
                        --majorCornerCount;
                    }
                }
            }
            EdgeColor initialColor = BLACK;
            for (int i = 0; i < cornerCount; ++i) {
                if (!corners[i].minor) {
                    --majorCornerCount;
                    switchColor(color, seed, EdgeColor(!majorCornerCount*initialColor));
                    corners[i].color = color;
                    if (!initialColor)
                        initialColor = color;
                }
            }
            for (int i = 0; i < cornerCount; ++i) {
                if (corners[i].minor) {
                    EdgeColor nextColor = corners[(i+1)%cornerCount].color;
                    corners[i].color = EdgeColor((color&nextColor)^WHITE);
                } else
                    color = corners[i].color;
            }
            int spline = 0;
            int start = corners[0].index;
            color = corners[0].color;
            int m = (int) contour->edges.size();
            for (int i = 0; i < m; ++i) {
                int index = (start+i)%m;
                if (spline+1 < cornerCount && corners[spline+1].index == index)
                    color = corners[++spline].color;
                contour->edges[index]->color = color;
            }
        }
    }
}

// EDGE COLORING BY DISTANCE - EXPERIMENTAL IMPLEMENTATION - WORK IN PROGRESS
#define MAX_RECOLOR_STEPS 16
#define EDGE_DISTANCE_PRECISION 16

static double edgeToEdgeDistance(const EdgeSegment &a, const EdgeSegment &b, int precision) {
    if (a.point(0) == b.point(0) || a.point(0) == b.point(1) || a.point(1) == b.point(0) || a.point(1) == b.point(1))
        return 0;
    double iFac = 1./precision;
    double minDistance = (b.point(0)-a.point(0)).length();
    for (int i = 0; i <= precision; ++i) {
        double t = iFac*i;
        double d = fabs(a.signedDistance(b.point(t), t).distance);
        minDistance = min(minDistance, d);
    }
    for (int i = 0; i <= precision; ++i) {
        double t = iFac*i;
        double d = fabs(b.signedDistance(a.point(t), t).distance);
        minDistance = min(minDistance, d);
    }
    return minDistance;
}

static double splineToSplineDistance(EdgeSegment *const *edgeSegments, int aStart, int aEnd, int bStart, int bEnd, int precision) {
    double minDistance = DBL_MAX;
    for (int ai = aStart; ai < aEnd; ++ai)
        for (int bi = bStart; bi < bEnd && minDistance; ++bi) {
            double d = edgeToEdgeDistance(*edgeSegments[ai], *edgeSegments[bi], precision);
            minDistance = min(minDistance, d);
        }
    return minDistance;
}

static void colorSecondDegreeGraph(int *coloring, const int *const *edgeMatrix, int vertexCount, unsigned long long seed) {
    for (int i = 0; i < vertexCount; ++i) {
        int possibleColors = 7;
        for (int j = 0; j < i; ++j) {
            if (edgeMatrix[i][j])
                possibleColors &= ~(1<<coloring[j]);
        }
        int color = 0;
        switch (possibleColors) {
            case 1:
                color = 0;
                break;
            case 2:
                color = 1;
                break;
            case 3:
                color = seedExtract2(seed); // 0 or 1
                break;
            case 4:
                color = 2;
                break;
            case 5:
                color = (int) !seedExtract2(seed)<<1; // 2 or 0
                break;
            case 6:
                color = seedExtract2(seed)+1; // 1 or 2
                break;
            case 7:
                color = (seedExtract3(seed)+i)%3; // 0 or 1 or 2
                break;
        }
        coloring[i] = color;
    }
}

static int vertexPossibleColors(const int *coloring, const int *edgeVector, int vertexCount) {
    int usedColors = 0;
    for (int i = 0; i < vertexCount; ++i)
        if (edgeVector[i])
            usedColors |= 1<<coloring[i];
    return 7&~usedColors;
}

static void uncolorSameNeighbors(std::queue<int> &uncolored, int *coloring, const int *const *edgeMatrix, int vertex, int vertexCount) {
    for (int i = vertex+1; i < vertexCount; ++i) {
        if (edgeMatrix[vertex][i] && coloring[i] == coloring[vertex]) {
            coloring[i] = -1;
            uncolored.push(i);
        }
    }
    for (int i = 0; i < vertex; ++i) {
        if (edgeMatrix[vertex][i] && coloring[i] == coloring[vertex]) {
            coloring[i] = -1;
            uncolored.push(i);
        }
    }
}

static bool tryAddEdge(int *coloring, int *const *edgeMatrix, int vertexCount, int vertexA, int vertexB, int *coloringBuffer) {
    static const int FIRST_POSSIBLE_COLOR[8] = { -1, 0, 1, 0, 2, 2, 1, 0 };
    edgeMatrix[vertexA][vertexB] = 1;
    edgeMatrix[vertexB][vertexA] = 1;
    if (coloring[vertexA] != coloring[vertexB])
        return true;
    int bPossibleColors = vertexPossibleColors(coloring, edgeMatrix[vertexB], vertexCount);
    if (bPossibleColors) {
        coloring[vertexB] = FIRST_POSSIBLE_COLOR[bPossibleColors];
        return true;
    }
    memcpy(coloringBuffer, coloring, sizeof(int)*vertexCount);
    std::queue<int> uncolored;
    {
        int *coloring = coloringBuffer;
        coloring[vertexB] = FIRST_POSSIBLE_COLOR[7&~(1<<coloring[vertexA])];
        uncolorSameNeighbors(uncolored, coloring, edgeMatrix, vertexB, vertexCount);
        int step = 0;
        while (!uncolored.empty() && step < MAX_RECOLOR_STEPS) {
            int i = uncolored.front();
            uncolored.pop();
            int possibleColors = vertexPossibleColors(coloring, edgeMatrix[i], vertexCount);
            if (possibleColors) {
                coloring[i] = FIRST_POSSIBLE_COLOR[possibleColors];
                continue;
            }
            do {
                coloring[i] = step++%3;
            } while (edgeMatrix[i][vertexA] && coloring[i] == coloring[vertexA]);
            uncolorSameNeighbors(uncolored, coloring, edgeMatrix, i, vertexCount);
        }
    }
    if (!uncolored.empty()) {
        edgeMatrix[vertexA][vertexB] = 0;
        edgeMatrix[vertexB][vertexA] = 0;
        return false;
    }
    memcpy(coloring, coloringBuffer, sizeof(int)*vertexCount);
    return true;
}

static int cmpDoublePtr(const void *a, const void *b) {
    return sign(**reinterpret_cast<const double *const *>(a)-**reinterpret_cast<const double *const *>(b));
}

void edgeColoringByDistance(Shape &shape, double angleThreshold, unsigned long long seed) {

    std::vector<EdgeSegment *> edgeSegments;
    std::vector<int> splineStarts;

    double crossThreshold = sin(angleThreshold);
    std::vector<int> corners;
    for (std::vector<Contour>::iterator contour = shape.contours.begin(); contour != shape.contours.end(); ++contour)
        if (!contour->edges.empty()) {
            // Identify corners
            corners.clear();
            Vector2 prevDirection = contour->edges.back()->direction(1);
            int index = 0;
            for (std::vector<EdgeHolder>::const_iterator edge = contour->edges.begin(); edge != contour->edges.end(); ++edge, ++index) {
                if (isCorner(prevDirection.normalize(), (*edge)->direction(0).normalize(), crossThreshold))
                    corners.push_back(index);
                prevDirection = (*edge)->direction(1);
            }

            splineStarts.push_back((int) edgeSegments.size());
            // Smooth contour
            if (corners.empty())
                for (std::vector<EdgeHolder>::iterator edge = contour->edges.begin(); edge != contour->edges.end(); ++edge)
                    edgeSegments.push_back(&**edge);
            // "Teardrop" case
            else if (corners.size() == 1) {
                int corner = corners[0];
                if (contour->edges.size() >= 3) {
                    int m = (int) contour->edges.size();
                    for (int i = 0; i < m; ++i) {
                        if (i == m/2)
                            splineStarts.push_back((int) edgeSegments.size());
                        if (symmetricalTrichotomy(i, m))
                            edgeSegments.push_back(&*contour->edges[(corner+i)%m]);
                        else
                            contour->edges[(corner+i)%m]->color = WHITE;
                    }
                } else if (contour->edges.size() >= 1) {
                    // Less than three edge segments for three colors => edges must be split
                    EdgeSegment *parts[7] = { };
                    contour->edges[0]->splitInThirds(parts[0+3*corner], parts[1+3*corner], parts[2+3*corner]);
                    if (contour->edges.size() >= 2) {
                        contour->edges[1]->splitInThirds(parts[3-3*corner], parts[4-3*corner], parts[5-3*corner]);
                        edgeSegments.push_back(parts[0]);
                        edgeSegments.push_back(parts[1]);
                        parts[2]->color = parts[3]->color = WHITE;
                        splineStarts.push_back((int) edgeSegments.size());
                        edgeSegments.push_back(parts[4]);
                        edgeSegments.push_back(parts[5]);
                    } else {
                        edgeSegments.push_back(parts[0]);
                        parts[1]->color = WHITE;
                        splineStarts.push_back((int) edgeSegments.size());
                        edgeSegments.push_back(parts[2]);
                    }
                    contour->edges.clear();
                    for (int i = 0; parts[i]; ++i)
                        contour->edges.push_back(EdgeHolder(parts[i]));
                }
            }
            // Multiple corners
            else {
                int cornerCount = (int) corners.size();
                int spline = 0;
                int start = corners[0];
                int m = (int) contour->edges.size();
                for (int i = 0; i < m; ++i) {
                    int index = (start+i)%m;
                    if (spline+1 < cornerCount && corners[spline+1] == index) {
                        splineStarts.push_back((int) edgeSegments.size());
                        ++spline;
                    }
                    edgeSegments.push_back(&*contour->edges[index]);
                }
            }
        }
    splineStarts.push_back((int) edgeSegments.size());

    int segmentCount = (int) edgeSegments.size();
    int splineCount = (int) splineStarts.size()-1;
    if (!splineCount)
        return;

    std::vector<double> distanceMatrixStorage(splineCount*splineCount);
    std::vector<double *> distanceMatrix(splineCount);
    for (int i = 0; i < splineCount; ++i)
        distanceMatrix[i] = &distanceMatrixStorage[i*splineCount];
    const double *distanceMatrixBase = &distanceMatrixStorage[0];

    for (int i = 0; i < splineCount; ++i) {
        distanceMatrix[i][i] = -1;
        for (int j = i+1; j < splineCount; ++j) {
            double dist = splineToSplineDistance(&edgeSegments[0], splineStarts[i], splineStarts[i+1], splineStarts[j], splineStarts[j+1], EDGE_DISTANCE_PRECISION);
            distanceMatrix[i][j] = dist;
            distanceMatrix[j][i] = dist;
        }
    }

    std::vector<const double *> graphEdgeDistances;
    graphEdgeDistances.reserve(splineCount*(splineCount-1)/2);
    for (int i = 0; i < splineCount; ++i)
        for (int j = i+1; j < splineCount; ++j)
            graphEdgeDistances.push_back(&distanceMatrix[i][j]);
    int graphEdgeCount = (int) graphEdgeDistances.size();
    if (!graphEdgeDistances.empty())
        qsort(&graphEdgeDistances[0], graphEdgeDistances.size(), sizeof(const double *), &cmpDoublePtr);

    std::vector<int> edgeMatrixStorage(splineCount*splineCount);
    std::vector<int *> edgeMatrix(splineCount);
    for (int i = 0; i < splineCount; ++i)
        edgeMatrix[i] = &edgeMatrixStorage[i*splineCount];
    int nextEdge = 0;
    for (; nextEdge < graphEdgeCount && !*graphEdgeDistances[nextEdge]; ++nextEdge) {
        int elem = (int) (graphEdgeDistances[nextEdge]-distanceMatrixBase);
        int row = elem/splineCount;
        int col = elem%splineCount;
        edgeMatrix[row][col] = 1;
        edgeMatrix[col][row] = 1;
    }

    std::vector<int> coloring(2*splineCount);
    colorSecondDegreeGraph(&coloring[0], &edgeMatrix[0], splineCount, seed);
    for (; nextEdge < graphEdgeCount; ++nextEdge) {
        int elem = (int) (graphEdgeDistances[nextEdge]-distanceMatrixBase);
        tryAddEdge(&coloring[0], &edgeMatrix[0], splineCount, elem/splineCount, elem%splineCount, &coloring[splineCount]);
    }

    const EdgeColor colors[3] = { YELLOW, CYAN, MAGENTA };
    int spline = -1;
    for (int i = 0; i < segmentCount; ++i) {
        if (splineStarts[spline+1] == i)
            ++spline;
        edgeSegments[i]->color = colors[coloring[spline]];
    }
}

#define DISTANCE_DELTA_FACTOR 1.001

TrueDistanceSelector::EdgeCache::EdgeCache() : absDistance(0) { }

void TrueDistanceSelector::reset(const Point2 &p) {
    double delta = DISTANCE_DELTA_FACTOR*(p-this->p).length();
    minDistance.distance += nonZeroSign(minDistance.distance)*delta;
    this->p = p;
}

void TrueDistanceSelector::addEdge(EdgeCache &cache, const EdgeSegment *prevEdge, const EdgeSegment *edge, const EdgeSegment *nextEdge) {
    double delta = DISTANCE_DELTA_FACTOR*(p-cache.point).length();
    if (cache.absDistance-delta <= fabs(minDistance.distance)) {
        double dummy;
        SignedDistance distance = edge->signedDistance(p, dummy);
        if (distance < minDistance)
            minDistance = distance;
        cache.point = p;
        cache.absDistance = fabs(distance.distance);
    }
}

void TrueDistanceSelector::merge(const TrueDistanceSelector &other) {
    if (other.minDistance < minDistance)
        minDistance = other.minDistance;
}

TrueDistanceSelector::DistanceType TrueDistanceSelector::distance() const {
    return minDistance.distance;
}

PerpendicularDistanceSelectorBase::EdgeCache::EdgeCache() : absDistance(0), aDomainDistance(0), bDomainDistance(0), aPerpendicularDistance(0), bPerpendicularDistance(0) { }

bool PerpendicularDistanceSelectorBase::getPerpendicularDistance(double &distance, const Vector2 &ep, const Vector2 &edgeDir) {
    double ts = dotProduct(ep, edgeDir);
    if (ts > 0) {
        double perpendicularDistance = crossProduct(ep, edgeDir);
        if (fabs(perpendicularDistance) < fabs(distance)) {
            distance = perpendicularDistance;
            return true;
        }
    }
    return false;
}

PerpendicularDistanceSelectorBase::PerpendicularDistanceSelectorBase() : minNegativePerpendicularDistance(-fabs(minTrueDistance.distance)), minPositivePerpendicularDistance(fabs(minTrueDistance.distance)), nearEdge(NULL), nearEdgeParam(0) { }

void PerpendicularDistanceSelectorBase::reset(double delta) {
    minTrueDistance.distance += nonZeroSign(minTrueDistance.distance)*delta;
    minNegativePerpendicularDistance = -fabs(minTrueDistance.distance);
    minPositivePerpendicularDistance = fabs(minTrueDistance.distance);
    nearEdge = NULL;
    nearEdgeParam = 0;
}

bool PerpendicularDistanceSelectorBase::isEdgeRelevant(const EdgeCache &cache, const EdgeSegment *edge, const Point2 &p) const {
    double delta = DISTANCE_DELTA_FACTOR*(p-cache.point).length();
    return (
        cache.absDistance-delta <= fabs(minTrueDistance.distance) ||
        fabs(cache.aDomainDistance) < delta ||
        fabs(cache.bDomainDistance) < delta ||
        (cache.aDomainDistance > 0 && (cache.aPerpendicularDistance < 0 ?
            cache.aPerpendicularDistance+delta >= minNegativePerpendicularDistance :
            cache.aPerpendicularDistance-delta <= minPositivePerpendicularDistance
        )) ||
        (cache.bDomainDistance > 0 && (cache.bPerpendicularDistance < 0 ?
            cache.bPerpendicularDistance+delta >= minNegativePerpendicularDistance :
            cache.bPerpendicularDistance-delta <= minPositivePerpendicularDistance
        ))
    );
}

void PerpendicularDistanceSelectorBase::addEdgeTrueDistance(const EdgeSegment *edge, const SignedDistance &distance, double param) {
    if (distance < minTrueDistance) {
        minTrueDistance = distance;
        nearEdge = edge;
        nearEdgeParam = param;
    }
}

void PerpendicularDistanceSelectorBase::addEdgePerpendicularDistance(double distance) {
    if (distance <= 0 && distance > minNegativePerpendicularDistance)
        minNegativePerpendicularDistance = distance;
    if (distance >= 0 && distance < minPositivePerpendicularDistance)
        minPositivePerpendicularDistance = distance;
}

void PerpendicularDistanceSelectorBase::merge(const PerpendicularDistanceSelectorBase &other) {
    if (other.minTrueDistance < minTrueDistance) {
        minTrueDistance = other.minTrueDistance;
        nearEdge = other.nearEdge;
        nearEdgeParam = other.nearEdgeParam;
    }
    if (other.minNegativePerpendicularDistance > minNegativePerpendicularDistance)
        minNegativePerpendicularDistance = other.minNegativePerpendicularDistance;
    if (other.minPositivePerpendicularDistance < minPositivePerpendicularDistance)
        minPositivePerpendicularDistance = other.minPositivePerpendicularDistance;
}

double PerpendicularDistanceSelectorBase::computeDistance(const Point2 &p) const {
    double minDistance = minTrueDistance.distance < 0 ? minNegativePerpendicularDistance : minPositivePerpendicularDistance;
    if (nearEdge) {
        SignedDistance distance = minTrueDistance;
        nearEdge->distanceToPerpendicularDistance(distance, p, nearEdgeParam);
        if (fabs(distance.distance) < fabs(minDistance))
            minDistance = distance.distance;
    }
    return minDistance;
}

SignedDistance PerpendicularDistanceSelectorBase::trueDistance() const {
    return minTrueDistance;
}

void PerpendicularDistanceSelector::reset(const Point2 &p) {
    double delta = DISTANCE_DELTA_FACTOR*(p-this->p).length();
    PerpendicularDistanceSelectorBase::reset(delta);
    this->p = p;
}

void PerpendicularDistanceSelector::addEdge(EdgeCache &cache, const EdgeSegment *prevEdge, const EdgeSegment *edge, const EdgeSegment *nextEdge) {
    if (isEdgeRelevant(cache, edge, p)) {
        double param;
        SignedDistance distance = edge->signedDistance(p, param);
        addEdgeTrueDistance(edge, distance, param);
        cache.point = p;
        cache.absDistance = fabs(distance.distance);

        Vector2 ap = p-edge->point(0);
        Vector2 bp = p-edge->point(1);
        Vector2 aDir = edge->direction(0).normalize(true);
        Vector2 bDir = edge->direction(1).normalize(true);
        Vector2 prevDir = prevEdge->direction(1).normalize(true);
        Vector2 nextDir = nextEdge->direction(0).normalize(true);
        double add = dotProduct(ap, (prevDir+aDir).normalize(true));
        double bdd = -dotProduct(bp, (bDir+nextDir).normalize(true));
        if (add > 0) {
            double pd = distance.distance;
            if (getPerpendicularDistance(pd, ap, -aDir))
                addEdgePerpendicularDistance(pd = -pd);
            cache.aPerpendicularDistance = pd;
        }
        if (bdd > 0) {
            double pd = distance.distance;
            if (getPerpendicularDistance(pd, bp, bDir))
                addEdgePerpendicularDistance(pd);
            cache.bPerpendicularDistance = pd;
        }
        cache.aDomainDistance = add;
        cache.bDomainDistance = bdd;
    }
}

PerpendicularDistanceSelector::DistanceType PerpendicularDistanceSelector::distance() const {
    return computeDistance(p);
}

void MultiDistanceSelector::reset(const Point2 &p) {
    double delta = DISTANCE_DELTA_FACTOR*(p-this->p).length();
    r.reset(delta);
    g.reset(delta);
    b.reset(delta);
    this->p = p;
}

void MultiDistanceSelector::addEdge(EdgeCache &cache, const EdgeSegment *prevEdge, const EdgeSegment *edge, const EdgeSegment *nextEdge) {
    if (
        (edge->color&RED && r.isEdgeRelevant(cache, edge, p)) ||
        (edge->color&GREEN && g.isEdgeRelevant(cache, edge, p)) ||
        (edge->color&BLUE && b.isEdgeRelevant(cache, edge, p))
    ) {
        double param;
        SignedDistance distance = edge->signedDistance(p, param);
        if (edge->color&RED)
            r.addEdgeTrueDistance(edge, distance, param);
        if (edge->color&GREEN)
            g.addEdgeTrueDistance(edge, distance, param);
        if (edge->color&BLUE)
            b.addEdgeTrueDistance(edge, distance, param);
        cache.point = p;
        cache.absDistance = fabs(distance.distance);

        Vector2 ap = p-edge->point(0);
        Vector2 bp = p-edge->point(1);
        Vector2 aDir = edge->direction(0).normalize(true);
        Vector2 bDir = edge->direction(1).normalize(true);
        Vector2 prevDir = prevEdge->direction(1).normalize(true);
        Vector2 nextDir = nextEdge->direction(0).normalize(true);
        double add = dotProduct(ap, (prevDir+aDir).normalize(true));
        double bdd = -dotProduct(bp, (bDir+nextDir).normalize(true));
        if (add > 0) {
            double pd = distance.distance;
            if (PerpendicularDistanceSelectorBase::getPerpendicularDistance(pd, ap, -aDir)) {
                pd = -pd;
                if (edge->color&RED)
                    r.addEdgePerpendicularDistance(pd);
                if (edge->color&GREEN)
                    g.addEdgePerpendicularDistance(pd);
                if (edge->color&BLUE)
                    b.addEdgePerpendicularDistance(pd);
            }
            cache.aPerpendicularDistance = pd;
        }
        if (bdd > 0) {
            double pd = distance.distance;
            if (PerpendicularDistanceSelectorBase::getPerpendicularDistance(pd, bp, bDir)) {
                if (edge->color&RED)
                    r.addEdgePerpendicularDistance(pd);
                if (edge->color&GREEN)
                    g.addEdgePerpendicularDistance(pd);
                if (edge->color&BLUE)
                    b.addEdgePerpendicularDistance(pd);
            }
            cache.bPerpendicularDistance = pd;
        }
        cache.aDomainDistance = add;
        cache.bDomainDistance = bdd;
    }
}

void MultiDistanceSelector::merge(const MultiDistanceSelector &other) {
    r.merge(other.r);
    g.merge(other.g);
    b.merge(other.b);
}

MultiDistanceSelector::DistanceType MultiDistanceSelector::distance() const {
    MultiDistance multiDistance;
    multiDistance.r = r.computeDistance(p);
    multiDistance.g = g.computeDistance(p);
    multiDistance.b = b.computeDistance(p);
    return multiDistance;
}

SignedDistance MultiDistanceSelector::trueDistance() const {
    SignedDistance distance = r.trueDistance();
    if (g.trueDistance() < distance)
        distance = g.trueDistance();
    if (b.trueDistance() < distance)
        distance = b.trueDistance();
    return distance;
}

MultiAndTrueDistanceSelector::DistanceType MultiAndTrueDistanceSelector::distance() const {
    MultiDistance multiDistance = MultiDistanceSelector::distance();
    MultiAndTrueDistance mtd;
    mtd.r = multiDistance.r;
    mtd.g = multiDistance.g;
    mtd.b = multiDistance.b;
    mtd.a = trueDistance().distance;
    return mtd;
}

static void initDistance(double &distance) {
    distance = -DBL_MAX;
}

static void initDistance(MultiDistance &distance) {
    distance.r = -DBL_MAX;
    distance.g = -DBL_MAX;
    distance.b = -DBL_MAX;
}

static void initDistance(MultiAndTrueDistance &distance) {
    distance.r = -DBL_MAX;
    distance.g = -DBL_MAX;
    distance.b = -DBL_MAX;
    distance.a = -DBL_MAX;
}

static double resolveDistance(double distance) {
    return distance;
}

static double resolveDistance(const MultiDistance &distance) {
    return median(distance.r, distance.g, distance.b);
}

template <class EdgeSelector>
SimpleContourCombiner<EdgeSelector>::SimpleContourCombiner(const Shape &shape) { }

template <class EdgeSelector>
void SimpleContourCombiner<EdgeSelector>::reset(const Point2 &p) {
    shapeEdgeSelector.reset(p);
}

template <class EdgeSelector>
EdgeSelector &SimpleContourCombiner<EdgeSelector>::edgeSelector(int) {
    return shapeEdgeSelector;
}

template <class EdgeSelector>
typename SimpleContourCombiner<EdgeSelector>::DistanceType SimpleContourCombiner<EdgeSelector>::distance() const {
    return shapeEdgeSelector.distance();
}

template class SimpleContourCombiner<TrueDistanceSelector>;
template class SimpleContourCombiner<PerpendicularDistanceSelector>;
template class SimpleContourCombiner<MultiDistanceSelector>;
template class SimpleContourCombiner<MultiAndTrueDistanceSelector>;

template <class EdgeSelector>
OverlappingContourCombiner<EdgeSelector>::OverlappingContourCombiner(const Shape &shape) {
    windings.reserve(shape.contours.size());
    for (std::vector<Contour>::const_iterator contour = shape.contours.begin(); contour != shape.contours.end(); ++contour)
        windings.push_back(contour->winding());
    edgeSelectors.resize(shape.contours.size());
}

template <class EdgeSelector>
void OverlappingContourCombiner<EdgeSelector>::reset(const Point2 &p) {
    this->p = p;
    for (typename std::vector<EdgeSelector>::iterator contourEdgeSelector = edgeSelectors.begin(); contourEdgeSelector != edgeSelectors.end(); ++contourEdgeSelector)
        contourEdgeSelector->reset(p);
}

template <class EdgeSelector>
EdgeSelector &OverlappingContourCombiner<EdgeSelector>::edgeSelector(int i) {
    return edgeSelectors[i];
}

template <class EdgeSelector>
typename OverlappingContourCombiner<EdgeSelector>::DistanceType OverlappingContourCombiner<EdgeSelector>::distance() const {
    int contourCount = (int) edgeSelectors.size();
    EdgeSelector shapeEdgeSelector;
    EdgeSelector innerEdgeSelector;
    EdgeSelector outerEdgeSelector;
    shapeEdgeSelector.reset(p);
    innerEdgeSelector.reset(p);
    outerEdgeSelector.reset(p);
    for (int i = 0; i < contourCount; ++i) {
        DistanceType edgeDistance = edgeSelectors[i].distance();
        shapeEdgeSelector.merge(edgeSelectors[i]);
        if (windings[i] > 0 && resolveDistance(edgeDistance) >= 0)
            innerEdgeSelector.merge(edgeSelectors[i]);
        if (windings[i] < 0 && resolveDistance(edgeDistance) <= 0)
            outerEdgeSelector.merge(edgeSelectors[i]);
    }

    DistanceType shapeDistance = shapeEdgeSelector.distance();
    DistanceType innerDistance = innerEdgeSelector.distance();
    DistanceType outerDistance = outerEdgeSelector.distance();
    double innerScalarDistance = resolveDistance(innerDistance);
    double outerScalarDistance = resolveDistance(outerDistance);
    DistanceType distance;
    initDistance(distance);

    int winding = 0;
    if (innerScalarDistance >= 0 && fabs(innerScalarDistance) <= fabs(outerScalarDistance)) {
        distance = innerDistance;
        winding = 1;
        for (int i = 0; i < contourCount; ++i)
            if (windings[i] > 0) {
                DistanceType contourDistance = edgeSelectors[i].distance();
                if (fabs(resolveDistance(contourDistance)) < fabs(outerScalarDistance) && resolveDistance(contourDistance) > resolveDistance(distance))
                    distance = contourDistance;
            }
    } else if (outerScalarDistance <= 0 && fabs(outerScalarDistance) < fabs(innerScalarDistance)) {
        distance = outerDistance;
        winding = -1;
        for (int i = 0; i < contourCount; ++i)
            if (windings[i] < 0) {
                DistanceType contourDistance = edgeSelectors[i].distance();
                if (fabs(resolveDistance(contourDistance)) < fabs(innerScalarDistance) && resolveDistance(contourDistance) < resolveDistance(distance))
                    distance = contourDistance;
            }
    } else
        return shapeDistance;

    for (int i = 0; i < contourCount; ++i)
        if (windings[i] != winding) {
            DistanceType contourDistance = edgeSelectors[i].distance();
            if (resolveDistance(contourDistance)*resolveDistance(distance) >= 0 && fabs(resolveDistance(contourDistance)) < fabs(resolveDistance(distance)))
                distance = contourDistance;
        }
    if (resolveDistance(distance) == resolveDistance(shapeDistance))
        distance = shapeDistance;
    return distance;
}

template class OverlappingContourCombiner<TrueDistanceSelector>;
template class OverlappingContourCombiner<PerpendicularDistanceSelector>;
template class OverlappingContourCombiner<MultiDistanceSelector>;
template class OverlappingContourCombiner<MultiAndTrueDistanceSelector>;

}

#define ESTSDF_MAX_DIST 1e24f // Cannot be FLT_MAX because it might be divided by range, which could be < 1

namespace msdfgen {

struct Entry {
    float absDist;
    int bitmapX, bitmapY;
    Point2 nearPoint;

    bool operator<(const Entry &other) const {
        return absDist > other.absDist;
    }
} entry;

template <int N>
static void msdfErrorCorrectionInner(const BitmapRef<float, N> &sdf, const Shape &shape, const SDFTransformation &transformation, const MSDFGeneratorConfig &config) {
    if (config.errorCorrection.mode == ErrorCorrectionConfig::DISABLED)
        return;
    Bitmap<byte, 1> stencilBuffer;
    if (!config.errorCorrection.buffer)
        stencilBuffer = Bitmap<byte, 1>(sdf.width, sdf.height);
    BitmapRef<byte, 1> stencil;
    stencil.pixels = config.errorCorrection.buffer ? config.errorCorrection.buffer : (byte *) stencilBuffer;
    stencil.width = sdf.width, stencil.height = sdf.height;
    MSDFErrorCorrection ec(stencil, transformation);
    ec.setMinDeviationRatio(config.errorCorrection.minDeviationRatio);
    ec.setMinImproveRatio(config.errorCorrection.minImproveRatio);
    switch (config.errorCorrection.mode) {
        case ErrorCorrectionConfig::DISABLED:
        case ErrorCorrectionConfig::INDISCRIMINATE:
            break;
        case ErrorCorrectionConfig::EDGE_PRIORITY:
            ec.protectCorners(shape);
            ec.protectEdges<N>(sdf);
            break;
        case ErrorCorrectionConfig::EDGE_ONLY:
            ec.protectAll();
            break;
    }
    if (config.errorCorrection.distanceCheckMode == ErrorCorrectionConfig::DO_NOT_CHECK_DISTANCE || (config.errorCorrection.distanceCheckMode == ErrorCorrectionConfig::CHECK_DISTANCE_AT_EDGE && config.errorCorrection.mode != ErrorCorrectionConfig::EDGE_ONLY)) {
        ec.findErrors<N>(sdf);
        if (config.errorCorrection.distanceCheckMode == ErrorCorrectionConfig::CHECK_DISTANCE_AT_EDGE)
            ec.protectAll();
    }
    if (config.errorCorrection.distanceCheckMode == ErrorCorrectionConfig::ALWAYS_CHECK_DISTANCE || config.errorCorrection.distanceCheckMode == ErrorCorrectionConfig::CHECK_DISTANCE_AT_EDGE) {
        if (config.overlapSupport)
            ec.findErrors<OverlappingContourCombiner, N>(sdf, shape);
        else
            ec.findErrors<SimpleContourCombiner, N>(sdf, shape);
    }
    ec.apply(sdf);
}

template <int N>
static void msdfErrorCorrectionShapeless(const BitmapRef<float, N> &sdf, const SDFTransformation &transformation, double minDeviationRatio, bool protectAll) {
    Bitmap<byte, 1> stencilBuffer(sdf.width, sdf.height);
    MSDFErrorCorrection ec(stencilBuffer, transformation);
    ec.setMinDeviationRatio(minDeviationRatio);
    if (protectAll)
        ec.protectAll();
    ec.findErrors<N>(sdf);
    ec.apply(sdf);
}

void msdfErrorCorrection(const BitmapRef<float, 4> &sdf, const Shape &shape, const SDFTransformation &transformation, const MSDFGeneratorConfig &config) {
    msdfErrorCorrectionInner(sdf, shape, transformation, config);
}

#define ARTIFACT_T_EPSILON .01
#define PROTECTION_RADIUS_TOLERANCE 1.001

#define CLASSIFIER_FLAG_CANDIDATE 0x01
#define CLASSIFIER_FLAG_ARTIFACT 0x02

MSDFGEN_PUBLIC const double ErrorCorrectionConfig::defaultMinDeviationRatio = 1.11111111111111111;
MSDFGEN_PUBLIC const double ErrorCorrectionConfig::defaultMinImproveRatio = 1.11111111111111111;

/// The base artifact classifier recognizes artifacts based on the contents of the SDF alone.
class BaseArtifactClassifier {
public:
    inline BaseArtifactClassifier(double span, bool protectedFlag) : span(span), protectedFlag(protectedFlag) { }
    /// Evaluates if the median value xm interpolated at xt in the range between am at at and bm at bt indicates an artifact.
    inline int rangeTest(double at, double bt, double xt, float am, float bm, float xm) const {
        // For protected texels, only consider inversion artifacts (interpolated median has different sign than boundaries). For the rest, it is sufficient that the interpolated median is outside its boundaries.
        if ((am > .5f && bm > .5f && xm <= .5f) || (am < .5f && bm < .5f && xm >= .5f) || (!protectedFlag && median(am, bm, xm) != xm)) {
            double axSpan = (xt-at)*span, bxSpan = (bt-xt)*span;
            // Check if the interpolated median's value is in the expected range based on its distance (span) from boundaries a, b.
            if (!(xm >= am-axSpan && xm <= am+axSpan && xm >= bm-bxSpan && xm <= bm+bxSpan))
                return CLASSIFIER_FLAG_CANDIDATE|CLASSIFIER_FLAG_ARTIFACT;
            return CLASSIFIER_FLAG_CANDIDATE;
        }
        return 0;
    }
    /// Returns true if the combined results of the tests performed on the median value m interpolated at t indicate an artifact.
    inline bool evaluate(double t, float m, int flags) const {
        return (flags&2) != 0;
    }
private:
    double span;
    bool protectedFlag;
};

/// The shape distance checker evaluates the exact shape distance to find additional artifacts at a significant performance cost.
template <template <typename> class ContourCombiner, int N>
class ShapeDistanceChecker {
public:
    class ArtifactClassifier : public BaseArtifactClassifier {
    public:
        inline ArtifactClassifier(ShapeDistanceChecker *parent, const Vector2 &direction, double span) : BaseArtifactClassifier(span, parent->protectedFlag), parent(parent), direction(direction) { }
        /// Returns true if the combined results of the tests performed on the median value m interpolated at t indicate an artifact.
        inline bool evaluate(double t, float m, int flags) const {
            if (flags&CLASSIFIER_FLAG_CANDIDATE) {
                // Skip expensive distance evaluation if the point has already been classified as an artifact by the base classifier.
                if (flags&CLASSIFIER_FLAG_ARTIFACT)
                    return true;
                Vector2 tVector = t*direction;
                float oldMSD[N], newMSD[3];
                // Compute the color that would be currently interpolated at the artifact candidate's position.
                Point2 sdfCoord = parent->sdfCoord+tVector;
                interpolate(oldMSD, parent->sdf, sdfCoord);
                // Compute the color that would be interpolated at the artifact candidate's position if error correction was applied on the current texel.
                double aWeight = (1-fabs(tVector.x))*(1-fabs(tVector.y));
                float aPSD = median(parent->msd[0], parent->msd[1], parent->msd[2]);
                newMSD[0] = float(oldMSD[0]+aWeight*(aPSD-parent->msd[0]));
                newMSD[1] = float(oldMSD[1]+aWeight*(aPSD-parent->msd[1]));
                newMSD[2] = float(oldMSD[2]+aWeight*(aPSD-parent->msd[2]));
                // Compute the evaluated distance (interpolated median) before and after error correction, as well as the exact shape distance.
                float oldPSD = median(oldMSD[0], oldMSD[1], oldMSD[2]);
                float newPSD = median(newMSD[0], newMSD[1], newMSD[2]);
                float refPSD = float(parent->distanceMapping(parent->distanceFinder.distance(parent->shapeCoord+tVector*parent->texelSize)));
                // Compare the differences of the exact distance and the before and after distances.
                return parent->minImproveRatio*fabsf(newPSD-refPSD) < double(fabsf(oldPSD-refPSD));
            }
            return false;
        }
    private:
        ShapeDistanceChecker *parent;
        Vector2 direction;
    };
    Point2 shapeCoord, sdfCoord;
    const float *msd;
    bool protectedFlag;
    inline ShapeDistanceChecker(const BitmapConstRef<float, N> &sdf, const Shape &shape, const Projection &projection, DistanceMapping distanceMapping, double minImproveRatio) : distanceFinder(shape), sdf(sdf), distanceMapping(distanceMapping), minImproveRatio(minImproveRatio) {
        texelSize = projection.unprojectVector(Vector2(1));
        if (shape.inverseYAxis)
            texelSize.y = -texelSize.y;
    }
    inline ArtifactClassifier classifier(const Vector2 &direction, double span) {
        return ArtifactClassifier(this, direction, span);
    }
private:
    ShapeDistanceFinder<ContourCombiner<PerpendicularDistanceSelector> > distanceFinder;
    BitmapConstRef<float, N> sdf;
    DistanceMapping distanceMapping;
    Vector2 texelSize;
    double minImproveRatio;
};

MSDFErrorCorrection::MSDFErrorCorrection() { }

MSDFErrorCorrection::MSDFErrorCorrection(const BitmapRef<byte, 1> &stencil, const SDFTransformation &transformation) : stencil(stencil), transformation(transformation) {
    minDeviationRatio = ErrorCorrectionConfig::defaultMinDeviationRatio;
    minImproveRatio = ErrorCorrectionConfig::defaultMinImproveRatio;
    memset(stencil.pixels, 0, sizeof(byte)*stencil.width*stencil.height);
}

void MSDFErrorCorrection::setMinDeviationRatio(double minDeviationRatio) {
    this->minDeviationRatio = minDeviationRatio;
}

void MSDFErrorCorrection::setMinImproveRatio(double minImproveRatio) {
    this->minImproveRatio = minImproveRatio;
}

void MSDFErrorCorrection::protectCorners(const Shape &shape) {
    for (std::vector<Contour>::const_iterator contour = shape.contours.begin(); contour != shape.contours.end(); ++contour)
        if (!contour->edges.empty()) {
            const EdgeSegment *prevEdge = contour->edges.back();
            for (std::vector<EdgeHolder>::const_iterator edge = contour->edges.begin(); edge != contour->edges.end(); ++edge) {
                int commonColor = prevEdge->color&(*edge)->color;
                // If the color changes from prevEdge to edge, this is a corner.
                if (!(commonColor&(commonColor-1))) {
                    // Find the four texels that envelop the corner and mark them as protected.
                    Point2 p = transformation.project((*edge)->point(0));
                    int l = (int) floor(p.x-.5);
                    int b = (int) floor(p.y-.5);
                    if (shape.inverseYAxis)
                        b = stencil.height-b-2;
                    int r = l+1;
                    int t = b+1;
                    // Check that the positions are within bounds.
                    if (l < stencil.width && b < stencil.height && r >= 0 && t >= 0) {
                        if (l >= 0 && b >= 0)
                            *stencil(l, b) |= (byte) PROTECTED;
                        if (r < stencil.width && b >= 0)
                            *stencil(r, b) |= (byte) PROTECTED;
                        if (l >= 0 && t < stencil.height)
                            *stencil(l, t) |= (byte) PROTECTED;
                        if (r < stencil.width && t < stencil.height)
                            *stencil(r, t) |= (byte) PROTECTED;
                    }
                }
                prevEdge = *edge;
            }
        }
}

/// Determines if the channel contributes to an edge between the two texels a, b.
static bool edgeBetweenTexelsChannel(const float *a, const float *b, int channel) {
    // Find interpolation ratio t (0 < t < 1) where an edge is expected (mix(a[channel], b[channel], t) == 0.5).
    double t = (a[channel]-.5)/(a[channel]-b[channel]);
    if (t > 0 && t < 1) {
        // Interpolate channel values at t.
        float c[3] = {
            mix(a[0], b[0], t),
            mix(a[1], b[1], t),
            mix(a[2], b[2], t)
        };
        // This is only an edge if the zero-distance channel is the median.
        return median(c[0], c[1], c[2]) == c[channel];
    }
    return false;
}

/// Returns a bit mask of which channels contribute to an edge between the two texels a, b.
static int edgeBetweenTexels(const float *a, const float *b) {
    return (
        RED*edgeBetweenTexelsChannel(a, b, 0)+
        GREEN*edgeBetweenTexelsChannel(a, b, 1)+
        BLUE*edgeBetweenTexelsChannel(a, b, 2)
    );
}

/// Marks texel as protected if one of its non-median channels is present in the channel mask.
static void protectExtremeChannels(byte *stencil, const float *msd, float m, int mask) {
    if (
        (mask&RED && msd[0] != m) ||
        (mask&GREEN && msd[1] != m) ||
        (mask&BLUE && msd[2] != m)
    )
        *stencil |= (byte) MSDFErrorCorrection::PROTECTED;
}

template <int N>
void MSDFErrorCorrection::protectEdges(const BitmapConstRef<float, N> &sdf) {
    float radius;
    // Horizontal texel pairs
    radius = float(PROTECTION_RADIUS_TOLERANCE*transformation.unprojectVector(Vector2(transformation.distanceMapping(DistanceMapping::Delta(1)), 0)).length());
    for (int y = 0; y < sdf.height; ++y) {
        const float *left = sdf(0, y);
        const float *right = sdf(1, y);
        for (int x = 0; x < sdf.width-1; ++x) {
            float lm = median(left[0], left[1], left[2]);
            float rm = median(right[0], right[1], right[2]);
            if (fabsf(lm-.5f)+fabsf(rm-.5f) < radius) {
                int mask = edgeBetweenTexels(left, right);
                protectExtremeChannels(stencil(x, y), left, lm, mask);
                protectExtremeChannels(stencil(x+1, y), right, rm, mask);
            }
            left += N, right += N;
        }
    }
    // Vertical texel pairs
    radius = float(PROTECTION_RADIUS_TOLERANCE*transformation.unprojectVector(Vector2(0, transformation.distanceMapping(DistanceMapping::Delta(1)))).length());
    for (int y = 0; y < sdf.height-1; ++y) {
        const float *bottom = sdf(0, y);
        const float *top = sdf(0, y+1);
        for (int x = 0; x < sdf.width; ++x) {
            float bm = median(bottom[0], bottom[1], bottom[2]);
            float tm = median(top[0], top[1], top[2]);
            if (fabsf(bm-.5f)+fabsf(tm-.5f) < radius) {
                int mask = edgeBetweenTexels(bottom, top);
                protectExtremeChannels(stencil(x, y), bottom, bm, mask);
                protectExtremeChannels(stencil(x, y+1), top, tm, mask);
            }
            bottom += N, top += N;
        }
    }
    // Diagonal texel pairs
    radius = float(PROTECTION_RADIUS_TOLERANCE*transformation.unprojectVector(Vector2(transformation.distanceMapping(DistanceMapping::Delta(1)))).length());
    for (int y = 0; y < sdf.height-1; ++y) {
        const float *lb = sdf(0, y);
        const float *rb = sdf(1, y);
        const float *lt = sdf(0, y+1);
        const float *rt = sdf(1, y+1);
        for (int x = 0; x < sdf.width-1; ++x) {
            float mlb = median(lb[0], lb[1], lb[2]);
            float mrb = median(rb[0], rb[1], rb[2]);
            float mlt = median(lt[0], lt[1], lt[2]);
            float mrt = median(rt[0], rt[1], rt[2]);
            if (fabsf(mlb-.5f)+fabsf(mrt-.5f) < radius) {
                int mask = edgeBetweenTexels(lb, rt);
                protectExtremeChannels(stencil(x, y), lb, mlb, mask);
                protectExtremeChannels(stencil(x+1, y+1), rt, mrt, mask);
            }
            if (fabsf(mrb-.5f)+fabsf(mlt-.5f) < radius) {
                int mask = edgeBetweenTexels(rb, lt);
                protectExtremeChannels(stencil(x+1, y), rb, mrb, mask);
                protectExtremeChannels(stencil(x, y+1), lt, mlt, mask);
            }
            lb += N, rb += N, lt += N, rt += N;
        }
    }
}

void MSDFErrorCorrection::protectAll() {
    byte *end = stencil.pixels+stencil.width*stencil.height;
    for (byte *mask = stencil.pixels; mask < end; ++mask)
        *mask |= (byte) PROTECTED;
}

/// Returns the median of the linear interpolation of texels a, b at t.
static float interpolatedMedian(const float *a, const float *b, double t) {
    return median(
        mix(a[0], b[0], t),
        mix(a[1], b[1], t),
        mix(a[2], b[2], t)
    );
}
/// Returns the median of the bilinear interpolation with the given constant, linear, and quadratic terms at t.
static float interpolatedMedian(const float *a, const float *l, const float *q, double t) {
    return float(median(
        t*(t*q[0]+l[0])+a[0],
        t*(t*q[1]+l[1])+a[1],
        t*(t*q[2]+l[2])+a[2]
    ));
}

/// Determines if the interpolated median xm is an artifact.
static bool isArtifact(bool isProtected, double axSpan, double bxSpan, float am, float bm, float xm) {
    return (
        // For protected texels, only report an artifact if it would cause fill inversion (change between positive and negative distance).
        (!isProtected || (am > .5f && bm > .5f && xm <= .5f) || (am < .5f && bm < .5f && xm >= .5f)) &&
        // This is an artifact if the interpolated median is outside the range of possible values based on its distance from a, b.
        !(xm >= am-axSpan && xm <= am+axSpan && xm >= bm-bxSpan && xm <= bm+bxSpan)
    );
}

/// Checks if a linear interpolation artifact will occur at a point where two specific color channels are equal - such points have extreme median values.
template <class ArtifactClassifier>
static bool hasLinearArtifactInner(const ArtifactClassifier &artifactClassifier, float am, float bm, const float *a, const float *b, float dA, float dB) {
    // Find interpolation ratio t (0 < t < 1) where two color channels are equal (mix(dA, dB, t) == 0).
    double t = (double) dA/(dA-dB);
    if (t > ARTIFACT_T_EPSILON && t < 1-ARTIFACT_T_EPSILON) {
        // Interpolate median at t and let the classifier decide if its value indicates an artifact.
        float xm = interpolatedMedian(a, b, t);
        return artifactClassifier.evaluate(t, xm, artifactClassifier.rangeTest(0, 1, t, am, bm, xm));
    }
    return false;
}

/// Checks if a bilinear interpolation artifact will occur at a point where two specific color channels are equal - such points have extreme median values.
template <class ArtifactClassifier>
static bool hasDiagonalArtifactInner(const ArtifactClassifier &artifactClassifier, float am, float dm, const float *a, const float *l, const float *q, float dA, float dBC, float dD, double tEx0, double tEx1) {
    // Find interpolation ratios t (0 < t[i] < 1) where two color channels are equal.
    double t[2];
    int solutions = solveQuadratic(t, dD-dBC+dA, dBC-dA-dA, dA);
    for (int i = 0; i < solutions; ++i) {
        // Solutions t[i] == 0 and t[i] == 1 are singularities and occur very often because two channels are usually equal at texels.
        if (t[i] > ARTIFACT_T_EPSILON && t[i] < 1-ARTIFACT_T_EPSILON) {
            // Interpolate median xm at t.
            float xm = interpolatedMedian(a, l, q, t[i]);
            // Determine if xm deviates too much from medians of a, d.
            int rangeFlags = artifactClassifier.rangeTest(0, 1, t[i], am, dm, xm);
            // Additionally, check xm against the interpolated medians at the local extremes tEx0, tEx1.
            double tEnd[2];
            float em[2];
            // tEx0
            if (tEx0 > 0 && tEx0 < 1) {
                tEnd[0] = 0, tEnd[1] = 1;
                em[0] = am, em[1] = dm;
                tEnd[tEx0 > t[i]] = tEx0;
                em[tEx0 > t[i]] = interpolatedMedian(a, l, q, tEx0);
                rangeFlags |= artifactClassifier.rangeTest(tEnd[0], tEnd[1], t[i], em[0], em[1], xm);
            }
            // tEx1
            if (tEx1 > 0 && tEx1 < 1) {
                tEnd[0] = 0, tEnd[1] = 1;
                em[0] = am, em[1] = dm;
                tEnd[tEx1 > t[i]] = tEx1;
                em[tEx1 > t[i]] = interpolatedMedian(a, l, q, tEx1);
                rangeFlags |= artifactClassifier.rangeTest(tEnd[0], tEnd[1], t[i], em[0], em[1], xm);
            }
            if (artifactClassifier.evaluate(t[i], xm, rangeFlags))
                return true;
        }
    }
    return false;
}

/// Checks if a linear interpolation artifact will occur inbetween two horizontally or vertically adjacent texels a, b.
template <class ArtifactClassifier>
static bool hasLinearArtifact(const ArtifactClassifier &artifactClassifier, float am, const float *a, const float *b) {
    float bm = median(b[0], b[1], b[2]);
    return (
        // Out of the pair, only report artifacts for the texel further from the edge to minimize side effects.
        fabsf(am-.5f) >= fabsf(bm-.5f) && (
            // Check points where each pair of color channels meets.
            hasLinearArtifactInner(artifactClassifier, am, bm, a, b, a[1]-a[0], b[1]-b[0]) ||
            hasLinearArtifactInner(artifactClassifier, am, bm, a, b, a[2]-a[1], b[2]-b[1]) ||
            hasLinearArtifactInner(artifactClassifier, am, bm, a, b, a[0]-a[2], b[0]-b[2])
        )
    );
}

/// Checks if a bilinear interpolation artifact will occur inbetween two diagonally adjacent texels a, d (with b, c forming the other diagonal).
template <class ArtifactClassifier>
static bool hasDiagonalArtifact(const ArtifactClassifier &artifactClassifier, float am, const float *a, const float *b, const float *c, const float *d) {
    float dm = median(d[0], d[1], d[2]);
    // Out of the pair, only report artifacts for the texel further from the edge to minimize side effects.
    if (fabsf(am-.5f) >= fabsf(dm-.5f)) {
        float abc[3] = {
            a[0]-b[0]-c[0],
            a[1]-b[1]-c[1],
            a[2]-b[2]-c[2]
        };
        // Compute the linear terms for bilinear interpolation.
        float l[3] = {
            -a[0]-abc[0],
            -a[1]-abc[1],
            -a[2]-abc[2]
        };
        // Compute the quadratic terms for bilinear interpolation.
        float q[3] = {
            d[0]+abc[0],
            d[1]+abc[1],
            d[2]+abc[2]
        };
        // Compute interpolation ratios tEx (0 < tEx[i] < 1) for the local extremes of each color channel (the derivative 2*q[i]*tEx[i]+l[i] == 0).
        double tEx[3] = {
            -.5*l[0]/q[0],
            -.5*l[1]/q[1],
            -.5*l[2]/q[2]
        };
        // Check points where each pair of color channels meets.
        return (
            hasDiagonalArtifactInner(artifactClassifier, am, dm, a, l, q, a[1]-a[0], b[1]-b[0]+c[1]-c[0], d[1]-d[0], tEx[0], tEx[1]) ||
            hasDiagonalArtifactInner(artifactClassifier, am, dm, a, l, q, a[2]-a[1], b[2]-b[1]+c[2]-c[1], d[2]-d[1], tEx[1], tEx[2]) ||
            hasDiagonalArtifactInner(artifactClassifier, am, dm, a, l, q, a[0]-a[2], b[0]-b[2]+c[0]-c[2], d[0]-d[2], tEx[2], tEx[0])
        );
    }
    return false;
}

template <int N>
void MSDFErrorCorrection::findErrors(const BitmapConstRef<float, N> &sdf) {
    // Compute the expected deltas between values of horizontally, vertically, and diagonally adjacent texels.
    double hSpan = minDeviationRatio*transformation.unprojectVector(Vector2(transformation.distanceMapping(DistanceMapping::Delta(1)), 0)).length();
    double vSpan = minDeviationRatio*transformation.unprojectVector(Vector2(0, transformation.distanceMapping(DistanceMapping::Delta(1)))).length();
    double dSpan = minDeviationRatio*transformation.unprojectVector(Vector2(transformation.distanceMapping(DistanceMapping::Delta(1)))).length();
    // Inspect all texels.
    for (int y = 0; y < sdf.height; ++y) {
        for (int x = 0; x < sdf.width; ++x) {
            const float *c = sdf(x, y);
            float cm = median(c[0], c[1], c[2]);
            bool protectedFlag = (*stencil(x, y)&PROTECTED) != 0;
            const float *l = NULL, *b = NULL, *r = NULL, *t = NULL;
            // Mark current texel c with the error flag if an artifact occurs when it's interpolated with any of its 8 neighbors.
            *stencil(x, y) |= (byte) (ERROR*(
                (x > 0 && ((l = sdf(x-1, y)), hasLinearArtifact(BaseArtifactClassifier(hSpan, protectedFlag), cm, c, l))) ||
                (y > 0 && ((b = sdf(x, y-1)), hasLinearArtifact(BaseArtifactClassifier(vSpan, protectedFlag), cm, c, b))) ||
                (x < sdf.width-1 && ((r = sdf(x+1, y)), hasLinearArtifact(BaseArtifactClassifier(hSpan, protectedFlag), cm, c, r))) ||
                (y < sdf.height-1 && ((t = sdf(x, y+1)), hasLinearArtifact(BaseArtifactClassifier(vSpan, protectedFlag), cm, c, t))) ||
                (x > 0 && y > 0 && hasDiagonalArtifact(BaseArtifactClassifier(dSpan, protectedFlag), cm, c, l, b, sdf(x-1, y-1))) ||
                (x < sdf.width-1 && y > 0 && hasDiagonalArtifact(BaseArtifactClassifier(dSpan, protectedFlag), cm, c, r, b, sdf(x+1, y-1))) ||
                (x > 0 && y < sdf.height-1 && hasDiagonalArtifact(BaseArtifactClassifier(dSpan, protectedFlag), cm, c, l, t, sdf(x-1, y+1))) ||
                (x < sdf.width-1 && y < sdf.height-1 && hasDiagonalArtifact(BaseArtifactClassifier(dSpan, protectedFlag), cm, c, r, t, sdf(x+1, y+1)))
            ));
        }
    }
}

template <template <typename> class ContourCombiner, int N>
void MSDFErrorCorrection::findErrors(const BitmapConstRef<float, N> &sdf, const Shape &shape) {
    // Compute the expected deltas between values of horizontally, vertically, and diagonally adjacent texels.
    double hSpan = minDeviationRatio*transformation.unprojectVector(Vector2(transformation.distanceMapping(DistanceMapping::Delta(1)), 0)).length();
    double vSpan = minDeviationRatio*transformation.unprojectVector(Vector2(0, transformation.distanceMapping(DistanceMapping::Delta(1)))).length();
    double dSpan = minDeviationRatio*transformation.unprojectVector(Vector2(transformation.distanceMapping(DistanceMapping::Delta(1)))).length();
    {
        ShapeDistanceChecker<ContourCombiner, N> shapeDistanceChecker(sdf, shape, transformation, transformation.distanceMapping, minImproveRatio);
        bool rightToLeft = false;
        // Inspect all texels.
        for (int y = 0; y < sdf.height; ++y) {
            int row = shape.inverseYAxis ? sdf.height-y-1 : y;
            for (int col = 0; col < sdf.width; ++col) {
                int x = rightToLeft ? sdf.width-col-1 : col;
                if ((*stencil(x, row)&ERROR))
                    continue;
                const float *c = sdf(x, row);
                shapeDistanceChecker.shapeCoord = transformation.unproject(Point2(x+.5, y+.5));
                shapeDistanceChecker.sdfCoord = Point2(x+.5, row+.5);
                shapeDistanceChecker.msd = c;
                shapeDistanceChecker.protectedFlag = (*stencil(x, row)&PROTECTED) != 0;
                float cm = median(c[0], c[1], c[2]);
                const float *l = NULL, *b = NULL, *r = NULL, *t = NULL;
                // Mark current texel c with the error flag if an artifact occurs when it's interpolated with any of its 8 neighbors.
                *stencil(x, row) |= (byte) (ERROR*(
                    (x > 0 && ((l = sdf(x-1, row)), hasLinearArtifact(shapeDistanceChecker.classifier(Vector2(-1, 0), hSpan), cm, c, l))) ||
                    (row > 0 && ((b = sdf(x, row-1)), hasLinearArtifact(shapeDistanceChecker.classifier(Vector2(0, -1), vSpan), cm, c, b))) ||
                    (x < sdf.width-1 && ((r = sdf(x+1, row)), hasLinearArtifact(shapeDistanceChecker.classifier(Vector2(+1, 0), hSpan), cm, c, r))) ||
                    (row < sdf.height-1 && ((t = sdf(x, row+1)), hasLinearArtifact(shapeDistanceChecker.classifier(Vector2(0, +1), vSpan), cm, c, t))) ||
                    (x > 0 && row > 0 && hasDiagonalArtifact(shapeDistanceChecker.classifier(Vector2(-1, -1), dSpan), cm, c, l, b, sdf(x-1, row-1))) ||
                    (x < sdf.width-1 && row > 0 && hasDiagonalArtifact(shapeDistanceChecker.classifier(Vector2(+1, -1), dSpan), cm, c, r, b, sdf(x+1, row-1))) ||
                    (x > 0 && row < sdf.height-1 && hasDiagonalArtifact(shapeDistanceChecker.classifier(Vector2(-1, +1), dSpan), cm, c, l, t, sdf(x-1, row+1))) ||
                    (x < sdf.width-1 && row < sdf.height-1 && hasDiagonalArtifact(shapeDistanceChecker.classifier(Vector2(+1, +1), dSpan), cm, c, r, t, sdf(x+1, row+1)))
                ));
            }
        }
    }
}

template <int N>
void MSDFErrorCorrection::apply(const BitmapRef<float, N> &sdf) const {
    int texelCount = sdf.width*sdf.height;
    const byte *mask = stencil.pixels;
    float *texel = sdf.pixels;
    for (int i = 0; i < texelCount; ++i) {
        if (*mask&ERROR) {
            // Set all color channels to the median.
            float m = median(texel[0], texel[1], texel[2]);
            texel[0] = m, texel[1] = m, texel[2] = m;
        }
        ++mask;
        texel += N;
    }
}

BitmapConstRef<byte, 1> MSDFErrorCorrection::getStencil() const {
    return stencil;
}

template void MSDFErrorCorrection::protectEdges(const BitmapConstRef<float, 4> &sdf);
template void MSDFErrorCorrection::findErrors(const BitmapConstRef<float, 4> &sdf);
template void MSDFErrorCorrection::findErrors<SimpleContourCombiner>(const BitmapConstRef<float, 4> &sdf, const Shape &shape);
template void MSDFErrorCorrection::findErrors<OverlappingContourCombiner>(const BitmapConstRef<float, 4> &sdf, const Shape &shape);
template void MSDFErrorCorrection::apply(const BitmapRef<float, 4> &sdf) const;

template <typename DistanceType>
class DistancePixelConversion;

template <>
class DistancePixelConversion<double> {
    DistanceMapping mapping;
public:
    typedef BitmapRef<float, 1> BitmapRefType;
    inline explicit DistancePixelConversion(DistanceMapping mapping) : mapping(mapping) { }
    inline void operator()(float *pixels, double distance) const {
        *pixels = float(mapping(distance));
    }
};

template <>
class DistancePixelConversion<MultiDistance> {
    DistanceMapping mapping;
public:
    typedef BitmapRef<float, 3> BitmapRefType;
    inline explicit DistancePixelConversion(DistanceMapping mapping) : mapping(mapping) { }
    inline void operator()(float *pixels, const MultiDistance &distance) const {
        pixels[0] = float(mapping(distance.r));
        pixels[1] = float(mapping(distance.g));
        pixels[2] = float(mapping(distance.b));
    }
};

template <>
class DistancePixelConversion<MultiAndTrueDistance> {
    DistanceMapping mapping;
public:
    typedef BitmapRef<float, 4> BitmapRefType;
    inline explicit DistancePixelConversion(DistanceMapping mapping) : mapping(mapping) { }
    inline void operator()(float *pixels, const MultiAndTrueDistance &distance) const {
        pixels[0] = float(mapping(distance.r));
        pixels[1] = float(mapping(distance.g));
        pixels[2] = float(mapping(distance.b));
        pixels[3] = float(mapping(distance.a));
    }
};

template <class ContourCombiner>
void generateDistanceField(const typename DistancePixelConversion<typename ContourCombiner::DistanceType>::BitmapRefType &output, const Shape &shape, const SDFTransformation &transformation) {
    DistancePixelConversion<typename ContourCombiner::DistanceType> distancePixelConversion(transformation.distanceMapping);
    {
        ShapeDistanceFinder<ContourCombiner> distanceFinder(shape);
        bool rightToLeft = false;
        for (int y = 0; y < output.height; ++y) {
            int row = shape.inverseYAxis ? output.height-y-1 : y;
            for (int col = 0; col < output.width; ++col) {
                int x = rightToLeft ? output.width-col-1 : col;
                Point2 p = transformation.unproject(Point2(x+.5, y+.5));
                typename ContourCombiner::DistanceType distance = distanceFinder.distance(p);
                distancePixelConversion(output(x, row), distance);
            }
            rightToLeft = !rightToLeft;
        }
    }
}

void generateMTSDF(const BitmapRef<float, 4> &output, const Shape &shape, const SDFTransformation &transformation, const MSDFGeneratorConfig &config) {
    if (config.overlapSupport)
        generateDistanceField<OverlappingContourCombiner<MultiAndTrueDistanceSelector> >(output, shape, transformation);
    else
        generateDistanceField<SimpleContourCombiner<MultiAndTrueDistanceSelector> >(output, shape, transformation);
    msdfErrorCorrection(output, shape, transformation, config);
}

}

#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic pop
#elif defined(_MSC_VER)
#pragma warning(pop)
#endif
