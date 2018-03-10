#include <cstdio>
#include <cstring>
#include <string>

using namespace std;

void ShowPrompt()
{
	printf("-");
}

const static string WHITE_SPACE_CHARS{" \t\r\n"};

const size_t MAX_INPUT_BUFFER_SIZE = 4096;

string Trim(const string& s, const string& chars = WHITE_SPACE_CHARS)
{
	auto pos1 = s.find_first_not_of(WHITE_SPACE_CHARS);
	auto pos2 = s.find_last_not_of(WHITE_SPACE_CHARS);
	if (pos1 == string::npos) {
		pos1 = 0;
	}
	if (pos2 == string::npos || pos2 < pos1) {
		pos2 = pos1 - 1;
	}
	return s.substr(pos1, pos2 - pos1 + 1);
}

string GetCommand()
{
	char buf[MAX_INPUT_BUFFER_SIZE] = "";
	fgets(buf, sizeof(buf), stdin);
	return Trim(string{buf});
}

void Process(const string& cmd)
{
	fprintf(stderr, "[DEBUG] command: '%s'\n", cmd.c_str());

	if (cmd == "q") {
		exit(0);
	}
}

int main(int argc, char* const* argv)
{
	for (;;) {
		ShowPrompt();
		auto cmd = GetCommand();
		Process(cmd);
	}
	return 0;
}
