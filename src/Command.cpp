#include <iostream>
#include "Command.h"

void Command::Parse(const std::string& cmd)
{
	words_.clear();

	size_t pos = 0;
	std::string word = "";
	char quote = '\0';
	for (size_t i = 0; i < cmd.size(); ++i) {
		char c = cmd[i];
		if (quote == '\0' && (c == ' ' || c == '\t' || c == '\r' || c == '\n')) {
			if (!word.empty()) {
				if (words_.empty() && word.size() > 1) {
					words_.push_back(std::make_pair(pos, word.substr(0, 1)));
					words_.push_back(std::make_pair(pos + 1, word.substr(1)));
				} else {
					words_.push_back(std::make_pair(pos, word));
				}
			}
			word = "";
		} else {
			if (word.empty()) {
				pos = i;
			}
			if (c == '\'' || c == '\"') {
				word += c;
				if (quote == '\0') {
					quote = c;
				} else {
					quote = '\0';
				}
			} else if (c == '\\') {
				if (i + 1 >= cmd.size()) {
					break;
				}
				switch (cmd[++i]) {
				case 'r': word += '\r'; break;
				case 'n': word += '\n'; break;
				case '\\': word += '\\'; break;
				case '\'': word += '\''; break;
				case '\"': word += '\"'; break;
				default: word += cmd[i + 1]; break;
				}
			} else {
				word += c;
			}
		}
	}
	if (!word.empty()) {
		if (words_.empty() && word.size() > 1) {
			words_.push_back(std::make_pair(pos, word.substr(0, 1)));
			words_.push_back(std::make_pair(pos + 1, word.substr(1)));
		} else {
			words_.push_back(std::make_pair(pos, word));
		}
	}
	cmd_ = cmd;

	//Dump();
}

void Command::Dump() const
{
	std::cerr << "[DEBUG] Split: '" << cmd_ << "' =>\n";
	for (auto x : words_) {
		std::cerr << "[DEBUG]    (pos = " << x.first << "): '" << x.second << "'\n";
	}
	std::cerr << std::flush;
}

