/**
 * @file beampatterns.h Beampatterns
 * @defgroup beampatterns beampatterns
 *
 * A "beam pattern" computes the change of intensity for a signal arriving
 * from a given arrival angle as a function of frequency, steering angle, and
 * the local speed of sound. Beam patterns are provided in linear units
 * with a range from 0.0 to 1.0.  All of the beam patterns defined in this
 * module are immutable to support thread safety without locking.
 *
 * @defgroup bp_test Beampatterns Tests
 * @ingroup beampatterns
 *
 * Regression tests for the beampatterns package
 */
#pragma once

#include <beampatterns/beampattern_utilities.h>
#include <beampatterns/bp_model.h>
#include <beampatterns/bp_arb.h>
#include <beampatterns/bp_cardioid.h>
#include <beampatterns/bp_line.h>
#include <beampatterns/bp_multi.h>
#include <beampatterns/bp_omni.h>
#include <beampatterns/bp_piston.h>
#include <beampatterns/bp_planar.h>
#include <beampatterns/bp_trig.h>
#include <beampatterns/bp_grid.h>
#include <beampatterns/bp_solid.h>
