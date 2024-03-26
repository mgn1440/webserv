//  Create HttpResponse class
//  Parse nginx configuration file while constructing class
//  Create General nginx configuration file
//  Save parsed information on server and location maps

# include <string>
# include <fstream>
# include <sstream>

#include "../includes/HttpResponse.hpp"

HttpResponse::HttpResponse(const ConfigHandler& configHandler)
	: mConfigHandler(configHandler)
{}

HttpResponse::~HttpResponse()
{}

