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
 * @file helpers.h
 * @brief This file contains several helper tools (functions and classes).
 * @author DEGORAS PROJECT TEAM
 * @copyright EUPL License
***********************************************************************************************************************/

// =====================================================================================================================
#pragma once
// =====================================================================================================================

// ========== C++ INCLUDES =============================================================================================
#include <string>
#include <fstream>
#include <vector>
#include <algorithm>
#include <sstream>
#include <iterator>
// =====================================================================================================================

// ========== DP INCLUDES ==============================================================================================
#include "libdpslr_global.h"
// =====================================================================================================================


// ========== DPSLR NAMESPACES =========================================================================================
namespace dpslr{
namespace helpers{
// =====================================================================================================================

// Transform a string to upper case.
LIBDPSLR_EXPORT std::string toUpper(const std::string& str);

// Transform a string to lower case.
LIBDPSLR_EXPORT std::string toLower(const std::string& str);

// String left trim.
LIBDPSLR_EXPORT std::string ltrim(const std::string& s);

// String right trim.
LIBDPSLR_EXPORT std::string rtrim(const std::string& s);

// String trim.
LIBDPSLR_EXPORT std::string trim(const std::string& s);

/**
 * @brief Searches for the closest element in a sorted array to a given value.
 *
 * @param sorted_array The sorted array to search in.
 * @param x The value to find the closest element to.
 * @return The index of the closest element in the array.
*/
template <typename T>
long search_closest(const std::vector<T>& sorted_array, T x)
{
    auto iter_geq = std::lower_bound(sorted_array.begin(), sorted_array.end(), x);

    if (iter_geq == sorted_array.begin())
        return 0;

    if (iter_geq == sorted_array.end())
        return sorted_array.size() - 1;

    T a = *(iter_geq - 1);
    T b = *iter_geq;

    if (fabs(x - a) < fabs(x - b))
        return std::distance(sorted_array.begin(), iter_geq - 1);

    return std::distance(sorted_array.begin(), iter_geq);
}

// Custom split.
template <class Container>
void split (Container& result, const std::string& s, const std::string& delimiters, bool empties = true);

// Custom split 2.
template <class Container>
Container split (const std::string& s, const std::string& delimiters, bool empties = true );

// Custom push back.
// Insert b in a.
template <class Container>
void insert(Container& a, const Container& b);

// Custom string join.
template <typename Container>
std::string join(const Container& strings, const std::string& delimiters = " ");

// Custom contains helper.
template <typename Container, typename T>
bool contains(const Container& container, T elem);

/**
 * @brief Custom count bin function.
 *
 * This function counts how many values in the container are in the given interval. The boundaries can be
 * customized as open or closed intervals (in the mathematical sense).
 *
 * @param[in] container The container with the values.
 * @param[in] min
 * @param[in] max
 * @param[in] ex_min True if you want to exclude the minimum value (open interval).
 * @param[in] ex_max True if you want to exclude the maximum value (open interval).
 *
 * @return The number of elements in the container that are in the given interval.
 */
template <typename Container, typename T>
unsigned countBin(const Container& container, T min,T max, bool exmin = false, bool exmax = true);

// Custom find helper.
template <typename Container, typename T>
bool find(const Container& container, const T  & elem, int& pos);

// Custom number to fixed string conversion.
template<typename T>
std::string numberToFixstr(T x, unsigned int prec);

// Custom number to fixed string conversion.
template<typename T>
std::string numberToStr(T x, unsigned int prec, unsigned int dec_places);

// Custom truncate function to double.
template<typename T>
double truncToDouble(T x, unsigned int prec, unsigned int dec_places);

// Custom round function to double.
template<typename T>
double roundToDouble(T x, unsigned int dec_places);

// Extract from a vector the input indexes.
template<typename T>
std::vector<T> extract(const std::vector<T>& data, const std::vector<size_t>& indexes)
{
    // Auxiliar container.
    std::vector<T> result;
    // Extract the data.
    for(const std::size_t& index : indexes)
        result.push_back(data[index]);
    // Return the result.
    return result;
}

/**
 * Retrieves the data and indexes between the lower and upper bounds from a vector.
 *
 * @tparam T The type of data in the vector.
 * @param v The input vector.
 * @param lower The lower bound value.
 * @param upper The upper bound value.
 * @return A pair of vectors, where the first vector contains the data between the lower and upper bounds,
 *         and the second vector contains the corresponding indexes -> <[data], [indexes]>.
 */
template<typename T>
std::pair<std::vector<T>, std::vector<size_t>> dataBetween(const std::vector<T>& v, T lower, T upper)
{
    // TODO use intervals.
    std::vector<T> data_res;
    std::vector<size_t> index_res;

    auto l = std::lower_bound(v.begin(), v.end(), lower);
    auto u = std::upper_bound(v.begin(), v.end(), upper);

    for (auto it = l; it != u; ++it)
    {
        size_t index = std::distance(v.begin(), it);
        data_res.push_back(*it);
        index_res.push_back(index);
    }

    return std::make_pair(data_res, index_res);
}

// ========== OPERATORS DECLARATION ====================================================================================

template <class T>
std::ostream& operator<<(std::ostream& out, const std::vector<T>& v)
{
    out << '[';
    if (!v.empty())
        std::copy (v.begin(), v.end(), std::ostream_iterator<T>(out, ","));
    out << "\b]";
    return out;
}

// =====================================================================================================================


// Helper class for counting file line numbres of a file.
// ---------------------------------------------------------------------------------------------------------------------
class LIBDPSLR_EXPORT InputFileStream : public std::ifstream
{
public:

    InputFileStream(const std::string& path);

    std::istream& getline(std::string& line);
    bool isEmpty();
    unsigned int getLineNumber() const;

private:
    unsigned int line_number;
};
// ---------------------------------------------------------------------------------------------------------------------

// Convenient class to cast strings to bool using exceptions.
// ---------------------------------------------------------------------------------------------------------------------
class LIBDPSLR_EXPORT BoolString : public std::string
{
public:
    BoolString(const std::string &s);
    operator bool() const;
};
// ---------------------------------------------------------------------------------------------------------------------

}} // END NAMESPACES
// =====================================================================================================================

// ========== DP TEMPLATES INCLUDES ====================================================================================
#include "helpers.tpp"
// =====================================================================================================================
