#define IsHex(a) ((a>='0'&&a<='9')||(a>='A'&&a<='F')||(a>='a'&&a<='f'))

void ResetLog();
void SendToLog(char* str);
char* ExceptionToString(PEXCEPTION_RECORD p);