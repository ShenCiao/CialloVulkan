#pragma once

#include <cassert>
#include <limits>
#include <array>
#include <CGAL/Polynomial.h>
#include <boost/math/tools/roots.hpp>

namespace ciallo::geom
{
	template <size_t N>
	class Bezier
	{
		using Polynomial = CGAL::Polynomial<float>;

		class BinomialCoefficients
		{
		public:
			BinomialCoefficients()
			{
				auto binomial = [](size_t n, size_t k)
				{
					assert(k <= n);
					size_t val = 1;
					for (size_t i = 1; i <= k; i++)
					{
						val *= n + 1 - i;
						val /= i;
					}
					return val;
				};

				size_t center = N / 2;
				size_t k = 0;

				while (k <= center)
				{
					mCoefficients[k] = binomial(N, k);
					k++;
				}

				// Utilize the symmetrical nature of the binomial coefficients.
				while (k <= N)
				{
					mCoefficients[k] = mCoefficients[N - k];
					k++;
				}
			}

			static constexpr size_t size()
			{
				return N + 1;
			}

			size_t operator [](size_t idx) const
			{
				assert(idx < size());
				return mCoefficients[idx];
			}

		private:
			size_t mCoefficients[size()]{0};
		};

		const static inline BinomialCoefficients binomialCoefficients{};

		std::array<Polynomial, 2> m_bezierPoly; // Cache object for fast compute on t, not been lazy evaluated for now.
		std::array<Point, N + 1> m_controlPoints = {};

		void updatePolynomial()
		{
			Polynomial t{0.0f, 1.0f}; // Variable of the function

			using CGAL::ipower;
			Polynomial fx, fy;
			for (size_t i = 0; i < N + 1; ++i)
			{
				auto coefficient = static_cast<float>(binomialCoefficients[i]) *
					ipower(t, static_cast<int>(i)) * ipower(1.0f - t, static_cast<int>(N - i));
				fx += coefficient * m_controlPoints[i].x();
				fy += coefficient * m_controlPoints[i].y();
			}
			m_bezierPoly[0] = fx;
			m_bezierPoly[1] = fy;
		}

		void setControlPoints(point_iter auto it, point_iter auto end)
		{
			size_t s = end - it;
			assert(s == N+1);
			for (size_t i = 0; i < s; ++i)
			{
				m_controlPoints[i] = it[i];
			}
		}

	public:
		Bezier()
		{
			for (size_t i = 0; i < N + 1; i++)
			{
				m_controlPoints[i] = Point{0.0f, 0.0f};
			}
			updatePolynomial();
		}

		Bezier(point_iter auto it, point_iter auto end)
		{
			setControlPoints(it, end);
			updatePolynomial();
		}

		size_t order() const
		{
			return N;
		}

		float operator()(float t, int axis) const
		{
			assert(axis < 2);
			return m_bezierPoly[axis].evaluate(t);
		}

		Point operator()(float t) const
		{
			return Point{operator()(t, 0), operator()(t, 1)};
		}

		friend std::ostream& operator<<(std::ostream& os, const Bezier<N>& curve)
		{
			for (size_t i = 0; i < N + 1; ++i)
			{
				os << curve.m_controlPoints[i] << "  ";
			}
			return os;
		}

		std::array<Bezier<N>, 2> split(float t) const
		{
			std::array<Point, N + 1> l;
			std::array<Point, N + 1> r;
			l[0] = m_controlPoints[0];
			r[0] = m_controlPoints[N];

			std::array<Point, N + 1> prev = m_controlPoints;
			std::array<Point, N + 1> curr;

			size_t subs = 0;
			while (subs < N)
			{
				for (size_t i = 0; i < N - subs; i++)
				{
					float x = (1.0f - t) * prev[i].x() + t * prev[i + 1].x();
					float y = (1.0f - t) * prev[i].y() + t * prev[i + 1].y();
					curr[i] = Point{x, y};

					if (i == 0)
						l[subs + 1] = curr[i];
					if (i == (N - subs - 1))
						r[subs + 1] = curr[i];
				}
				std::swap(prev, curr);
				subs++;
			}

			std::reverse(r.begin(), r.end());

			Bezier<N> left{l.begin(), l.end()};
			Bezier<N> right{r.begin(), r.end()};

			return {left, right};
		}


		Bezier operator+(Vector v)
		{
			Bezier curve = *this;
			return curve += v;
		}

		/**
		 * \brief Find the t value at given value.
		 * Only within range 0.0-1.0. Only find one value closer(?) to 0.0. May need change.
		 * \param given The given value.
		 * \param axis Axis to query.
		 * \return T value.
		 */
		float findT(float given, int axis = 0)
		{
			using boost::math::tools::newton_raphson_iterate;

			auto target = [given, axis, this](float var)
			{
				Polynomial diff = m_bezierPoly[axis] - given;
				diff.diff();
				return std::make_pair(m_bezierPoly[axis].evaluate(var) - given, diff.evaluate(var));
			};

			const int digits = std::numeric_limits<float>::digits;
			boost::uintmax_t maxIt = 100;
			float result = newton_raphson_iterate(target, 0.0f, 0.0f, 1.0f, digits, maxIt);

			if (std::abs(target(result).first) >= 1e-2f)
			{
				throw std::range_error("Can not find the desired value!");
			}
			return result;
		}

		std::array<std::array<float, N + 1>, 2> polynomialCoefficients()
		{
			return {polynomialCoefficients(0), polynomialCoefficients(1)};
		}

		std::array<float, N + 1> polynomialCoefficients(int axis)
		{
			assert(axis < 2);
			std::array<float, N + 1> co;
			for (size_t i = 0; i < N + 1; ++i)
			{
				co[i] = CGAL::get_coefficient(m_bezierPoly[axis], i);
			}
			return co;
		}
	};
}
