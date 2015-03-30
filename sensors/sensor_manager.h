/**
 *  @file sensor_manager.h
 *  Definition of the Class sensor_manager
 *  Created on: 12-Feb-2015 3:41:30 PM
 */

#pragma once

#include <usml/usml_config.h>

#include <usml/threads/threads.h>
#include <usml/sensors/sensor.h>
#include <usml/sensors/sensor_params.h>
#include <usml/sensors/sensor_pair_manager.h>
#include <usml/sensors/source_params_map.h>
#include <usml/sensors/receiver_params_map.h>
#include <usml/sensors/sensor_map_template.h>

namespace usml {
namespace sensors {

using namespace usml::threads;

class sensor;

/// @ingroup sensors
/// @{

/**
 * Container for all the sensor's in use by the USML. This class inherits from
 * the map_template class. This class implements the singleton GOF pattern.
 * The map stores pointers to sensor's and take's ownership of the pointers.
 * See usml/sensors/map_template.h A typedef of sensor::id_type has been defined
 * to allow for modification of the key of the map at a later time if needed.
 *
 * @author Ted Burns, AEgis Technologies Group, Inc.
 * @version 1.0
 * @updated 27-Feb-2015 3:15:03 PM
 */
class USML_DECLSPEC sensor_manager: public sensor_map_template<
		const sensor::id_type, sensor::reference> {
public:

	/**
	 * Singleton Constructor - Creates sensor_manager instance just once.
	 * Accessible everywhere.
	 * @return  pointer to the instance of the singleton sensor_manager
	 */
	static sensor_manager* instance();

	/**
	 * Construct a new instance of a specific sensor type.
	 * Sets the position and orientation values to NAN.
	 * These values are not set until the update_sensor()
	 * is invoked for the first time.
	 *
	 * @param sensorID		Identification used to find this sensor instance
	 * 						in sensor_manager.
	 * @param paramsID		Identification used to lookup sensor type data
	 * 						in source_params_map and receiver_params_map.
	 * @param description	Human readable name for this sensor instance.
	 * @return 				False if sensorID already exists.
	 */
	bool add_sensor(sensor::id_type sensorID, sensor_params::id_type paramsID,
			const std::string& description = std::string());

	/**
	 * Removes an existing sensor instance by sensorID.
	 * Also deletes the sensor from the sensor_pair_manager.
	 *
	 * @param sensorID		Identification used to find this sensor instance
	 * 						in sensor_manager.
	 * @return 				False if sensorID was not found.
	 */
	bool remove_sensor(const sensor::id_type sensorID);

	/**
	 * Updates an existing sensor instance by sensorID.
	 * Also updates the sensor in the sensor_pair_manager.
	 *
	 * @param sensorID		Identification used to find this sensor instance
	 * 						in sensor_manager.
	 * @param position  	Updated position data
	 * @param orientation	Updated orientation value
	 * @param force_update	When true, forces update without checking thresholds.
	 * @return 				False if sensorID was not found.
	 */
	bool update_sensor(const sensor::id_type sensorID, const wposition1& position,
			const sensor_orientation& orientation, bool force_update = false);

private:

	/**
	 * The singleton access pointer.
	 */
	static unique_ptr<sensor_manager> _instance;

	/**
	 * The _mutex for the singleton pointer.
	 */
	static read_write_lock _instance_mutex;
};

/// @}
}// end of namespace sensors
} // end of namespace usml
