#ifndef CONSOLE_H__
#define CONSOLE_H__

class Console
{
public:
	int GetInputChar();
private:
	enum NonBlock { NB_ENABLE, NB_DISABLE };
	void SetNonBlock(NonBlock state);
	int KeyboardHit();
};

#endif
