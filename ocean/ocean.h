/**
 * @file ocean.h Ocean Components
 * @defgroup ocean ocean
 *
 * This package defines the interface between WaveQ3D and the
 * models used to describe the synthetic natural environment.
 * This package also includes example implementations that have
 * been derived from public sources.  Implementations of
 * limited distribution ocean models, such as those found in the
 * U.S. Navy's Oceanographic and Atmospheric Master Library (OAML)
 * should be specified in a separate package.
 *
 * This package allows multiple threads to share a common representation
 * of the ocean. Ocean data memory is managed using const shared pointers
 * (often abbreviated as csptr) so that dynamically created ocean
 * objects remain available as long as one thread is still
 * using them. Ocean object shared pointers are defined as immutable
 * so that multiple threads can access common ocean data without locking.
 * This creates an additional burden on ocean object creators
 * because objects must be configured before they are wrapped in the
 * const shared pointer.
 *
 * @defgroup profiles Ocean Profiles
 * @ingroup ocean
 *
 * A "profile model" computes the environmental parameters of
 * ocean water. The modeled properties include the sound velocity
 * profile and the attenuation due to sea water absorption.
 * This class implements an attenuation model through delegation.
 * The delegated model is defined separately and added to its host
 * during/after construction.  The host is defined as an attenuation_model
 * subclass so that its children can share the attenuation model
 * through this delegation.
 *
 * @defgroup boundaries Ocean Boundaries
 * @ingroup ocean
 *
 * A "boundary model" computes the environmental parameters of
 * the ocean's surface or bottom.  The modeled properties include
 * the depth and reflection properties of the interface.
 * This class implements a reflection loss model through delegation.
 * The delegated model is defined separately and added to its host
 * during/after construction.  The host is defined as an reflect_loss_model
 * subclass so that its children can share the reflection loss model
 * through this delegation.
 *
 * @defgroup ocean_model Ocean Model
 * @ingroup ocean
 *
 * Combines the effects of surface, bottom, volume, and profile into a single
 * model.
 *
 * @defgroup ocean_test Ocean Tests
 * @ingroup ocean
 *
 * Regression tests for the ocean package
 */
#pragma once

#include <ocean/ambient_constant.h>
#include <ocean/ambient_model.h>
#include <ocean/ambient_wenz.h>
#include <ocean/ascii_arc_bathy.h>
#include <ocean/ascii_profile.h>
#include <ocean/attenuation_constant.h>
#include <ocean/attenuation_model.h>
#include <ocean/attenuation_thorp.h>
#include <ocean/boundary_flat.h>
#include <ocean/boundary_grid.h>
#include <ocean/boundary_model.h>
#include <ocean/boundary_slope.h>
#include <ocean/data_grid_mackenzie.h>
#include <ocean/ocean_model.h>
#include <ocean/ocean_shared.h>
#include <ocean/ocean_utils.h>
#include <ocean/profile_catenary.h>
#include <ocean/profile_grid.h>
#include <ocean/profile_linear.h>
#include <ocean/profile_model.h>
#include <ocean/profile_munk.h>
#include <ocean/profile_n2.h>
#include <ocean/reflect_loss_beckmann.h>
#include <ocean/reflect_loss_constant.h>
#include <ocean/reflect_loss_eckart.h>
#include <ocean/reflect_loss_model.h>
#include <ocean/reflect_loss_netcdf.h>
#include <ocean/reflect_loss_rayleigh.h>
#include <ocean/reflect_loss_rayleigh_grid.h>
#include <ocean/scattering_constant.h>
#include <ocean/scattering_chapman.h>
#include <ocean/scattering_lambert.h>
#include <ocean/scattering_model.h>
#include <ocean/volume_flat.h>
#include <ocean/volume_model.h>
#include <ocean/wave_height_pierson.h>
