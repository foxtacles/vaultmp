#ifndef INPUT_H
#define INPUT_H

BOOL installhook(LPVOID);


//Letters
#define VK_A 97
#define VK_B 98
#define VK_C 99
#define VK_D 100
#define VK_E 101
#define VK_F 102
#define VK_G 103
#define VK_H 104
#define VK_I 105
#define VK_J 106
#define VK_K 107
#define VK_L 108
#define VK_M 109
#define VK_N 110
#define VK_O 111
#define VK_P 112
#define VK_Q 113
#define VK_R 114
#define VK_S 115
#define VK_T 116
#define VK_U 117
#define VK_V 118
#define VK_W 119
#define VK_X 120
#define VK_Y 121
#define VK_Z 122

//Numbers
#define VK_0 48
#define VK_1 49
#define VK_2 50
#define VK_3 51
#define VK_4 52
#define VK_5 53
#define VK_6 54
#define VK_7 55
#define VK_8 56
#define VK_9 57

extern char iBuf[5000];
extern int isChatinputOpen;
extern int isChatMouseInputOpen;

#endif
