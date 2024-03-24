#ifndef ACONFPARSER_HPP
# define ACONFPARSER_HPP
# include <sstream>
# include <string>
# include <set>
# include <vector>




class AConfParser {
protected:
	AConfParser();
	virtual ~AConfParser();

	void	parseRoot(std::stringstream& ss, const std::string& word);
	void	parseIndex(std::stringstream& ss, const std::string& word);
	void	parseAutoIndex(std::stringstream& ss, const std::string& word);
	void	parseLimitExcept(std::stringstream& ss, const std::string& word);
	void	parseClosedBracket()

	bool	isEnd(std::stringstream& ss, std::string& word);
	void	Parse(std::ifstream confFile) = 0;
private:
	AConfParser&	operator=(const AConfParser& rhs);
	AConfParser(const AConfParser& rhs);


	bool							mbAutoIndex;
	std::vector<std::string>		mHttpMethod;
	std::string						mRoot;
	std::set<std::string>			mIndex;
};

#endif