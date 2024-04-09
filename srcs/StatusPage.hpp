#ifndef STATUSPAGE_HPP
# define STATUSPAGE_HPP
# include <string>
# include <map>

class StatusPage
{
public:
	~StatusPage();

	std::string GetStatusPageOf(int StatusCode);
	std::string GetStatusMessageOf(int StatusCode);
	static StatusPage* GetInstance(void);
private:
	static StatusPage* sInstance;
	StatusPage();
	StatusPage(const StatusPage& rhs);
	StatusPage& operator=(const StatusPage& rhs);
	void initStatusCode(void);

	std::map<int, std::string> mStatusMessage;
};

#endif