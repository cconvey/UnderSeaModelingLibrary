/**
 * @file data_grid_fast_2d.h
 * Wrapper for a data_grid in 2d that uses the fast non-recursive
 * interpolation algorithm.
 */
#ifndef USML_TYPES_DATA_GRID_FAST_2D_H
#define USML_TYPES_DATA_GRID_FAST_2D_H

#include <usml/types/data_grid.h>
#include <usml/types/seq_vector.h>

//#define FAST_GRID_DEBUG

namespace usml {
namespace types {
/// @ingroup data_grid
/// @{

/**
* Implements fast calculations for data_grids using a non-recursive
* engine on interpolation. Takes an existing data_grid and wraps it
* into a new data_grid_fast and overrides the interpolate function
* to implement the non-recursive algorithm.
*
* Assumes that both axes of the passed data_grid both have the
* same interp_type.
*
* WARNING: This wrapper is specific to 2-dimensional grids
*          only. For 3-dimensional grids, the user should
*          implement the data_grid_fast_3d wrapper instead.
*
* Since the data is passed in and referenced by this wrapper,
* the data is taken control of and destroyed at the end of its
* use cycle.
*/


class USML_DECLSPEC data_grid_fast_2d : public data_grid<double,2>
{

    private:

        /** Used during interpolation to hold the axis offsets. */
        unsigned _offset[2];
        unsigned fast_index[2];

        /**
         * Matrix that is the inverse of the bicubic coefficients
         * This matrix will be used to construct the bicubic
         * coefficients. The result will be a 16x1 matrix.
         */
        c_matrix<double,16,16> inv_bicubic_coeff ;

        /**
         *
         */
        c_matrix<double,16,1> bicubic_coeff ;
        c_matrix<double,16,1> field ;
        c_matrix<double,4,1> inc ;
        c_matrix<double,1,16> xyloc ;
        c_matrix<double,1,1> result_pchip ;
        c_matrix<double,4,4> value ;
        const int kmin ;
        const int k0max ;
        const int k1max ;

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
         * respective derivaties with respect to x and y, and mixed xy
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
         * field(11,0) are the derivaties with respect to y of the extracted
         * data points. field(12,0) to field(15,0) are the derivatives with
         * respect to x and y of the extracted data points.
         *
         * @param interp_index  index on the grid for the closests data point
         * @param location      Location of the field calculation
         * @param derivative    Generate the derivative at the location (output)
         */
        double fast_pchip(const unsigned* interp_index, double* location,
                     double* derivative = NULL)
        {
            int k0 = interp_index[0];
            int k1 = interp_index[1];
            double norm0, norm1;

                // Checks for boundaries of the axes
            norm0 = (*_axis[0])(k0+1) - (*_axis[0])(k0);
            norm1 = (*_axis[1])(k1+1) - (*_axis[1])(k1);
            for(int i=-1; i<3; ++i) {
                for(int j=-1; j<3; ++j) {
                        //get appropriate data when at boundaries
                    if ((k0+i) >= k0max)
                        {fast_index[0] = k0max ;}
                    else if ((k0+i) <= kmin)
                        {fast_index[0] = kmin ;}
                    else
                        {fast_index[0] = k0+i ;}
                        //get appropriate data when at boundaries
                    if ((k1+j) >= k1max) {fast_index[1] = k1max ;}
                    else if ((k1+j) <= kmin) {fast_index[1] = kmin ;}
                    else {fast_index[1] = k1+j ;}
                    value(i+1,j+1) = data(fast_index);
                }
//                if (i<1) {
//                    if ((k0+i+1) >= k0max) {
//                        inc(i+1,0) = (_axis[0]->increment(k0max) + _axis[0]->increment(k0+i)) / _axis[0]->increment(k0max) ;
//                    }
//                    else if ((k0+i+1) <= kmin) {
//                        inc(i+1,0) = (_axis[0]->increment(k0+i+2) + _axis[0]->increment(kmin)) / _axis[0]->increment(kmin) ;
//                    }
//                    else {
//                        inc(i+1,0) = (_axis[0]->increment(k0+i+2) + _axis[0]->increment(k0+i)) / _axis[0]->increment(k0+i+1) ;
//                    }
//                } else {
//                    if ((k1+i-1) >= k1max) {
//                        inc(i+1,0) = (_axis[1]->increment(k1max) + _axis[1]->increment(k1+i-2)) / _axis[1]->increment(k1max) ;
//                    }
//                    else if ((k1+i-1) <= kmin) {
//                        inc(i+1,0) = (_axis[1]->increment(k1+i) + _axis[1]->increment(kmin)) / _axis[1]->increment(kmin) ;
//                    }
//                    else {
//                        inc(i+1,0) = (_axis[1]->increment(k1+i) + _axis[1]->increment(k1+i-2)) / _axis[1]->increment(k1+i-1) ;
//                    }
//                }
            }
inc(0,0) = inc(1,0) = inc(2,0) = inc(3,0) = 2;
            #ifdef FAST_GRID_DEBUG
                cout << "loc0: " << location[0] << "  loc1: " << location[1] << endl;
                cout << "offset0: " << k0 << "  offset1: " << k1 << endl;
                cout << "axis0: " << (*_axis[0])(k0) << "  axis1: " << (*_axis[1])(k1) << endl;
                cout << "data value at offset: " << ( (data(interp_index) > 1e6) ?
                            data(interp_index) - wposition::earth_radius : data(interp_index) ) << endl;
                cout << "inc: " << inc << endl;
                cout << "value: " << value << endl;
            #endif

                // Construct the field matrix
            field(0,0) = value(1,1);
            field(1,0) = value(1,2);
            field(2,0) = value(2,1);
            field(3,0) = value(2,2);
            field(4,0) = (value(2,1) - value(0,1)) / inc(0,0);
            field(5,0) = (value(2,2) - value(0,2)) / inc(0,0);
            field(6,0) = (value(3,1) - value(1,1)) / inc(1,0);
            field(7,0) = (value(3,2) - value(1,2)) / inc(1,0);
            field(8,0) = (value(1,2) - value(1,0)) / inc(2,0);
            field(9,0) = (value(1,3) - value(1,1)) / inc(3,0);
            field(10,0) = (value(2,2) - value(2,0)) / inc(2,0);
            field(11,0) = (value(2,3) - value(2,1)) / inc(3,0);
            field(12,0) = (value(2,2) - value(2,0) - value(0,2) + value(0,0)) / (inc(0,0)*inc(2,0));
            field(13,0) = (value(2,3) - value(2,1) - value(0,3) + value(0,1)) / (inc(0,0)*inc(3,0));
            field(14,0) = (value(3,2) - value(1,2) - value(3,0) + value(1,0)) / (inc(1,0)*inc(2,0));
            field(15,0) = (value(3,3) - value(3,1) - value(1,3) + value(1,1)) / (inc(1,0)*inc(3,0));

                // Construct the coefficients of the bicubic interpolation
            bicubic_coeff = prod(inv_bicubic_coeff, field);


                // Create the power series of the interpolation formula before hand for speed
            double x_inv = location[0] - (*_axis[0])(k0) ;
            double y_inv = location[1] - (*_axis[1])(k1) ;

            #ifdef FAST_GRID_DEBUG
                cout << "field: " << field << endl;
                cout << "bicubic_coeff: " << bicubic_coeff << endl;
                cout << "x_inv/norm0: " << x_inv/norm0 << "\ty_inv/norm1: " << y_inv/norm1 << endl;
            #endif

            xyloc(0,0) = 1;
            xyloc(0,1) = y_inv / norm1;
            xyloc(0,2) = xyloc(0,1) * xyloc(0,1) ;
            xyloc(0,3) = xyloc(0,2) * xyloc(0,1) ;
            xyloc(0,4) = x_inv / norm0;
            xyloc(0,5) = xyloc(0,4) * xyloc(0,1) ;
            xyloc(0,6) = xyloc(0,4) * xyloc(0,2) ;
            xyloc(0,7) = xyloc(0,4) * xyloc(0,3) ;
            xyloc(0,8) = xyloc(0,4) * xyloc(0,4) ;
            xyloc(0,9) = xyloc(0,8) * xyloc(0,1) ;
            xyloc(0,10) = xyloc(0,8) * xyloc(0,2) ;
            xyloc(0,11) = xyloc(0,8) * xyloc(0,3) ;
            xyloc(0,12) = xyloc(0,8) * xyloc(0,4) ;
            xyloc(0,13) = xyloc(0,12) * xyloc(0,1) ;
            xyloc(0,14) = xyloc(0,12) * xyloc(0,2) ;
            xyloc(0,15) = xyloc(0,12) * xyloc(0,3) ;

            result_pchip = prod(xyloc, bicubic_coeff);
            if(derivative) {
                derivative[0] = 0;
                derivative[1] = 0;
                for(int i=1; i<4; ++i){
                    for(int j=0; j<4; ++j){
                        derivative[0] += i * bicubic_coeff(i*4+j,0) *
                            xyloc(0,4*(i-1)) * xyloc(0,j) ;
                    }
                }
                for(int i=0; i<4; ++i){
                    for(int j=1; j<4; ++j){
                        derivative[1] += j * bicubic_coeff(i*4+j,0) *
                            xyloc(0,4*i) * xyloc(0,j-1) ;
                    }
                }
            }
            return result_pchip(0,0);
        }

    public:

        /**
        * Creates a fast interpolation grid from an existing data_grid.
        * Also constructs the inverse bicubic coefficient matrix to be
        * used at a later time during pchip calculations.
        *
        * @param _grid      The data_grid that is to be wrapped.
        */

        data_grid_fast_2d( const data_grid<double,2>& grid, bool copy_data = true ) :
            data_grid(grid,copy_data), bicubic_coeff(16,1), field(16,1), inc(4,1),
            xyloc(1,16), result_pchip(1,1), value(4,4), kmin(0u),
            k0max(_axis[0]->size()-1u), k1max(_axis[1]->size()-1u)
        {
                //Construct the inverse bicubic interpolation coefficient matrix
            inv_bicubic_coeff = zero_matrix<double> (16,16);
            inv_bicubic_coeff(0,0) = 1;
            inv_bicubic_coeff(1,8) = 1;
            inv_bicubic_coeff(2,0) = -3;
            inv_bicubic_coeff(2,1) = 3;
            inv_bicubic_coeff(2,8) = -2;
            inv_bicubic_coeff(2,9) = -1;
            inv_bicubic_coeff(3,0) = 2;
            inv_bicubic_coeff(3,1) = -2;
            inv_bicubic_coeff(3,8) = inv_bicubic_coeff(3,9) = 1;
            inv_bicubic_coeff(4,4) = 1;
            inv_bicubic_coeff(5,12) = 1;
            inv_bicubic_coeff(6,4) = -3;
            inv_bicubic_coeff(6,5) = 3;
            inv_bicubic_coeff(6,12) = -2;
            inv_bicubic_coeff(6,13) = -1;
            inv_bicubic_coeff(7,4) = 2;
            inv_bicubic_coeff(7,5) = -2;
            inv_bicubic_coeff(7,12) = inv_bicubic_coeff(7,13) = 1;
            inv_bicubic_coeff(8,0) = -3;
            inv_bicubic_coeff(8,2) = 3;
            inv_bicubic_coeff(8,4) = -2;
            inv_bicubic_coeff(8,6) = -1;
            inv_bicubic_coeff(9,8) = -3;
            inv_bicubic_coeff(9,10) = 3;
            inv_bicubic_coeff(9,12) = -2;
            inv_bicubic_coeff(9,14) = -1;
            inv_bicubic_coeff(10,0) = inv_bicubic_coeff(10,3) = 9;
            inv_bicubic_coeff(10,1) = inv_bicubic_coeff(10,2) = -9;
            inv_bicubic_coeff(10,4) = inv_bicubic_coeff(10,8) = 6;
            inv_bicubic_coeff(10,5) = inv_bicubic_coeff(10,10) = -6;
            inv_bicubic_coeff(10,6) = inv_bicubic_coeff(10,9) = 3;
            inv_bicubic_coeff(10,7) = inv_bicubic_coeff(10,11) = -3;
            inv_bicubic_coeff(10,12) = 4;
            inv_bicubic_coeff(10,13) = inv_bicubic_coeff(10,14) = 2;
            inv_bicubic_coeff(10,15) = 1;
            inv_bicubic_coeff(11,0) = inv_bicubic_coeff(11,3) = -6;
            inv_bicubic_coeff(11,1) = inv_bicubic_coeff(11,2) = 6;
            inv_bicubic_coeff(11,6) = inv_bicubic_coeff(11,12) = inv_bicubic_coeff(11,13) = -2;
            inv_bicubic_coeff(11,4) = -4;
            inv_bicubic_coeff(11,5) = 4;
            inv_bicubic_coeff(11,7) = 2;
            inv_bicubic_coeff(11,8) = inv_bicubic_coeff(11,9) = -3;
            inv_bicubic_coeff(11,10) = inv_bicubic_coeff(11,11) = 3;
            inv_bicubic_coeff(11,14) = inv_bicubic_coeff(11,15) = -1;
            inv_bicubic_coeff(12,0) = 2;
            inv_bicubic_coeff(12,2) = -2;
            inv_bicubic_coeff(12,4) = inv_bicubic_coeff(12,6) = 1;
            inv_bicubic_coeff(13,8) = 2;
            inv_bicubic_coeff(13,10) = -2;
            inv_bicubic_coeff(13,12) = inv_bicubic_coeff(13,14) = 1;
            inv_bicubic_coeff(14,0) = inv_bicubic_coeff(14,3) = -6;
            inv_bicubic_coeff(14,1) = inv_bicubic_coeff(14,2) = 6;
            inv_bicubic_coeff(14,4) = inv_bicubic_coeff(14,6) = -3;
            inv_bicubic_coeff(14,5) = inv_bicubic_coeff(14,7) = 3;
            inv_bicubic_coeff(14,8) = -4;
            inv_bicubic_coeff(14,10) = 4;
            inv_bicubic_coeff(14,9) = inv_bicubic_coeff(14,12) = inv_bicubic_coeff(14,14) = -2;
            inv_bicubic_coeff(14,11) = 2;
            inv_bicubic_coeff(14,13) = inv_bicubic_coeff(14,15) = -1;
            inv_bicubic_coeff(15,0) = inv_bicubic_coeff(15,3) = 4;
            inv_bicubic_coeff(15,1) = inv_bicubic_coeff(15,2) = -4;
            inv_bicubic_coeff(15,4) = inv_bicubic_coeff(15,6) = inv_bicubic_coeff(15,8) = inv_bicubic_coeff(15,9) = 2;
            inv_bicubic_coeff(15,5) = inv_bicubic_coeff(15,7) = inv_bicubic_coeff(15,10) = inv_bicubic_coeff(15,11) = -2;
            inv_bicubic_coeff(15,12) = inv_bicubic_coeff(15,13) = inv_bicubic_coeff(15,14) = inv_bicubic_coeff(15,15) = 1;

            fast_index[0] = 0;
            fast_index[1] = 0;
        }

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
        * @param return     Returns the value at the field location
        */

        double interpolate(double* location, double* derivative = NULL) {

            double result = 0;
            // find the interval index in each dimension

            for (unsigned dim = 0; dim < 2; ++dim) {

            // limit interpolation to axis domain if _edge_limit turned on

                if ( edge_limit(dim) ) {
                    double a = *(_axis[dim]->begin()) ;
                    double b = *(_axis[dim]->rbegin()) ;
                    double inc = _axis[dim]->increment(0);
                    if ( inc < 0) {                                                     // a > b
                        if ( location[dim] >= a ) {                                     //left of the axis
                            location[dim] = a ;
                            _offset[dim] = 0 ;
                        } else if ( location[dim] <= b ) {                              //right of the axis
                            location[dim] = b ;
                            _offset[dim] = _axis[dim]->size()-2 ;
                        } else {
                            _offset[dim] = _axis[dim]->find_index(location[dim]);       //somewhere in-between the endpoints of the axis
                        }
                    }
                    if (inc > 0 ) {                                                     // a < b
                        if ( location[dim] <= a ) {                                     //left of the axis
                            location[dim] = a ;
                            _offset[dim] = 0 ;
                        } else if ( location[dim] >= b ) {                              //right of the axis
                            location[dim] = b ;
                            _offset[dim] = _axis[dim]->size()-2 ;
                        } else {
                            _offset[dim] = _axis[dim]->find_index(location[dim]);       //somewhere in-between the endpoints of the axis
                        }
                    }

                // allow extrapolation if _edge_limit turned off

                } else {
                    _offset[dim] = _axis[dim]->find_index(location[dim]);
                }
            }

            switch(interp_type(0)) {

                ///****nearest****
            case -1:
                for(int dim = 0 ; dim < 2; ++dim) {
                    double inc = _axis[dim]->increment(0) ;
                    double u = abs(location[dim]-(*_axis[dim])(_offset[dim])) / inc ;
                    if(u < 0.5) {
                        fast_index[dim] = _offset[dim];
                    } else {
                        fast_index[dim] = _offset[dim]+1;
                    }
                }
                if(derivative) derivative[0] = derivative[1] = 0;
                return data(fast_index);
                break;

                ///****linear****
            case 0:
                double f11, f21, f12, f22, x_diff, y_diff;
                double x, x1, x2, y, y1, y2;

                x = location[0];
                x1 = (*_axis[0])(_offset[0]) ;
                x2 = (*_axis[0])(_offset[0]+1) ;
                y = location[1];
                y1 = (*_axis[1])(_offset[1]) ;
                y2 = (*_axis[1])(_offset[1]+1) ;
                f11 = data(_offset);
                fast_index[0] = _offset[0]+1 ; fast_index[1] = _offset[1] ;
                f21 = data(fast_index);
                fast_index[0] = _offset[0] ; fast_index[1] = _offset[1]+1 ;
                f12 = data(fast_index);
                fast_index[0] = _offset[0]+1 ; fast_index[1] = _offset[1]+1 ;
                f22 = data(fast_index);
                x_diff = x2 - x1 ;
                y_diff = y2 - y1 ;
                result = ( f11*(x2-x)*(y2-y) +
                           f21*(x-x1)*(y2-y) +
                           f12*(x2-x)*(y-y1) +
                           f22*(x-x1)*(y-y1) ) / (x_diff*y_diff);
                if(derivative) {
                    derivative[0] = (f21 - f11) / _axis[0]->increment(_offset[0]);
                    derivative[1] = (f12 - f11) / _axis[1]->increment(_offset[1]);
                }
                return result;
                break;

                ///****pchip****
            case 1:
                result = fast_pchip(_offset, location, derivative);
                return result;
                break;

            default:
                throw std::invalid_argument("Interp must be NEAREST, LINEAR, or PCHIP");
                break;
            }
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
        * @param   dx          First dimension of derviative (output).
        * @param   dy          Second dimension of derviative (output).
        */

        void interpolate(const matrix<double>& x, const matrix<double>& y, matrix<
                double>* result, matrix<double>* dx = NULL, matrix<double>* dy =
                NULL)
        {
            double location[2];
            double derivative[2];
            for (unsigned n = 0; n < x.size1(); ++n) {
                for (unsigned m = 0; m < x.size2(); ++m) {
                    location[0] = x(n, m);
                    location[1] = y(n, m);
                    if (dx == NULL || dy == NULL) {
                        (*result)(n, m) = (double) interpolate(location);
                    } else {
                        (*result)(n, m)
                                = (double) interpolate(location, derivative);
                        (*dx)(n, m) = (double) derivative[0];
                        (*dy)(n, m) = (double) derivative[1];
                    }
                }
            }
        }
};

} // end of namespace types
} // end of namespace usml

#endif