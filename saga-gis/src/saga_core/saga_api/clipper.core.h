/*******************************************************************************
* Author    :  Angus Johnson                                                   *
* Version   :  Clipper2 - ver.1.0.3                                            *
* Date      :  26 August 2022                                                  *
* Website   :  http://www.angusj.com                                           *
* Copyright :  Angus Johnson 2010-2022                                         *
* Purpose   :  Core Clipper Library structures and functions                   *
* License   :  http://www.boost.org/LICENSE_1_0.txt                            *
*******************************************************************************/

#ifndef CLIPPER_CORE_H
#define CLIPPER_CORE_H

#include <cstdlib>
#include <cmath>
#include <vector>
#include <string>
#include <iostream>
#include <algorithm>

namespace Clipper2Lib 
{

	static double const PI = 3.141592653589793238;

	//By far the most widely used filling rules for polygons are EvenOdd
	//and NonZero, sometimes called Alternate and Winding respectively.
	//https://en.wikipedia.org/wiki/Nonzero-rule
	enum class FillRule { EvenOdd, NonZero, Positive, Negative };
	
// Point ------------------------------------------------------------------------

template <typename T>
struct Point {
	T x;
	T y;
#ifdef USINGZ
	int64_t z;
#endif

	template <typename T2>
	inline void Init(const T2 x_ = 0, const T2 y_ = 0)
	{
		#pragma warning(suppress:4984)
		if constexpr (std::numeric_limits<T>::is_integer &&
			!std::numeric_limits<T2>::is_integer)
		{
			x = static_cast<T>(std::round(x_));
			y = static_cast<T>(std::round(y_));
		}
		else
		{
			x = static_cast<T>(x_);
			y = static_cast<T>(y_);
		}		
	}

#ifdef USINGZ

	explicit Point() : x(0), y(0), z(0) {};

	template <typename T2>
	explicit Point(const T2 x_= 0, const T2 y_ = 0, const int64_t z_ = 0)
	{
		Init(x_, y_);
		z = z_;
	}

	template <typename T2>
	explicit Point<T>(const Point<T2>& p) 
	{ 
		Init(p.x, p.y);
		z = 0;
	}

	Point operator * (const double scale) const
	{
		return Point(x * scale, y * scale, z);
	}


	friend std::ostream& operator<<(std::ostream& os, const Point& point)
	{
		os << point.x << "," << point.y << "," << point.z;
		return os;
	}

#else

	explicit Point() : x(0), y(0) {};

	template <typename T2>
	explicit Point(const T2 x_ = 0, const T2 y_ = 0) { Init(x_, y_); }

	template <typename T2>
	explicit Point<T>(const Point<T2>& p) { Init(p.x, p.y); }

	Point operator * (const double scale) const
	{
		return Point(x * scale, y * scale);
	}

	friend std::ostream& operator<<(std::ostream& os, const Point& point)
	{
		os << point.x << "," << point.y << " ";
		return os;
	}

#endif

	friend bool operator==(const Point &a, const Point &b) 
	{
		return a.x == b.x && a.y == b.y;
	}

	friend bool operator!=(const Point& a, const Point& b)
	{
		return !(a == b);
	}

	inline Point<T> operator-() const
	{
		return Point<T>(-x,-y);
	}

	inline Point operator+(const Point &b) const
	{
		return Point(x+b.x, y+b.y);
	}

	inline Point operator-(const Point &b) const
	{
		return Point(x-b.x, y-b.y);
	}

};

//nb: using 'using' here (instead of typedef) as they can be used in templates
using Point64 = Point<int64_t>;
using PointD = Point<double>;

template <typename T>
using Path = std::vector<Point<T>>;
template <typename T>
using Paths = std::vector<Path<T>>;

using Path64 = Path<int64_t>;
using PathD = Path<double>;
using Paths64 = std::vector< Path64>;
using PathsD = std::vector< PathD>;

template <typename T>
std::ostream& operator << (std::ostream& outstream, const Path<T>& path)
{
	if (!path.empty())
	{
		auto pt = path.cbegin(), last = path.cend() - 1;
		while (pt != last)
			outstream << *pt++ << ", ";
		outstream << *last << std::endl;
	}
	return outstream;
}

template <typename T>
std::ostream& operator << (std::ostream& outstream, const Paths<T>& paths)
{
	for (auto p : paths)
		outstream << p;
	return outstream;
}

template <typename T1, typename T2>
inline Path<T1> ScalePath(const Path<T2>& path, double scale)
{
	Path<T1> result;
	result.reserve(path.size());
#ifdef USINGZ
	for (const Point<T2>& pt : path)
		result.push_back(Point<T1>(pt.x * scale, pt.y * scale, pt.z));
#else
	for (const Point<T2>& pt : path)
		result.push_back(Point<T1>(pt.x * scale, pt.y * scale));
#endif
	return result;
}

template <typename T1, typename T2>
inline Paths<T1> ScalePaths(const Paths<T2>& paths, double scale)
{
	Paths<T1> result;
	result.reserve(paths.size());
	for (const Path<T2>& path : paths)
		result.push_back(ScalePath<T1, T2>(path, scale));
	return result;
}

template <typename T1, typename T2>
inline Path<T1> TransformPath(const Path<T2>& path)
{
	Path<T1> result;
	result.reserve(path.size());
	std::transform(path.cbegin(), path.cend(), std::back_inserter(result),
		[](const Point<T2>& pt) {return Point<T1>(pt); });
	return result;
}

template <typename T1, typename T2>
inline Paths<T1> TransformPaths(const Paths<T2>& paths)
{
	Paths<T1> result;
	std::transform(paths.cbegin(), paths.cend(), std::back_inserter(result),
		[](const Path<T2>& path) {return TransformPath<T1, T2>(path); });
	return result;
}

inline PathD Path64ToPathD(const Path64& path)
{
	return TransformPath<double, int64_t>(path);
}

inline PathsD Paths64ToPathsD(const Paths64& paths)
{
	return TransformPaths<double, int64_t>(paths);
}

inline Path64 PathDToPath64(const PathD& path)
{
	return TransformPath<int64_t, double>(path);
}

inline Paths64 PathsDToPaths64(const PathsD& paths)
{
	return TransformPaths<int64_t, double>(paths);
}

template<typename T>
inline double Sqr(T val)
{
	return static_cast<double>(val) * static_cast<double>(val);
}

template<typename T>
inline bool NearEqual(const Point<T>& p1,
	const Point<T>& p2, double max_dist_sqrd)
{
	return Sqr(p1.x - p2.x) + Sqr(p1.y - p2.y) < max_dist_sqrd;
}

template<typename T>
inline Path<T> StripNearEqual(const Path<T>& path, 
	double max_dist_sqrd, bool is_closed_path)
{
	if (path.size() == 0) return Path<T>();
	Path<T> result;
	result.reserve(path.size());
	typename Path<T>::const_iterator path_iter = path.cbegin();
	Point<T> first_pt = *path_iter++, last_pt = first_pt;
	result.push_back(first_pt);
	for (; path_iter != path.cend(); ++path_iter)
	{
		if (!NearEqual(*path_iter, last_pt, max_dist_sqrd))
		{
			last_pt = *path_iter;
			result.push_back(last_pt);
		}
	}
	if (!is_closed_path) return result;
	while (result.size() > 1 &&
		NearEqual(result.back(), first_pt, max_dist_sqrd)) result.pop_back();
	return result;
}

template<typename T>
inline Paths<T> StripNearEqual(const Paths<T>& paths, 
	double max_dist_sqrd, bool is_closed_path)
{
	Paths<T> result;
	result.reserve(paths.size());
	for (typename Paths<T>::const_iterator paths_citer = paths.cbegin();
		paths_citer != paths.cend(); ++paths_citer)
	{
		result.push_back(StripNearEqual(*paths_citer, max_dist_sqrd, is_closed_path));
	}
	return result;
}

template<typename T>
inline Path<T> StripDuplicates(const Path<T>& path, bool is_closed_path)
{
	if (path.size() == 0) return Path<T>();
	Path<T> result;
	result.reserve(path.size());
	typename Path<T>::const_iterator path_iter = path.cbegin();
	Point<T> first_pt = *path_iter++, last_pt = first_pt;
	result.push_back(first_pt);
	for (; path_iter != path.cend(); ++path_iter)
	{
		if (*path_iter != last_pt)
		{
			last_pt = *path_iter;
			result.push_back(last_pt);
		}
	}
	if (!is_closed_path) return result;
	while (result.size() > 1 && result.back() == first_pt) result.pop_back();
	return result;
}

template<typename T>
inline Paths<T> StripDuplicates(const Paths<T>& paths, bool is_closed_path)
{
	Paths<T> result;
	result.reserve(paths.size());
	for (typename Paths<T>::const_iterator paths_citer = paths.cbegin();
		paths_citer != paths.cend(); ++paths_citer)
	{
		result.push_back(StripDuplicates(*paths_citer, is_closed_path));
	}
	return result;
}

// Rect ------------------------------------------------------------------------

template <typename T>
struct Rect;

using Rect64 = Rect<int64_t>;
using RectD = Rect<double>;

template <typename T>
struct Rect {
	T left;
	T top;
	T right;
	T bottom;

	Rect() :
		left(0),
		top(0),
		right(0),
		bottom(0) {}

	Rect(T l, T t, T r, T b) :
		left(l),
		top(t),
		right(r),
		bottom(b) {}


	T Width() const { return right - left; }
	T Height() const { return bottom - top; }
	void Width(T width) { right = left + width; }
	void Height(T height) { bottom = top + height; }

	Point<T> MidPoint() const
	{
		return Point<T>((left + right) / 2, (top + bottom) / 2);
	}

	bool Contains(const Point<T> pt)
	{
		return pt.x > left && pt.x < right&& pt.y > top && pt.y < bottom;
	}

	bool Contains(const Rect<T> rec)
	{
		return rec.left >= left && rec.right <= right && 
			rec.top >= top && rec.bottom <= bottom;
	}

	void Scale(double scale) {
		left *= scale; 
		top *= scale;
		right *= scale;
		bottom *= scale;
	}

	bool IsEmpty() const { return bottom <= top || right <= left; };

	friend std::ostream &operator<<(std::ostream &os, const Rect<T> &rect) {
		os << "("
		   << rect.left << "," << rect.top << "," << rect.right << "," << rect.bottom
		   << ")";
		return os;
	}
};

// clipper2Exception ---------------------------------------------------------

class Clipper2Exception : public std::exception {
public:
	explicit Clipper2Exception(const char *description) :
		m_descr(description) {}
	virtual const char *what() const throw() { return m_descr.c_str(); }

private:
	std::string m_descr;
};

// Miscellaneous ------------------------------------------------------------

template <typename T>
inline double CrossProduct(const Point<T>& pt1, const Point<T>& pt2, const Point<T>& pt3) {
	return (static_cast<double>(pt2.x - pt1.x) * static_cast<double>(pt3.y - 
		pt2.y) - static_cast<double>(pt2.y - pt1.y) * static_cast<double>(pt3.x - pt2.x));
}

template <typename T>
inline double DotProduct(const Point<T>& pt1, const Point<T>& pt2, const Point<T>& pt3) {
	return (static_cast<double>(pt2.x - pt1.x) * static_cast<double>(pt3.x - pt2.x) + 
		static_cast<double>(pt2.y - pt1.y) * static_cast<double>(pt3.y - pt2.y));
}

template <typename T>
inline double DotProduct(const Point<T>& vec1, const Point<T>& vec2)
{
	return static_cast<double>(vec1.x * vec2.x) + static_cast<double>(vec1.y * vec2.y);
}

template <typename T>
inline double DistanceSqr(const Point<T> pt1, const Point<T> pt2)
{
	return Sqr(pt1.x - pt2.x) + Sqr(pt1.y - pt2.y);
}

template <typename T>
inline double DistanceFromLineSqrd(const Point<T>& pt, const Point<T>& ln1, const Point<T>& ln2)
{
	//perpendicular distance of point (x³,y³) = (Ax³ + By³ + C)/Sqrt(A² + B²)
	//see http://en.wikipedia.org/wiki/Perpendicular_distance
	double A = static_cast<double>(ln1.y - ln2.y);
	double B = static_cast<double>(ln2.x - ln1.x);
	double C = A * ln1.x + B * ln1.y;
	C = A * pt.x + B * pt.y - C;
	return (C * C) / (A * A + B * B);
}

template <typename T>
inline double Area(const Path<T>& path)
{
	size_t cnt = path.size();
	if (cnt < 3) return 0.0;
	double a = 0.0;
	typename Path<T>::const_iterator it1, it2 = path.cend() - 1, stop = it2;
	if (!(cnt & 1)) ++stop;
	for (it1 = path.cbegin(); it1 != stop;)
	{
		a += static_cast<double>(it2->y + it1->y) * (it2->x - it1->x);
		it2 = it1 + 1;
		a += static_cast<double>(it1->y + it2->y) * (it1->x - it2->x);
		it1 += 2;
	}
	if (cnt & 1)
		a += static_cast<double>(it2->y + it1->y) * (it2->x - it1->x);
	return a * 0.5;
}

template <typename T>
inline double Area(const Paths<T>& paths)
{
	double a = 0.0;
	for (typename Paths<T>::const_iterator paths_iter = paths.cbegin();
		paths_iter != paths.cend(); ++paths_iter)
	{
		a += Area<T>(*paths_iter);
	}
	return a;
}

template <typename T>
inline bool IsPositive(const Path<T>& poly)
{
	// A curve has positive orientation [and area] if a region 'R' 
	// is on the left when traveling around the outside of 'R'.
	//https://mathworld.wolfram.com/CurveOrientation.html
	//nb: This statement is premised on using Cartesian coordinates
	return Area<T>(poly) >= 0;
}


enum class PointInPolygonResult { IsOn, IsInside, IsOutside };

template <typename T>
inline PointInPolygonResult PointInPolygon(const Point<T>& pt, const Path<T>& polygon)
{
	if (polygon.size() < 3)
		return PointInPolygonResult::IsOutside;

	int val = 0;
	typename Path<T>::const_iterator start = polygon.cbegin(), cit = start;
	typename Path<T>::const_iterator cend = polygon.cend(), pit = cend - 1;

	while (pit->y == pt.y)
	{
		if (pit == start) return PointInPolygonResult::IsOutside;
		--pit;
	}
	bool is_above = pit->y < pt.y;

	while (cit != cend)
	{
		if (is_above)
		{
			while (cit != cend && cit->y < pt.y) ++cit;
			if (cit == cend) break;
		}
		else
		{
			while (cit != cend && cit->y > pt.y) ++cit;
			if (cit == cend) break;
		}

		if (cit == start) pit = cend - 1;
		else  pit = cit - 1;

		if (cit->y == pt.y)
		{
			if (cit->x == pt.x || (cit->y == pit->y &&
				((pt.x < pit->x) != (pt.x < cit->x))))
				return PointInPolygonResult::IsOn;
			++cit;
			continue;
		}

		if (pt.x < cit->x && pt.x < pit->x)
		{
			// we're only interested in edges crossing on the left
		}
		else if (pt.x > pit->x && pt.x > cit->x)
			val = 1 - val; // toggle val
		else
		{
			double d = CrossProduct(*pit, *cit, pt);
			if (d == 0) return PointInPolygonResult::IsOn;
			if ((d < 0) == is_above) val = 1 - val;
		}
		is_above = !is_above;
		++cit;
	}
	return (val == 0) ? 
		PointInPolygonResult::IsOutside :
		PointInPolygonResult::IsInside;
}

}  // namespace

#endif  // CLIPPER_CORE_H
