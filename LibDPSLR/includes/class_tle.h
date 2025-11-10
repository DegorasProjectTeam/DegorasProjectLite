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

#include "libdpslr_global.h"

#include <string>

// TODO: Mejorar la clase poniendo bien los datos.
// Podemos usar esto de referencia: https://github.com/FedericoStra/tletools

class LIBDPSLR_EXPORT TLE
{
public:

    bool parseLines(const std::string& tle);

    bool isValid() const;
    std::string getLines() const;
    const std::string& getTitle() const;
    const std::string& getFirstLine() const;
    const std::string& getSecondLine() const;

    const std::string& getNorad() const;

private:
    std::string title;
    std::string first_line;
    std::string second_line;

    std::string norad_;
};


