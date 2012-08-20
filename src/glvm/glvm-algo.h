/*
 * C++ vector and matrix classes that resemble GLSL style.
 *
 * Copyright (C) 2009, 2010, 2011, 2012
 * Martin Lambers <marlam@marlam.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * This file provides a collection of geometry related algorithms based on the
 * GLVM data types.
 */

#ifndef GLVM_ALGO_H
#define GLVM_ALGO_H

#include <cmath>
#include <vector>

#include "glvm.h"


namespace glvm
{
    /*
     * Morton numbers, for Z-order curves.
     * This assumes that no overflow happens, i.e. that x and y only use half of their bit range.
     */
    template<typename T> T _morton(T x, T y)
    {
        T z = 0;
        for (T i = 0; i < sizeof(T) * CHAR_BIT / 2; i++) {
            z |= (x & 1 << i) << i | (y & 1 << i) << (i + 1);
        }
        return z;
    }
    inline unsigned char morton(unsigned char x, unsigned char y) { return _morton(x, y); }
    inline unsigned short morton(unsigned short x, unsigned short y) { return _morton(x, y); }
    inline unsigned int morton(unsigned int x, unsigned int y) { return _morton(x, y); }
    inline unsigned long morton(unsigned long x, unsigned long y) { return _morton(x, y); }
    inline unsigned long long morton(unsigned long long x, unsigned long long y) { return _morton(x, y); }
    /*
     * Inverse morton numbers, for Z-order curves.
     */
    template<typename T> T _get_odd_bits(T z)
    {
        T o = 0;
        for (T i = 0; i < static_cast<T>(sizeof(T) * CHAR_BIT / 2); i++) {
            o |= ((z >> (2 * i)) & 1) << i;
        }
        return o;
    }
    template<typename T> void _inv_morton(T z, T* x, T* y)
    {
        *y = _get_odd_bits(z);
        *x = _get_odd_bits(z >> 1);
    }
    inline void inv_morton(unsigned char z, unsigned char* x, unsigned char* y) { return _inv_morton(z, x, y); }
    inline void inv_morton(unsigned short z, unsigned short* x, unsigned short* y) { return _inv_morton(z, x, y); }
    inline void inv_morton(unsigned int z, unsigned int* x, unsigned int* y) { return _inv_morton(z, x, y); }
    inline void inv_morton(unsigned long z, unsigned long* x, unsigned long* y) { return _inv_morton(z, x, y); }
    inline void inv_morton(unsigned long long z, unsigned long long* x, unsigned long long* y) { return _inv_morton(z, x, y); }

    /*
     * Computes the minimal convex hull of the given set of 2D points.
     * The returned convex hull is ordered clockwise and starts with the
     * lowest of all leftmost points.
     */
    template<typename T>
    std::vector<vector<T, 2> > convex_hull(const std::vector<vector<T, 2> > &points)
    {
        std::vector<vector<T, 2> > ch;
        if (points.empty()) {
            return ch;
        }
        std::vector<vector<T, 2> > P = points;

        // Find start point (lowest x coordinate; if ambiguous, also lowest y coordinate)
        int start_point = 0;
        for (size_t i = 1; i < points.size(); i++) {
            if (points[i].x < points[start_point].x
                    || (equal(points[i].x, points[start_point].x, 0)
                        && points[i].y < points[start_point].y)) {
                start_point = i;
            }
        }

        // Jarvis march algorithm. See
        // http://en.wikipedia.org/wiki/Gift_wrapping_algorithm 20090824
        ch.push_back(P[start_point]);
        for (;;) {
            size_t i;
            for (i = 0; i < P.size(); i++) {
                if (all(equal(P[i], ch[0], 0))) {
                    continue;
                }

                vector<T, 2> p_i = P[i] - ch.back();
                bool all_points_on_right_side = true;
                for (size_t j = 0; j < P.size(); j++) {
                    if (j == i)
                        continue;

                    vector<T, 2> p_j = P[j] - ch.back();
                    bool pj_right_of_pi;
                    if (p_i.x > static_cast<T>(0)) {
                        T m = p_i.y / p_i.x;
                        pj_right_of_pi = (m * p_j.x - p_j.y >= static_cast<T>(0));
                    } else if (p_i.x < static_cast<T>(0)) {
                        T m = p_i.y / p_i.x;
                        pj_right_of_pi = (m * p_j.x - p_j.y <= static_cast<T>(0));
                    } else {
                        pj_right_of_pi = (p_i.y >= static_cast<T>(0)
                                ? p_j.x >= static_cast<T>(0)
                                : p_j.x <= static_cast<T>(0));
                    }
                    if (!pj_right_of_pi) {
                        all_points_on_right_side = false;
                        break;
                    }
                }
                if (all_points_on_right_side)
                    break;
            }
            // Test whether the new point lies on the line defined by the last
            // two convex hull points. If so, replace the last convex hull.
            bool replace_last_ch_point;
            if (ch.size() < 2) {
                replace_last_ch_point = false;
            } else {
                // If we have no more new points, test against the starting
                // point of the convex hull.
                vector<T, 2> Q = (i == P.size() ? ch[0] : P[i]);
                if (equal(ch[ch.size() - 1].x, ch[ch.size() - 2].x, 0)) {
                    replace_last_ch_point = equal(Q.x, ch[ch.size() - 1].x, 0);
                } else {
                    // Compute points relative to ch[ch.size() - 2]
                    vector<T, 2> p1 = ch[ch.size() - 1] - ch[ch.size() - 2];
                    vector<T, 2> p2 = Q - ch[ch.size() - 2];
                    T m = p1.y / p1.x;
                    replace_last_ch_point = equal(p2.x * m - p2.y, static_cast<T>(0), 0);
                }
            }
            if (replace_last_ch_point)
                ch.pop_back();
            if (i == P.size())
                break;
            ch.push_back(P[i]);
            P.erase(P.begin() + i);
        }
        return ch;
    }

    /*
     * Computes the area of the given non-self-intersecting polygon. The points
     * of the polygon must be ordered clockwise.
     */
    template<typename T>
    T polygon_area(const std::vector<vector<T, 2> > &polygon)
    {
        if (polygon.size() < 3) {
            return static_cast<T>(0);
        }

        T A = static_cast<T>(0);
        for (size_t i = 0; i < polygon.size() - 1; i++)
            A += polygon[i].x * polygon[i + 1].y - polygon[i + 1].x * polygon[i].y;
        A += polygon[polygon.size() - 1].x * polygon[0].y - polygon[0].x * polygon[polygon.size() - 1].y;
        A /= static_cast<T>(2);
        A = -A;         // because we went clockwise, not counterclockwise
        return A;
    }
}

#endif
