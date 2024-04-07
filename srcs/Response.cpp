#include "ConfigHandler.hpp"
#include "Response.hpp"

Response::Response(struct Request& rhs)
    : mConfigHandler(ConfigHandler::GetConfigHandler())
{
    // TODO:
    // find server block using the request
}

