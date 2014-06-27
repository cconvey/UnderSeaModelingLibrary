/**
 * @file eigenverb_bistatic.cc
 */

#include <usml/waveq3d/eigenverb_bistatic.h>
#include <usml/waveq3d/spreading_hybrid_gaussian.h>

/**  Constructor  **/
eigenverb_bistatic::eigenverb_bistatic( wave_queue& wave ) {
    _spreading_model = wave.getSpreading_Model() ;
    _wave_time = wave.getTimeRef() ;
}

/**
 * Places an eigenverb into the class of "upper" bins to be used for the overall
 * reverberation calculation.
 */
bool eigenverb_monostatic::notifyUpperCollision( unsigned de, unsigned az, double time,
                   double grazing, double speed, const seq_vector& frequencies,
                   const wposition1& position, const wvector1& ndirection, int ID ) {

    bool status = false ;
    eigenverb verb ;
    verb.de = de ;
    verb.az = az ;
    verb.time = _wave_time + time ;
    verb.grazing = grazing ;
    verb.c = speed ;
    verb.pos = position ;
    verb.ndir = ndirection ;
    verb.frequencies = frequencies ;

        // Calculate the one way TL and the width of the gaussian
        // at the time of impact with the boundary.
    c_vector<double,3> offset, distance ;
    offset(0) = time ;                                      // Temporal offset still exists
    offset(1) = offset(2) = 0.0 ;                           // No offset
    distance(0) = distance(1) = distance(2) = 0.0 ;         // Zero distance
    const vector<double> amp = _spreading_model->intensity( position, de, az, offset, distance ) ;
    verb.intensity = - 10.0 * log10( amp ) ;
    verb.sigma_de = _spreading_model->width_de( de, az, offset ) ;
    verb.sigma_az = _spreading_model->width_az( de, az, offset ) ;

    switch ID:
        // Interactions from the volume reverberation will use
        // IDs to distinguish where they will be cataloged to.
        case 1:
            _source_surface.push_back( verb ) ;
            status = true ;
            break ;
        case 2:
            _receiver_surface.push_back( verb ) ;
            status = true ;
            break ;
        // No ID present, means this is a generic surface interaction.
        default:
            break ;

    return status ;
}

/**
 * Places an eigenverb into the class of "lower" bins to be used for the overall
 * reverberation calculation.
 */
bool eigenverb_monostatic::notifyLowerCollision( unsigned de, unsigned az, double time,
                   double grazing, double speed, const seq_vector& frequencies,
                   const wposition1& position, const wvector1& ndirection, int ID ) {

    bool status = false ;
    eigenverb verb ;
    verb.de = de ;
    verb.az = az ;
    verb.time = _wave._time + time ;
    verb.grazing = grazing ;
    verb.c = speed ;
    verb.pos = position ;
    verb.ndir = ndirection ;
    verb.frequencies = frequencies ;

        // Calculate the one way TL and the width of the gaussian
        // at the time of impact with the boundary.
    c_vector<double,3> offset, distance ;
    offset(0) = time ;                                      // Temporal offset still exists
    offset(1) = offset(2) = 0.0 ;                           // No offset
    distance(0) = distance(1) = distance(2) = 0.0 ;         // Zero distance
    const vector<double> amp = _spreading_model->intensity( position, de, az, offset, distance ) ;
    verb.intensity = - 10.0 * log10( amp ) ;
    verb.sigma_de = _spreading_model->width_de( de, az, offset ) ;
    verb.sigma_az = _spreading_model->width_az( de, az, offset ) ;

    switch ID:
        // Interactions from the volume reverberation will use
        // IDs to distinguish where they will be cataloged to.
        case 1:
            _source_bottom.push_back( verb ) ;
            status = true ;
            break ;
        case 2:
            _receiver_bottom.push_back( verb ) ;
            status = true ;
            break ;
        // No ID present, means this is a generic bottom interaction.
        default:
            break ;

    return status ;
}

void eigenverb_monostatic::compute_reverberation() {} ;