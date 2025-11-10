/***********************************************************************************************************************
 * Copyright 2023 Degoras Project Team
 *
 * Licensed under the EUPL, Version 1.2 or – as soon they will be approved by the
 * European Commission - subsequent versions of the EUPL (the "Licence");
 *
 * You may not use this work except in compliance with the Licence.
 * You may obtain a copy of the Licence at:
 *
 * https://joinup.ec.europa.eu/software/page/eupl
 *
 * Unless required by applicable law or agreed to in writing, software distributed under the Licence is distributed on
 * an "AS IS" basis, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the Licence for the
 * specific language governing permissions and limitations under the Licence.
 **********************************************************************************************************************/

#pragma once

// ========== DPSLR INCLUDES ===========================================================================================
#include "libdpslr_global.h"
#include "dpslr_math.h"
// =====================================================================================================================

// ========== C++ INCLUDES ===========================================================================================
#include <tuple>
// =====================================================================================================================

namespace dpslr{
namespace geo{

// ========== CONSTANTS ================================================================================================
constexpr long double kLightPicosecondToMeter = 0.000299792458L;
constexpr long double kLightNanosecondToMeter = 0.299792458L;
// =====================================================================================================================

/// Generic namespace for meteorological purposes.
namespace meteo{

// ========== ENUMS ====================================================================================================

/**
 * @enum WtrVapPressModel
 * @brief Represents the different models that can be used to calculate the water vapor pressure.
 * @see IERS TN 21, chap. 9.
 * @see https://www.iers.org/IERS/EN/Publications/TechnicalNotes/tn21.html?nn=94912
 * @see IERS TN 32, chap. 9.
 * @see https://www.iers.org/IERS/EN/Publications/TechnicalNotes/tn32.html?nn=94912
 */
enum class WtrVapPressModel
{
    ORIGINAL_MM   = 1,   ///< Original formula. IERS TN 21, chap. 9.
    GIACOMO_DAVIS = 2    ///< Giacomo and Davis formula. IERS TN 32, chap. 9.
};

// =====================================================================================================================

// ========== FUNCTIONS ================================================================================================

/**
 * @brief Get the water vapor pressure using diferents models.
 *
 * This function calculates the water vapor pressure at the station site using differents models. You can use the
 * original formula used in Marini and Murray model (1973) (IERS TN 21, chap. 9) or the Giacomo and Davis model
 * (IERS TN 32, chap. 9).
 *
 * @see IERS TN 21, chap. 9.
 * @see https://www.iers.org/IERS/EN/Publications/TechnicalNotes/tn21.html?nn=94912
 * @see IERS TN 32, chap. 9.
 * @see https://www.iers.org/IERS/EN/Publications/TechnicalNotes/tn32.html?nn=94912
 * @see Giacomo, P., Equation for the dertermination of the density of moist air, Metrologia, V. 18, 1982
 * @param rh   Relative humidity at the laser site (percent eg, 50%).
 * @param temp Athmospheric temperature at the laser site (Kelvin).
 * @param pres Atmospheric pressure at the laser site (millibars).
 * @param wvpm Water vapor pressure model. See ::WtrVapPressModel for more details.
 * @return The water vapor pressure at the laser site (millibars).
 */
 double waterVaporPressure(double rh, double temp, double pres, WtrVapPressModel mode);

 // =====================================================================================================================

} // END NAMESPACE METEO.


/// Generic namespace for tropospheric models that calculate the path delay produced by the troposphere.
namespace tropo{

// ========== FUNCTIONS ================================================================================================

/**
 * @brief Calculates the tropospheric path delay (one way) using Marini and Murray model (1973).
 *
 * Calculate and return the tropospheric path delay (one way) produced by the troposhere for the laser beam path from a
 * ground station to a space object using the Marini and Murray model (1973). This model uses the pressure, temperature,
 * humidity, satellite elevation, station latitude, station height and laser wavelength. Commonly used in SLR.
 *
 * @param pres Atmospheric pressure (mbar).
 * @param temp Surface tempreature in (Kelvin).
 * @param rh   Relative humidity (%, eg. 50%).
 * @param el   Elevation of the target (radians).
 * @param wl   Beam wavelength (micrometres).
 * @param lat  Latitude of the station (radians).
 * @param ht   Height of the station (meters above sea level).
 * @param wvpm Water vapor pressure model. See ::WtrVapPressModel for more details.
 * @return One way tropospheric path delay (meters).
 */
LIBDPSLR_EXPORT double pathDelayMariniMurray(double pres, double temp, double rh, double el, double wl, double phi,
                                             double ht, meteo::WtrVapPressModel wvpm);

// =====================================================================================================================

} // END NAMESPACE TROPO.

/// Generic namespace for model different measurement units.
// TODO: Delete and use the new units system.
namespace meas{

template <typename T = double>
class Angle
{
public:
    enum class Unit
    {
        RADIANS = 0,
        DEGREES = 1
    };

    Angle(T angle = T(), Unit unit = Unit::RADIANS) : angle_(angle), unit_(unit) {}

    inline constexpr operator T() const {return angle_;}

    T convert(Unit unit)
    {
        if (this->unit_ != unit)
        {
            if (unit == Unit::RADIANS)
            {
                this->angle_ *= math::pi / 180.0;
                this->unit_ = unit;
            }
            else if (unit == Unit::DEGREES)
            {
                this->angle_ *= 180.0 / math::pi;
                this->unit_ = unit;
            }
        }
        return this->angle_;
    }

private:
    T angle_;
    Unit unit_;
};


template <typename T = double>
class Distance
{
public:
    enum class Unit
    {
        METRES = 0,
        LIGHT_PS = 1
    };

    Distance(T dist = T(), Unit unit = Unit::METRES, double ratio = 1.0) :
        dist_(dist), unit_(unit), ratio_(ratio) {};

    inline constexpr operator T() const {return dist_ * ratio_;};

    inline double getRatio() const {return this->ratio_;};

    void convert(Unit unit)
    {
        if (this->unit_ != unit)
        {
            if (unit == Unit::LIGHT_PS)
            {
                this->dist_ *= 1.0/kLightPicosecondToMeter;
                this->unit_ = unit;
            }

            else if (unit == Unit::METRES)
            {
                this->dist_ *= kLightPicosecondToMeter;
                this->unit_ = unit;
            }

        }
    };

    void convertRatio(double ratio)
    {
        dist_ *= ratio_ / ratio;
        ratio_ = ratio;
    }

private:
    T dist_;
    Unit unit_;
    double ratio_;
};
} // END NAMESPACE MEAS.

/// Generic namespace to handle different frames and transforms between them.
namespace frames{

/**
 * GeocentricCoords is defined as <x,y,z> tuple
 */
template <typename T = double, std::enable_if_t<std::is_floating_point<T>::value, bool> = true>
struct GeocentricPoint
{
    using DistType = meas::Distance<T>;

    meas::Distance<T> x;
    meas::Distance<T> y;
    meas::Distance<T> z;

    GeocentricPoint(T x = T(), T y = T(), T z = T(), typename DistType::Unit unit = DistType::Unit::METRES) :
        x(x, unit), y(y, unit), z(z, unit) {}

    template<typename Container>
    inline constexpr Container store() const {return Container{x,y,z};}
};

/**
 * GeodeticCoords is defined as <lat, lon, alt> tuple
 */
template <typename T = double, std::enable_if_t<std::is_floating_point<T>::value, bool> = true>
struct GeodeticPoint
{
    using AngleType = meas::Angle<T>;
    using DistType = meas::Distance<T>;

    AngleType lat;
    AngleType lon;
    DistType alt;

    GeodeticPoint(T lat = T(), T lon = T(), T alt = T(),
                   typename AngleType::Unit angle_unit = AngleType::Unit::RADIANS,
                   typename DistType::Unit dist_unit = DistType::Unit::METRES) :
        lat(lat, angle_unit), lon(lon, angle_unit), alt(alt, dist_unit)
    {}

    void convert(typename AngleType::Unit angle_unit, typename DistType::Unit dist_unit)
    {
        this->lat.convert(angle_unit);
        this->lon.convert(angle_unit);
        this->alt.convert(dist_unit);
    }

    template<typename Container>
    inline constexpr Container store() const {return Container{lat, lon, alt};}
};

} // END NAMESPACE FRAMES.
}} // END NAMESPACES GEO AND DPSLR.



// =====================================================================================================================
