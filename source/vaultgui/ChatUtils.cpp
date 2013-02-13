#include "ChatUtils.h"

void replaceAll(std::string& str, const std::string& from, const std::string& to) {
    if(from.empty())
        return;
    size_t start_pos = 0;
    while((start_pos = str.find(from, start_pos)) != std::string::npos) {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length(); // In case 'to' contains 'from', like replacing 'x' with 'yx'
    }
}

bool replace(std::string& str, const std::string& from, const std::string& to) {
    size_t start_pos = str.find(from);
    if(start_pos == std::string::npos)
        return false;
    str.replace(start_pos, from.length(), to);
    return true;
}

void myReplace(std::string& str, const std::string& oldStr, const std::string& newStr)
{
  size_t pos = 0;
  while((pos = str.find(oldStr, pos)) != std::string::npos)
  {
     str.replace(pos, oldStr.length(), newStr);
     pos += newStr.length();
  }
}

void ParseChatText(string& p)
{
	myReplace(p,"$white","[colour='FFFFFFFF']");
 	myReplace(p,"$silver","[colour='FFc0c0c0']");
 	myReplace(p,"$gray","[colour='FF808080']");
 	myReplace(p,"$black","[colour='FF000000']");
 	myReplace(p,"$red","[colour='FFFF0000']");
 	myReplace(p,"$maroon","[colour='FF800000']");
 	myReplace(p,"$yellow","[colour='FFFFFF00']");
 	myReplace(p,"$olive","[colour='FF808000']");
 	myReplace(p,"$lime","[colour='FF00FF00']");
 	myReplace(p,"$green","[colour='FF008000']");
 	myReplace(p,"$aqua","[colour='FF00FFFF']");
 	myReplace(p,"$teal","[colour='FF008080']");
 	myReplace(p,"$blue","[colour='FF0000FF']");
 	myReplace(p,"$navy","[colour='FF000080']");
 	myReplace(p,"$fuchsia","[colour='FFFF00FF']");
 	myReplace(p,"$purple","[colour='FF800080']");
	


	//p=p.replace("$red","[colour='FF0000']");
}