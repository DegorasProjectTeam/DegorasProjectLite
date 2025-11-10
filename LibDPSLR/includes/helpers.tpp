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

/** ********************************************************************************************************************
 * @file helpers.tpp
 * @see helpers.h
 * @author DEGORAS PROJECT TEAM
 * @copyright EUPL License
***********************************************************************************************************************/

// =====================================================================================================================
#pragma once
// =====================================================================================================================

// ========== C++ INCLUDES =============================================================================================
#include <string>
#include <limits>
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <vector>
#include <map>
#include <cmath>
// =====================================================================================================================

// ========== DPSLR NAMESPACES =========================================================================================
namespace dpslr{
namespace helpers{
// =====================================================================================================================

template <class Container>
void split (Container& result, const std::string& s, const std::string& delimiters, bool empties)
{
    result.clear();
    size_t current;
    size_t next = std::numeric_limits<size_t>::max();
    do
    {
        if (!empties)
        {
            // Before getting substring, discard empty groups
            next = s.find_first_not_of( delimiters, next + 1 );
            // If end has not been reached, get next substring
            if (next != std::string::npos)
            {
                current = next;
                next = s.find_first_of( delimiters, current );
                result.push_back( s.substr( current, next - current ) );
            }
        }
        else
        {
            // Get next substring group
            current = next + 1;
            next = s.find_first_of( delimiters, current );
            result.push_back( s.substr( current, next - current ) );
        }
    }
    while (next != std::string::npos);
}

template <class Container>
Container split (const std::string& s, const std::string& delimiters, bool empties)
{
    Container result;
    dpslr::helpers::split(result, s, delimiters, empties);
    return result;
}

template <class Container>
void insert(Container& a, const Container& b)
{
    a.insert(a.end(), b.begin(), b.end());
}

template <typename Container>
std::string join(const Container& strings, const std::string& delimiters)
{
    std::string result;
    for (const auto& s : strings)
        result += (s + delimiters);

    if (!result.empty())
        result.erase(result.end() - delimiters.size(), result.end());

    return result;
}

template <typename Container, typename T>
bool contains(const Container& container, T elem)
{
    const auto it = std::find(container.begin(), container.end(), elem);
    return (it != container.end());
}

template <typename K, typename T>
bool contains(const std::map<K,T> &map, K key)
{
    const auto it = map.find(key);
    return (it != map.end());
}

template <typename Container, typename D>
unsigned countBin(const Container& container,D min,D max,
                  bool exmin, bool exmax)
{
    // Convenient alias.
    using ConType = typename Container::value_type;

    // Count the values.
    unsigned counter = std::count_if(container.begin(), container.end(),
        [&min, &max, &exmin, &exmax](const ConType& i)
        {
            bool result;
            if(exmax && exmin)
                result = (i < max) && (i > min);
            else if(exmax && !exmin)
                result = (i < max) && (i >= min);
            else if(!exmax && exmin)
                result = (i <= max) && (i > min);
            else if(!exmax && !exmin)
                result = (i <= max) && (i >= min);
            return result;
        });

    // Return the result.
    return counter;
}

template <typename Container, typename T>
bool find(const Container& container, const T  & elem, int& pos)
{
    bool result = false;
    auto it = std::find(container.begin(), container.end(), elem);
    if (it != container.end())
    {
        pos = std::distance(container.begin(), it);
        result = true;
    }
    return result;
}

template<typename T>
std::string numberToFixstr(T x, unsigned int prec)
{
    std::ostringstream strout ;
    strout << std::showpoint << std::setprecision(prec) << x ;
    std::string str = strout.str() ;
    size_t end = str.find_last_not_of( '0' ) + 1 ;
    str.erase(end);
    if(str.back() == '.')
        str.pop_back();
    return str;
}

template<typename T>
std::string numberToStr(T x, unsigned int prec, unsigned int dec_places)
{
    std::ostringstream strout;
    std::vector<std::string> aux;
    std::string dplac;
    strout << std::showpoint << std::setprecision(prec) << x ;
    std::string str = strout.str() ;
    split(aux, str, ".", true);
    if(aux.size()==2 && aux[1].size() > dec_places)
    {
        dplac = aux[1].substr(0, dec_places);
        return aux[0]+"."+dplac;
    }
    else
    {
        return str;
    }
}

template<typename T>
double truncToDouble(T x, unsigned int prec, unsigned int dec_places)
{
    return std::stod(numberToStr(x, prec, dec_places));
}

template<typename T>
double roundToDouble(T x, unsigned int dec_places)
{
    const long double multiplier = std::pow(10.0, dec_places);
    return std::round(x * multiplier) / multiplier;
}

}}// END NAMESPACES.
// =====================================================================================================================

