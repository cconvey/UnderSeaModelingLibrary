/**
 * @file types.h Fundamental Data Types
 * @defgroup types types
 *
 * This package defines the basic data types specific to USML.
 *
 * @defgroup wposition World Coordinates
 * @ingroup types
 *
 * Locations and directions are defined in a spherical earth
 * coordinate system relative to the WGS84 center of curvature for the
 * local area of operations. The model also supports the ability to
 * transform these coordinates into the geodetic parameters of
 * latitude, longitude, and altitude.
 *
 * @defgroup data_grid Data Grids and Sequences
 * @ingroup types
 *
 * These components define support for N-dimensional data sets and their
 * associated axes. Supports interpolation in any number of dimensions.
 * Fast interpolation algorithms require an ability to quickly lookup an
 * axis index appropriate given a floating point axis value.
 * These axes are implemented as read-only, monotonic sequence of values.
 *
 * @defgroup bvector Body Coordinates
 * @ingroup types
 *
 * Vector relative to body along the aircraft principal axes and
 * body orientation.
 *
 * @defgroup types_test Type Tests
 * @ingroup types
 *
 * Regression tests for the types package
 */
#pragma once

#include <types/wvector.h>
#include <types/wvector1.h>
#include <types/wposition.h>
#include <types/wposition1.h>

#include <types/seq_linear.h>
#include <types/seq_log.h>
#include <types/seq_data.h>
#include <types/seq_rayfan.h>
#include <types/seq_augment.h>

#include <types/data_grid.h>
#include <types/gen_grid.h>
#include <types/data_grid_bathy.h>
#include <types/data_grid_svp.h>

#include <types/bvector.h>
#include <types/orientation.h>
