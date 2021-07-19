#include <string>
#include <vector>

#include "nlohmann/json.hpp"

#include "xmessage_builder.hpp"

namespace nl = nlohmann;

namespace xeus
{
    nl::json create_error_reply(const std::string& evalue,
                                const std::vector<std::string>& trace_back)
    {
        nl::json content;
        content["status"] = "error";
        content["ename"] = "Error";
        content["evalue"] = evalue;
        content["traceback"] = trace_back;
        return content;
    }

    nl::json create_error_reply(const std::string& ename,
                                const std::string& evalue,
                                const std::vector<std::string>& trace_back)
    {
        const std::vector<std::string>& default_trace_back = nl::json::array();
        return create_error_reply(ename, evalue, default_trace_back);
    }

    nl::json create_successful_reply(const std::string& data,
                                     const std::string& data_type)
    {
        nl::json content;
        content["status"] = "ok";
        content[data_type] = data;
        return content;
    }

    nl::json create_successful_reply()
    {
        nl::json content;
        content["status"] = "ok";
        content["payload"] = nl::json::array();
        content["user_expressions"] = nl::json::object();
        return content;
    }
}
