#include <iostream>
#include <dirent.h>
#include <stdexcept>
#include <sys/stat.h>
#include <ctime>
#include <fstream>
#include <vector>
#include <string>

static void addOneLine(std::string& ret, const std::string& fileName, const std::string& absPath);
std::string timespecToString(const timespec ts);

std::string getIndexListOf(const std::string& URI, const std::string& absPath)
{
	DIR* dir;
	struct dirent* file;
	std::string ret;
	ret += "<!DOCTYPE html><html><head>";
	ret += "<style> td.detailsColumn{padding-inline-start: 2em; text-align: end; white-space; nowrap;} </style>";
	ret += "<title>Index of ";
	ret += URI.substr(0, 10);
	ret += "</title></head><body><h1>Index of ";
	ret += URI;
	ret += "</h1><hr><pre><table>";
	dir = opendir(absPath.c_str());
	if (dir == NULL) throw std::runtime_error("directory not opend");
	std::vector<std::string> dirVec;
	std::vector<std::string> fileVec;
	while (true){
		file = readdir(dir);
		if (file == NULL) break;
		if (file->d_name[0] == '.' && file->d_name[1] != '.') continue; // hiding file pass
		struct stat statBuf;
		if (stat((absPath + "/" + file->d_name).c_str(), &statBuf) != -1){
			if (S_ISDIR(statBuf.st_mode))
				dirVec.push_back(std::string(file->d_name) + "/");
			else
				fileVec.push_back(file->d_name);
		}
	}
	for (std::vector<std::string>::iterator it = dirVec.begin(); it != dirVec.end(); it++){
		addOneLine(ret, *it, absPath);
	}
	for (std::vector<std::string>::iterator it = fileVec.begin(); it != fileVec.end(); it++){
		addOneLine(ret, *it, absPath);
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

static void addOneLine(std::string& ret, const std::string& fileName, const std::string& absPath)
{
	ret += "<tr>";
	ret += "<td>";
	ret += "<a href=\"";
	ret += fileName;
	ret += "\">";
	ret += fileName;
	ret += "</a>";
	ret += "</td>";
	struct stat st;
	memset(&st, 0, sizeof(struct stat));
	stat((absPath + "/" + fileName).c_str(), &st);
	ret += "<td class=\"detailsColumn\">                ";
	ret += timespecToString(st.st_mtimespec);
	ret += "\n";
	ret += "</td>";
	ret += "</tr>";
}