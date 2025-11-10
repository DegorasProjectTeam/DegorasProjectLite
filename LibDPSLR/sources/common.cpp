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

// ========== PROJECT INCLUDES =========================================================================================
#include "includes/common.h"
// =====================================================================================================================

#include <sstream>

// ========== DPSLR NAMESPACES =========================================================================================
namespace dpslr{
namespace common{
// =====================================================================================================================


// ========== CRD RECORD BASE ==========================================================================================
std::string ConsolidatedRecordStruct::generateCommentBlock() const
{
    // Base line.
    std::stringstream stream;

    // Generate all the comments.
    for(const auto& comment : this->comment_block)
        stream << "00 " << comment.substr(0, 80) << std::endl;

    // Return the comment block
    return stream.str().substr(0, stream.str().find_last_of('\n'));
}

void ConsolidatedRecordStruct::clearAll()
{
    consolidated_type = ConsolidatedFileTypeEnum::UNKNOWN_TYPE;
    generic_record_type = -1;
    comment_block.clear();
    tokens.clear();
    line_number = {};
}
// =====================================================================================================================
}}

