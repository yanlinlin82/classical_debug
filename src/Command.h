#ifndef COMMAND_H__
#define COMMAND_H__

#include <string>
#include <vector>
#include <utility>

class Command
{
public:
	void Parse(const std::string& cmd);
	void Dump() const;

	bool IsEmpty() const { return words_.empty(); }

	const std::vector<std::pair<size_t, std::string>> GetWords() const { return words_; }
	size_t GetCmdSize() const { return cmd_.size(); }
private:
	std::string cmd_;
	std::vector<std::pair<size_t, std::string>> words_;
};

#endif
