#include <iostream>
#include <dirent.h>
#include <stdexcept>
#include <sys/stat.h>
#include <ctime>

std::string timespecToString(const timespec ts);
std::string getIndexListOf(const std::string& path)
{
	DIR* dir;
	struct dirent* file;
	std::string ret;
	ret += "<!DOCTYPE html><html><head>";
	ret += "<style> td.detailsColumn{padding-inline-start: 2em; text-align: end; white-space; nowrap;} </style>";
	ret += "<title>Index of";
	ret += path;
	ret += "</title></head><body><h1>Index of";
	ret += path;
	ret += "</h1><hr><pre><table>";
	dir = opendir(path.c_str());
	if (dir == NULL) throw std::runtime_error("directory not opend");
	while (true){
		file = readdir(dir);
		if (file == NULL) break;
		if (file->d_name[0] == '.' && file->d_name[1] != '.') continue; // hiding file pass
		ret += "<tr>";
		ret += "<td>";
		ret += "<a href=\"";
		ret += file->d_name;
		ret += "\">";
		ret += file->d_name;
		ret += "</a>";
		ret += "</td>";
		struct stat st;
		stat(file->d_name, &st);
		if (file->d_name[0] == '.') continue;
		ret += "<td class=\"detailsColumn\">                ";
		ret += timespecToString(st.st_mtimespec);
		ret += "\n";
		ret += "</td>";
		ret += "</tr>";
	}
	ret += "</table></pre><hr></body></html>";
	return(ret);
}

std::string timespecToString(const timespec ts) {
    std::tm* timeinfo = NULL;
    char buffer[80];
    
    timeinfo = std::localtime(&ts.tv_sec);
    std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", timeinfo);
    
    return buffer;
}