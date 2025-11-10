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

// =====================================================================================================================
#pragma once
// =====================================================================================================================

#include <cmath>


// ========== LOCAL INCLUDES ===========================================================================================
#include "libdpslr_global.h"
// =====================================================================================================================

// ========== DPSLR NAMESPACES =========================================================================================
namespace dpslr{
namespace utils{
// =====================================================================================================================

// ========== INTERVAL CLASS ===========================================================================================
template <class T = double>
class LIBDPSLR_EXPORT Interval
{
public:

    Interval() :
        min_(0.0),
        max_(-1.0),
        incl_min_(true),
        incl_max_(true)
    {}

    Interval(T min, T max, bool incl_min = true, bool incl_max = true) :
        min_(min),
        max_(max),
        incl_min_(incl_min),
        incl_max_(incl_max)
    {}

    // Symetric interval constructor assuming 0 as center.
    Interval(T sym, bool incl_min = true, bool incl_max = true) :
        incl_min_(incl_min),
        incl_max_(incl_max)
    {
        this->max_ = std::abs(sym);
        this->min_ = -this->max_;
    }

    Interval(const Interval&) = default;
    Interval& operator =(Interval&&) = default;
    Interval(Interval&&) = default;
    Interval& operator =(const Interval&) = default;

    // Getters.
    T min() const {return this->min_;}
    T max() const {return this->max_;}
    T center() const {return (this->min_ + (this->max_ - this->min_)/2.0);}
    bool includeMin() const {return incl_min_;}
    bool includeMax() const {return incl_max_;}
    bool endpointsIncluded() const {return this->incl_max_ && this->incl_min_;}

    // Basic seters.
    void setMin(T min){this->min_ = min;}
    void setMax(T max){this->max_ = max;}
    void setIncludeMin(bool flag = true){this->incl_min_ = flag;}
    void setIncludeMax(bool flag = true){this->incl_max_ = flag;}

    // Full interval setter.
    void setInterval(T min, T max, bool incl_min = true, bool incl_max = true)
    {
        this->min_ = min;
        this->max_ = max;
        this->incl_min_ = incl_min;
        this->incl_max_ = incl_max;
    }

    // Symetric interval setter assuming 0 as center.
    void setInterval(T sym, bool incl_min = true, bool incl_max = true)
    {
        this->max_ = std::abs(sym);
        this->min_ = -this->max_;
        this->incl_min_ = incl_min;
        this->incl_max_ = incl_max;
    }

    // Move center method.
    void moveCenter(T distance)
    {
        this->max_ += distance;
        this->min_ += distance;
    }

    // Intervals related methods.

    bool isValid() const
    {
        return !this->endpointsIncluded() ? this->min_ <= this->max_ : this->min_ < this->max_;
    }

    bool isNull() const
    {
        return this->isValid() && this->min_ == this->max_;
    }

    T width() const {return isValid() ? (std::abs(this->max_ - this->min_)) : 0.0;}

    template <class D>
    bool contains(D value) const
    {
        if (!isValid())
            return false;
        if ((value < this->min_) || (value > this->max_ ))
            return false;
        if ((value == this->min_) && !this->incl_min_)
            return false;
        if ((value == this->max_) && !this->incl_max_)
            return false;
        return true;
    }

    template <class D>
    T toCenter(D value) const
    {
        return std::abs(value - this->center());
    }

    T toCenter() const
    {
        return std::abs(this->max_ - this->center());
    }

    template <class D = double>
    static D center(D min, D max)
    {
        return min + (max - min)/2.0;
    }

private:

    T min_;
    T max_;
    bool incl_min_;
    bool incl_max_;
};

}} // END NAMESPACES.
// =====================================================================================================================



















