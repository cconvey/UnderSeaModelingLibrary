/**
 * @file data_grid_bathy.h
 * Fast non-recursive 2D interpolation algorithm for bathymetry.
 */
#pragma once

#include <usml/types/data_grid.h>
#include <usml/types/seq_vector.h>
#include <usml/usml_config.h>

#include <algorithm>
#include <array>
#include <boost/numeric/ublas/expression_types.hpp>
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/matrix_expression.hpp>
#include <cmath>
#include <cstddef>
#include <memory>
#include <stdexcept>

namespace usml {
namespace types {

/// @ingroup data_grid
/// @{

/**
 * Implements fast calculations for 2D data_grids using a non-recursive
 * engine on interpolation. Assumes that both axes of the passed grid
 * have the same interp_type.
 *
 * Unlike the gen_grid class, this wrapper does not support modification of
 * the underlying data set.  It uses const shared pointers to reference the
 * data in the underlying data_grid.
 */
class USML_DECLSPEC data_grid_bathy : public data_grid<2> {
   public:
    /**
     * Creates a fast interpolation factors from an existing data_grid.
     * This included inverse bicubic coefficient matrix to be
     * used at a later time during pchip calculations.
     *
     * @param grid      The data_grid that is to be wrapped.
     */
    data_grid_bathy(data_grid<2>::csptr grid)
        : _kmin(0u),
          _k0max(grid->axis(0).size() - 1),
          _k1max(grid->axis(1).size() - 1) {
        // copy data from original grid

        for (size_t n = 0; n < 2; ++n) {
            this->_axis[n] = grid->axis_csptr(n);
            this->_interp_type[n] = grid->interp_type(n);
            this->_edge_limit[n] = grid->edge_limit(n);
        }
        this->_data = grid->data_csptr();

        // Construct the inverse bicubic interpolation coefficient matrix
        _inv_bicubic_coeff = zero_matrix<double>(16, 16);
        _inv_bicubic_coeff(0, 0) = 1;
        _inv_bicubic_coeff(1, 8) = 1;
        _inv_bicubic_coeff(2, 0) = -3;
        _inv_bicubic_coeff(2, 1) = 3;
        _inv_bicubic_coeff(2, 8) = -2;
        _inv_bicubic_coeff(2, 9) = -1;
        _inv_bicubic_coeff(3, 0) = 2;
        _inv_bicubic_coeff(3, 1) = -2;
        _inv_bicubic_coeff(3, 8) = _inv_bicubic_coeff(3, 9) = 1;
        _inv_bicubic_coeff(4, 4) = 1;
        _inv_bicubic_coeff(5, 12) = 1;
        _inv_bicubic_coeff(6, 4) = -3;
        _inv_bicubic_coeff(6, 5) = 3;
        _inv_bicubic_coeff(6, 12) = -2;
        _inv_bicubic_coeff(6, 13) = -1;
        _inv_bicubic_coeff(7, 4) = 2;
        _inv_bicubic_coeff(7, 5) = -2;
        _inv_bicubic_coeff(7, 12) = _inv_bicubic_coeff(7, 13) = 1;
        _inv_bicubic_coeff(8, 0) = -3;
        _inv_bicubic_coeff(8, 2) = 3;
        _inv_bicubic_coeff(8, 4) = -2;
        _inv_bicubic_coeff(8, 6) = -1;
        _inv_bicubic_coeff(9, 8) = -3;
        _inv_bicubic_coeff(9, 10) = 3;
        _inv_bicubic_coeff(9, 12) = -2;
        _inv_bicubic_coeff(9, 14) = -1;
        _inv_bicubic_coeff(10, 0) = _inv_bicubic_coeff(10, 3) = 9;
        _inv_bicubic_coeff(10, 1) = _inv_bicubic_coeff(10, 2) = -9;
        _inv_bicubic_coeff(10, 4) = _inv_bicubic_coeff(10, 8) = 6;
        _inv_bicubic_coeff(10, 5) = _inv_bicubic_coeff(10, 10) = -6;
        _inv_bicubic_coeff(10, 6) = _inv_bicubic_coeff(10, 9) = 3;
        _inv_bicubic_coeff(10, 7) = _inv_bicubic_coeff(10, 11) = -3;
        _inv_bicubic_coeff(10, 12) = 4;
        _inv_bicubic_coeff(10, 13) = _inv_bicubic_coeff(10, 14) = 2;
        _inv_bicubic_coeff(10, 15) = 1;
        _inv_bicubic_coeff(11, 0) = _inv_bicubic_coeff(11, 3) = -6;
        _inv_bicubic_coeff(11, 1) = _inv_bicubic_coeff(11, 2) = 6;
        _inv_bicubic_coeff(11, 6) = _inv_bicubic_coeff(11, 12) =
            _inv_bicubic_coeff(11, 13) = -2;
        _inv_bicubic_coeff(11, 4) = -4;
        _inv_bicubic_coeff(11, 5) = 4;
        _inv_bicubic_coeff(11, 7) = 2;
        _inv_bicubic_coeff(11, 8) = _inv_bicubic_coeff(11, 9) = -3;
        _inv_bicubic_coeff(11, 10) = _inv_bicubic_coeff(11, 11) = 3;
        _inv_bicubic_coeff(11, 14) = _inv_bicubic_coeff(11, 15) = -1;
        _inv_bicubic_coeff(12, 0) = 2;
        _inv_bicubic_coeff(12, 2) = -2;
        _inv_bicubic_coeff(12, 4) = _inv_bicubic_coeff(12, 6) = 1;
        _inv_bicubic_coeff(13, 8) = 2;
        _inv_bicubic_coeff(13, 10) = -2;
        _inv_bicubic_coeff(13, 12) = _inv_bicubic_coeff(13, 14) = 1;
        _inv_bicubic_coeff(14, 0) = _inv_bicubic_coeff(14, 3) = -6;
        _inv_bicubic_coeff(14, 1) = _inv_bicubic_coeff(14, 2) = 6;
        _inv_bicubic_coeff(14, 4) = _inv_bicubic_coeff(14, 6) = -3;
        _inv_bicubic_coeff(14, 5) = _inv_bicubic_coeff(14, 7) = 3;
        _inv_bicubic_coeff(14, 8) = -4;
        _inv_bicubic_coeff(14, 10) = 4;
        _inv_bicubic_coeff(14, 9) = _inv_bicubic_coeff(14, 12) =
            _inv_bicubic_coeff(14, 14) = -2;
        _inv_bicubic_coeff(14, 11) = 2;
        _inv_bicubic_coeff(14, 13) = _inv_bicubic_coeff(14, 15) = -1;
        _inv_bicubic_coeff(15, 0) = _inv_bicubic_coeff(15, 3) = 4;
        _inv_bicubic_coeff(15, 1) = _inv_bicubic_coeff(15, 2) = -4;
        _inv_bicubic_coeff(15, 4) = _inv_bicubic_coeff(15, 6) =
            _inv_bicubic_coeff(15, 8) = _inv_bicubic_coeff(15, 9) = 2;
        _inv_bicubic_coeff(15, 5) = _inv_bicubic_coeff(15, 7) =
            _inv_bicubic_coeff(15, 10) = _inv_bicubic_coeff(15, 11) = -2;
        _inv_bicubic_coeff(15, 12) = _inv_bicubic_coeff(15, 13) =
            _inv_bicubic_coeff(15, 14) = _inv_bicubic_coeff(15, 15) = 1;

        // Pre-construct increments for all intervals once to save time
        matrix<double> inc_x(_k0max + 1u, 1);
        for (size_t i = 0; i < _k0max + 1u; ++i) {
            if (i == 0 || i == _k0max) {
                inc_x(i, 0) = 2;
            } else {
                inc_x(i, 0) =
                    (axis(0).increment(i - 1) + axis(0).increment(i + 1)) /
                    axis(0).increment(i);
            }
        }
        matrix<double> inc_y(_k1max + 1u, 1);
        for (size_t i = 0; i < _k1max + 1u; ++i) {
            if (i == 0 || i == _k1max) {
                inc_y(i, 0) = 2;
            } else {
                inc_y(i, 0) =
                    (axis(1).increment(i - 1) + axis(1).increment(i + 1)) /
                    axis(1).increment(i);
            }
        }

        // Pre-construct all derivatives and cross-dervs once to save time
        _derv_x = matrix<double>(_k0max + 1u, _k1max + 1u);
        _derv_y = matrix<double>(_k0max + 1u, _k1max + 1u);
        _derv_x_y = matrix<double>(_k0max + 1u, _k1max + 1u);
        for (size_t i = 0; i < _k0max + 1u; ++i) {
            for (size_t j = 0; j < _k1max + 1u; ++j) {
                if (i < 1 && j < 1) {  // top-left corner
                    _derv_x(i, j) =
                        (data_2d(i + 1, j) - data_2d(i, j)) / inc_x(i, 0);
                    _derv_y(i, j) =
                        (data_2d(i, j + 1) - data_2d(i, j)) / inc_y(j, 0);
                    _derv_x_y(i, j) =
                        (data_2d(i + 1, j + 1) - data_2d(i + 1, j) -
                         data_2d(i, j + 1) + data_2d(i, j)) /
                        (inc_x(i, 0) * inc_y(j, 0));
                } else if (i == _k0max && j == _k1max) {  // bottom-right corner
                    _derv_x(i, j) =
                        (data_2d(i, j) - data_2d(i - 1, j)) / inc_x(i, 0);
                    _derv_y(i, j) =
                        (data_2d(i, j) - data_2d(i, j - 1)) / inc_y(j, 0);
                    _derv_x_y(i, j) =
                        (data_2d(i, j) - data_2d(i, j - 1) - data_2d(i - 1, j) +
                         data_2d(i - 1, j - 1)) /
                        (inc_x(i, 0) * inc_y(j, 0));
                } else if (i < 1 && j == _k1max) {  // top-right corner
                    _derv_x(i, j) =
                        (data_2d(i + 1, j) - data_2d(i, j)) / inc_x(i, 0);
                    _derv_y(i, j) =
                        (data_2d(i, j) - data_2d(i, j - 1)) / inc_y(j, 0);
                    _derv_x_y(i, j) =
                        (data_2d(i + 1, j) - data_2d(i + 1, j - 1) -
                         data_2d(i, j) + data_2d(i, j - 1)) /
                        (inc_x(i, 0) * inc_y(j, 0));
                } else if (j < 1 && i == _k0max) {  // bottom-left corner
                    _derv_x(i, j) =
                        (data_2d(i, j) - data_2d(i - 1, j)) / inc_x(i, 0);
                    _derv_y(i, j) =
                        (data_2d(i, j + 1) - data_2d(i, j)) / inc_y(j, 0);
                    _derv_x_y(i, j) =
                        (data_2d(i, j + 1) - data_2d(i, j) -
                         data_2d(i - 1, j + 1) + data_2d(i - 1, j)) /
                        (inc_x(i, 0) * inc_y(j, 0));
                } else if (i < 1 && (1 <= j && j < _k1max)) {  // top row
                    _derv_x(i, j) =
                        (data_2d(i + 1, j) - data_2d(i, j)) / inc_x(i, 0);
                    _derv_y(i, j) =
                        (data_2d(i, j + 1) - data_2d(i, j - 1)) / inc_y(j, 0);
                    _derv_x_y(i, j) =
                        (data_2d(i + 1, j + 1) - data_2d(i + 1, j - 1) -
                         data_2d(i, j + 1) + data_2d(i, j - 1)) /
                        (inc_x(i, 0) * inc_y(j, 0));
                } else if (j < 1 &&
                           (1 <= i && i < _k0max)) {  // left most column
                    _derv_x(i, j) =
                        (data_2d(i + 1, j) - data_2d(i - 1, j)) / inc_x(i, 0);
                    _derv_y(i, j) =
                        (data_2d(i, j + 1) - data_2d(i, j)) / inc_y(j, 0);
                    _derv_x_y(i, j) =
                        (data_2d(i + 1, j + 1) - data_2d(i + 1, j) -
                         data_2d(i - 1, j + 1) + data_2d(i - 1, j)) /
                        (inc_x(i, 0) * inc_y(j, 0));
                } else if (j == _k1max &&
                           (1 <= i && i < _k0max)) {  // right most column
                    _derv_x(i, j) =
                        (data_2d(i + 1, j) - data_2d(i - 1, j)) / inc_x(i, 0);
                    _derv_y(i, j) =
                        (data_2d(i, j) - data_2d(i, j - 1)) / inc_y(j, 0);
                    _derv_x_y(i, j) =
                        (data_2d(i + 1, j) - data_2d(i + 1, j - 1) -
                         data_2d(i - 1, j) + data_2d(i - 1, j - 1)) /
                        (inc_x(i, 0) * inc_y(j, 0));
                } else if (i == _k0max &&
                           (1 <= j && j < _k1max)) {  // bottom row
                    _derv_x(i, j) =
                        (data_2d(i, j) - data_2d(i - 1, j)) / inc_x(i, 0);
                    _derv_y(i, j) =
                        (data_2d(i, j + 1) - data_2d(i, j - 1)) / inc_y(j, 0);
                    _derv_x_y(i, j) =
                        (data_2d(i, j + 1) - data_2d(i, j - 1) -
                         data_2d(i - 1, j + 1) + data_2d(i - 1, j - 1)) /
                        (inc_x(i, 0) * inc_y(j, 0));
                } else {  // inside, restrictive
                    _derv_x(i, j) =
                        (data_2d(i + 1, j) - data_2d(i - 1, j)) / inc_x(i, 0);
                    _derv_y(i, j) =
                        (data_2d(i, j + 1) - data_2d(i, j - 1)) / inc_y(j, 0);
                    _derv_x_y(i, j) =
                        (data_2d(i + 1, j + 1) - data_2d(i + 1, j - 1) -
                         data_2d(i - 1, j + 1) + data_2d(i - 1, j - 1)) /
                        (inc_x(i, 0) * inc_y(j, 0));
                }
            }  // end for-loop in j
        }      // end for-loop in i
    }          // end constructor

    /**
     * Overrides the interpolate function within data_grid using the
     * non-recursive formula. Determines which interpolate function to
     * based on the interp_type enumeral stored within the 0th dimensional
     * axis.
     *
     * Interpolate at a single location.
     *
     * @param location   Location to do the interpolation at
     * @param derivative Derivative at the location (output)
     * @return           Returns the value at the field location
     */
    double interpolate(const double location[],
                       double* derivative = nullptr) const override {
        std::array<double, 2> loc;
        std::copy_n(location, 2, loc.begin());
        size_t offset[2];
        size_t fast_index[2];

        for (size_t dim = 0; dim < 2; ++dim) {
            const seq_vector& ax = axis(dim);

            // limit interpolation to axis domain if _edge_limit turned on

            if (edge_limit(dim)) {
                double a = ax(0);
                double b = ax(ax.size() - 2);
                double sign = (ax.increment(0) < 0) ? -1.0 : 1.0;
                double d = loc[dim] * sign;
                if (d <= a * sign) {  // left of the axis
                    loc[dim] = a;
                    offset[dim] = 0;
                } else if (d >= b * sign) {  // right of the axis
                    loc[dim] = b;
                    offset[dim] = axis(dim).size() - 2;
                } else {  // between end-points of axis
                    offset[dim] = axis(dim).find_index(loc[dim]);
                }

                // allow extrapolation if _edge_limit turned off

            } else {
                offset[dim] = axis(dim).find_index(loc[dim]);
            }
        }

        double result = 0.0;
        switch (interp_type(0)) {
            //****nearest****
            case interp_enum::nearest:
                for (int dim = 0; dim < 2; ++dim) {
                    double inc = axis(dim).increment(0);
                    double u = abs(loc[dim] - offset[dim]) / inc;
                    if (u < 0.5) {
                        fast_index[dim] = offset[dim];
                    } else {
                        fast_index[dim] = offset[dim] + 1;
                    }
                }
                if (derivative) {
                    derivative[0] = derivative[1] = 0;
                }
                result = data(fast_index);
                break;

            //****linear****
            case interp_enum::linear:
                double f11, f21, f12, f22, x_diff, y_diff;
                double x, x1, x2, y, y1, y2;

                x = loc[0];
                x1 = axis(0)(offset[0]);
                x2 = axis(0)(offset[0] + 1);
                y = loc[1];
                y1 = axis(1)(offset[1]);
                y2 = axis(1)(offset[1] + 1);
                f11 = data(offset);
                fast_index[0] = offset[0] + 1;
                fast_index[1] = offset[1];
                f21 = data(fast_index);
                fast_index[0] = offset[0];
                fast_index[1] = offset[1] + 1;
                f12 = data(fast_index);
                fast_index[0] = offset[0] + 1;
                fast_index[1] = offset[1] + 1;
                f22 = data(fast_index);
                x_diff = x2 - x1;
                y_diff = y2 - y1;
                result =
                    (f11 * (x2 - x) * (y2 - y) + f21 * (x - x1) * (y2 - y) +
                     f12 * (x2 - x) * (y - y1) + f22 * (x - x1) * (y - y1)) /
                    (x_diff * y_diff);
                if (derivative) {
                    derivative[0] = (f21 * (y2 - y) - f11 * (y2 - y) +
                                     f22 * (y - y1) - f12 * (y - y1)) /
                                    (x_diff * y_diff);
                    derivative[1] = (f12 * (x2 - x) - f11 * (x2 - x) +
                                     f22 * (x - x1) - f21 * (x - x1)) /
                                    (x_diff * y_diff);
                }
                break;

            //****pchip****
            case interp_enum::pchip:
                result = fast_pchip(offset, loc.data(), derivative);
                break;

            default:
                throw std::invalid_argument(
                    "Interp must be NEAREST, LINEAR, or PCHIP");
                break;
        }
        return result;
    }

    /**
     * Overrides the interpolate function within data_grid using the
     * non-recursive formula.
     *
     * Interpolate at a series of locations.
     *
     * @param   x           First dimension of location.
     * @param   y           Second dimension of location.
     * @param   result      Interpolated values at each location (output).
     * @param   dx          First dimension of derivative (output).
     * @param   dy          Second dimension of derivative (output).
     */

    void interpolate(const matrix<double>& x, const matrix<double>& y,
                     matrix<double>* result, matrix<double>* dx = nullptr,
                     matrix<double>* dy = nullptr) const {
        double location[2];
        double derivative[2];
        for (size_t n = 0; n < x.size1(); ++n) {
            for (size_t m = 0; m < x.size2(); ++m) {
                location[0] = x(n, m);
                location[1] = y(n, m);
                if (dx == nullptr || dy == nullptr) {
                    (*result)(n, m) = (double)interpolate(location);
                } else {
                    (*result)(n, m) = (double)interpolate(location, derivative);
                    (*dx)(n, m) = (double)derivative[0];
                    (*dy)(n, m) = (double)derivative[1];
                }
            }
        }
    }

   private:
    /** Utility accessor function for data grid values */
    inline double data_2d(size_t row, size_t col) {
        size_t grid_index[2];
        grid_index[0] = row;
        grid_index[1] = col;
        return data(grid_index);
    }

    /**
     * A non-recursive version of the Piecewise Cubic Hermite
     * polynomial (PCHIP) specific to the 2-dimensional grid of
     * data. This algorithm was generated from the below formula and
     * by performing a linear transformation from the data set to the
     * interval [0,1] x [0,1]:
     *
     *  g(x,y) = sum [i,0,3] (
     *                sum [j,0,3] (
     *                    a_ij * x^i * y^j
     *                )
     *           )
     *
     * where (x,y) is the point of interpolation and a_ij are known
     * as the bicubic interpolation coefficients. An inverse matrix
     * is then constructed from the 16 equations that are generated.
     * Using this inverse matrix and the 4 surround data points, their
     * respective derivatives with respect to x and y, and mixed xy
     * xy derivatives, we can construct each a_ij.
     *
     * The partial and mixed derivates are computed using a centered
     * differencing approximation, so that,
     *
     *      f_x(i,j) = [ f(i+1,j) - f(i-1,j) ] / [ x_(i+1) - x(i-1) ]
     *      f_y(i,j) = [ f(i,j+1) - f(i,j-1) ] / [ y_(i+1) - y(i-1) ]
     *      f_xy(i,j) = { f(i+1,j+1) - f(i+1,j-1) - f(i-1,j+1) +
     *                  f(i-1,j-1) } / [ x_(i+1) - x(i-1) ] *
     *                  [ y_(i+1) - y(i-1) ]
     *
     * @xref http://en.wikipedia.org/wiki/Bicubic_interpolation
     * @xref http://en.wikipedia.org/wiki/Finite_difference
     *
     * Below is a representation of the data extracted from the field
     * and how it is stored within the matrix:
     *
     *              * field(1,0)                  * field(3,0)
     *                      * (interp point)
     *
     *              * field(0,0)                  * field(2,0)
     *
     * field(0,0) to field(3,0) are the data points that surrounding the
     * interpolation point. field(4,0) to field(7,0) are the derivatives
     * with respect to x of the extracted data points. field(8,0) to
     * field(11,0) are the derivatives with respect to y of the extracted
     * data points. field(12,0) to field(15,0) are the derivatives with
     * respect to x and y of the extracted data points.
     *
     * @param interp_index  index on the grid for the closest data point
     * @param location      Location of the field calculation
     * @param derivative    Generate the derivative at the location (output)
     * @return              Returns the value at the field location
     */
    double fast_pchip(const size_t* interp_index, double* location,
                      double* derivative = nullptr) const {
        size_t k0 = interp_index[0];
        size_t k1 = interp_index[1];
        double norm0, norm1;
        size_t fast_index[2];
        c_matrix<double, 4, 4> value;

        // Checks for boundaries of the axes
        norm0 = axis(0).increment(k0);
        norm1 = axis(1).increment(k1);
        for (int i = -1; i < 3; ++i) {
            for (int j = -1; j < 3; ++j) {
                // get appropriate data when at boundaries
                if ((k0 + i) >= _k0max) {
                    fast_index[0] = _k0max;
                } else if ((k0 + i) <= _kmin) {
                    fast_index[0] = _kmin;
                } else {
                    fast_index[0] = k0 + i;
                }
                // get appropriate data when at boundaries
                if ((k1 + j) >= _k1max) {
                    fast_index[1] = _k1max;
                } else if ((k1 + j) <= _kmin) {
                    fast_index[1] = _kmin;
                } else {
                    fast_index[1] = k1 + j;
                }
                value(i + 1, j + 1) = data(fast_index);
            }  // end for-loop in j
        }      // end for-loop in i

        // Construct the field matrix
        c_matrix<double, 16, 1> field;
        field(0, 0) = value(1, 1);                 // f(0,0)
        field(1, 0) = value(1, 2);                 // f(0,1)
        field(2, 0) = value(2, 1);                 // f(1,0)
        field(3, 0) = value(2, 2);                 // f(1,1)
        field(4, 0) = _derv_x(k0, k1);             // f_x(0,0)
        field(5, 0) = _derv_x(k0, k1 + 1);         // f_x(0,1)
        field(6, 0) = _derv_x(k0 + 1, k1);         // f_x(1,0)
        field(7, 0) = _derv_x(k0 + 1, k1 + 1);     // f_x(1,1)
        field(8, 0) = _derv_y(k0, k1);             // f_y(0,0)
        field(9, 0) = _derv_y(k0, k1 + 1);         // f_y(0,1)
        field(10, 0) = _derv_y(k0 + 1, k1);        // f_y(1,0)
        field(11, 0) = _derv_y(k0 + 1, k1 + 1);    // f_y(1,1)
        field(12, 0) = _derv_x_y(k0, k1);          // f_x_y(0,0)
        field(13, 0) = _derv_x_y(k0, k1 + 1);      // f_x_y(0,1)
        field(14, 0) = _derv_x_y(k0 + 1, k1);      // f_x_y(1,0)
        field(15, 0) = _derv_x_y(k0 + 1, k1 + 1);  // f_x_y(1,1)

        // Construct the coefficients of the bicubic interpolation
        c_matrix<double, 16, 1> bicubic_coeff = prod(_inv_bicubic_coeff, field);

        // Create the power series of the interpolation formula before hand for
        // speed
        double x_inv = location[0] - axis(0)(k0);
        double y_inv = location[1] - axis(1)(k1);

        c_matrix<double, 1, 16> xyloc;
        xyloc(0, 0) = 1;
        xyloc(0, 1) = y_inv / norm1;
        xyloc(0, 2) = xyloc(0, 1) * xyloc(0, 1);
        xyloc(0, 3) = xyloc(0, 2) * xyloc(0, 1);
        xyloc(0, 4) = x_inv / norm0;
        xyloc(0, 5) = xyloc(0, 4) * xyloc(0, 1);
        xyloc(0, 6) = xyloc(0, 4) * xyloc(0, 2);
        xyloc(0, 7) = xyloc(0, 4) * xyloc(0, 3);
        xyloc(0, 8) = xyloc(0, 4) * xyloc(0, 4);
        xyloc(0, 9) = xyloc(0, 8) * xyloc(0, 1);
        xyloc(0, 10) = xyloc(0, 8) * xyloc(0, 2);
        xyloc(0, 11) = xyloc(0, 8) * xyloc(0, 3);
        xyloc(0, 12) = xyloc(0, 8) * xyloc(0, 4);
        xyloc(0, 13) = xyloc(0, 12) * xyloc(0, 1);
        xyloc(0, 14) = xyloc(0, 12) * xyloc(0, 2);
        xyloc(0, 15) = xyloc(0, 12) * xyloc(0, 3);

        c_matrix<double, 1, 1> result_pchip;
        result_pchip = prod(xyloc, bicubic_coeff);
        if (derivative) {
            derivative[0] = 0;
            derivative[1] = 0;
            for (int i = 1; i < 4; ++i) {
                for (int j = 0; j < 4; ++j) {
                    derivative[0] += i * bicubic_coeff(i * 4 + j, 0) *
                                     xyloc(0, 4 * (i - 1)) * xyloc(0, j);
                }
            }
            derivative[0] /= axis(0).increment(k0);
            for (int i = 0; i < 4; ++i) {
                for (int j = 1; j < 4; ++j) {
                    derivative[1] += j * bicubic_coeff(i * 4 + j, 0) *
                                     xyloc(0, 4 * i) * xyloc(0, j - 1);
                }
            }
            derivative[1] /= axis(1).increment(k1);
        }
        return result_pchip(0, 0);
    }

    /**
     * Matrix that is the inverse of the bicubic coefficients
     * This matrix will be used to construct the bicubic
     * coefficients. The result will be a 16x1 matrix.
     */
    c_matrix<double, 16, 16> _inv_bicubic_coeff;

    /**
     * Create variables that are used multiple times through out
     * the course of multiple calls per instance. Thus allowing
     * these variables to be created once and used as many times
     * as needed and save memory and time.
     */
    matrix<double> _derv_x;
    matrix<double> _derv_y;
    matrix<double> _derv_x_y;
    const size_t _kmin;
    const size_t _k0max;
    const size_t _k1max;
};

}  // end of namespace types
}  // end of namespace usml
