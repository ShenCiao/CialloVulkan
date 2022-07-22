#pragma once
// -----------------------------------------------------------------------------
// Being copied from https://github.com/oysteinmyrmo/bezier
// Change geometry primitives and only keep functions currently in used.
// -----------------------------------------------------------------------------
#include <cassert>
#include <cmath>
#include <limits>
#include <algorithm>
#include <array>


namespace ciallo::geom
{
	template <size_t N>
	// Should be the same signature as https://doc.cgal.org/latest/Arrangement_on_surface_2/classCGAL_1_1Arr__Bezier__curve__traits__2_1_1Curve__2.html
	class Bezier
	{
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

		struct PolynomialPair
		{
			size_t t = 0;
			size_t one_minus_t = 0;

			float valueAt(float t) const
			{
				return static_cast<float>(pow(1.0f - t, one_minus_t) * pow(t, static_cast<float>(this->t)));
			}
		};

		class PolynomialCoefficients
		{
		public:
			PolynomialCoefficients()
			{
				for (size_t i = 0; i <= N; i++)
				{
					mPolynomialPairs[i].t = i;
					mPolynomialPairs[i].one_minus_t = N - i;
					assert(mPolynomialPairs[i].t + mPolynomialPairs[i].one_minus_t == N);
				}
			}

			float valueAt(size_t pos, float t) const
			{
				assert(pos < size());
				return mPolynomialPairs[pos].valueAt(t);
			}

			static constexpr size_t size()
			{
				return N + 1;
			}

			const PolynomialPair& operator [](size_t idx) const
			{
				assert(idx < size());
				return mPolynomialPairs[idx];
			}

		private:
			PolynomialPair mPolynomialPairs[size()];
		};

		const static inline BinomialCoefficients binomialCoefficients{};
		const static inline PolynomialCoefficients polynomialCoefficients{};
		std::array<Point, N + 1> mControlPoints = {};

	public:
		Bezier()
		{
			for (size_t i = 0; i < N+1; i++)
			{
				mControlPoints[i] = Point{0.0f, 0.0f};
			}
		}

		Bezier(point_iter auto it, point_iter auto end)
		{
			size_t s = end - it;
			assert(s == N+1);
			for (size_t i = 0; i < s; ++i)
			{
				mControlPoints[i] = it[i];
			}
		}

		size_t order() const
		{
			return N;
		}

		float operator()(float t, int axis) const
		{
			assert(axis < 2);
			float sum = 0;
			for (size_t n = 0; n < N + 1; n++)
			{
				sum += binomialCoefficients[n] * polynomialCoefficients[n].valueAt(t) * mControlPoints[n][axis];
			}
			return sum;
		}

		Point operator()(float t) const
		{
			return Point{operator()(t, 0), operator()(t, 1)};
		}

		friend std::ostream& operator<<(std::ostream& os, const Bezier<N>& curve)
		{
			for (size_t i = 0; i < N+1; ++i)
			{
				os << curve.mControlPoints[i] << "  ";
			}
			return os;
		}

		std::array<Bezier<N>, 2> split(float t) const
		{
			std::array<Point, N + 1> l;
			std::array<Point, N + 1> r;
			l[0] = mControlPoints[0];
			r[0] = mControlPoints[N];

			std::array<Point, N + 1> prev = mControlPoints;
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

			std::reverse(r.begin(), r.end()); //Warning: bad for performance

			Bezier<N> left{l.begin(), l.end()};
			Bezier<N> right{r.begin(), r.end()};

			return {left, right};
		}
	};
}
