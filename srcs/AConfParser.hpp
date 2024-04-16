#ifndef ACONFPARSER_HPP
# define ACONFPARSER_HPP
# include <sstream>
# include <string>
# include <set>
# include <map>
# include <vector>

# define DEF_ST_LINE_SIZE 8000
# define DEF_HEADER_SIZE 8000
# define DEF_BODY_SIZE 8000000

class AConfParser {
protected:
	AConfParser();
	AConfParser(const AConfParser& rhs);
	AConfParser& operator=(const AConfParser& rhs);
	virtual ~AConfParser();

	void parseRoot(std::stringstream& ss, std::string& word);
	void parseIndex(std::stringstream& ss, std::string& word);
	void parseCGI(std::stringstream& ss, std::string& word);
	void parseAutoIndex(std::stringstream& ss, std::string& word);
	void parseLimitExcept(std::stringstream& ss, std::string& word);
	void parseClosedBracket(std::stringstream& ss, std::string& word);
	void parseClientMaxSize(std::stringstream& ss, std::string& word);
	virtual void PrintInfo() = 0; //debug

	bool isEnd(std::stringstream& ss, std::string& word);
	virtual void parse(std::ifstream& confFile) = 0;

	bool mbIsDuplicatedAutoIndex;
	bool mbIsDuplicatedLimitExcept;
	bool mbIsDuplicatedClientMaxSize;
	bool mbIsDuplicatedRoot;
	bool mbAutoIndex;
	size_t mMaxSize;
	std::vector<std::string> mHttpMethod;
	std::string mRoot;
	std::set<std::string> mIndex;
	std::map<std::string, std::string> mCGI; // Key: CGI 확장자 (php, py) Value: 실행파일 경로
private:

};




#endif