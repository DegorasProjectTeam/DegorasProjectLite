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
 * @file helpers.cpp
 * @see helpers.h
 * @author DEGORAS PROJECT TEAM
 * @copyright EUPL License
***********************************************************************************************************************/

// ========== C++ INCLUDES =============================================================================================
#include <algorithm>
#include <stdexcept>
#include <regex>
// =====================================================================================================================

// ========== DP INCLUDES ==============================================================================================
#include "includes/helpers.h"
// =====================================================================================================================

// ========== DPSLR NAMESPACES =========================================================================================
namespace dpslr{
namespace helpers{
// =====================================================================================================================

std::string toUpper(const std::string& str)
{
    // Aux string.
    std::string result;
    // Make the transformation and return the new string..
    std::transform(str.begin(), str.end(), std::back_inserter(result), ::toupper);
    return result;
}

std::string toLower(const std::string& str)
{
    // Aux string.
    std::string result;
    // Make the transformation and return the new string..
    std::transform(str.begin(), str.end(), std::back_inserter(result), ::tolower);
    return result;
}

std::string ltrim(const std::string &s)
{
    return std::regex_replace(s, std::regex("^\\s+"), std::string(""));
}

std::string rtrim(const std::string &s)
{
    return std::regex_replace(s, std::regex("\\s+$"), std::string(""));
}

std::string trim(const std::string &s)
{
    return ltrim(rtrim(s));
}


InputFileStream::InputFileStream(const std::string& path):
    std::ifstream(path),
    line_number(0){}

std::istream& InputFileStream::getline(std::string& line)
{
    this->line_number++;
    return std::getline(*this, line);
}

bool InputFileStream::isEmpty() {return this->peek() == std::ifstream::traits_type::eof();}

unsigned int InputFileStream::getLineNumber() const {return this->line_number;}

BoolString::BoolString(const std::string &s) : std::string(s)
{
    if (s != "0" && s != "1")
        throw std::invalid_argument(s);
}

BoolString::operator bool() const {return *this == "1";}

}}// END NAMESPACES.
// =====================================================================================================================
