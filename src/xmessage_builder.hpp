/***************************************************************************
* Copyright (c) 2016, Johan Mabille, Sylvain Corlay, Martin Renou          *
* Copyright (c) 2016, QuantStack                                           *
*                                                                          *
* Distributed under the terms of the BSD 3-Clause License.                 *
*                                                                          *
* The full license is in the file LICENSE, distributed with this software. *
****************************************************************************/

#ifndef XEUS_MESSAGE_BUILDER_HPP
#define XEUS_MESSAGE_BUILDER_HPP

#include <string>
#include <vector>

#include "nlohmann/json.hpp"

namespace nl = nlohmann;

namespace xeus
{
    nl::json create_error_reply(const std::string& ename,
                                const std::string& evalue,
                                const std::vector<std::string>& trace_back);
    nl::json create_successful_reply(const std::string& data,
                                     const std::string& data_type);
    nl::json create_successful_reply();
}

#endif
