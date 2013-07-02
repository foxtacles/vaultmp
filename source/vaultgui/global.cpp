#include "global.h"


globalData gData;
/*remotePlayers *playersData;
char name[64];
	bool player;
	float health;
	float pos[3];
	float rot[3];
};
*/
remotePlayers *playersData;

deque<string> chatQueue;

unsigned int createHash(const char * s,int length)
{
    unsigned int hash = 0;

    for(int i=0; i<length; ++s,i++)
    {
    	hash += *s;
    	hash += (hash << 10);
    	hash ^= (hash >> 6);
    }

    hash += (hash << 3);
    hash ^= (hash >> 11);
    hash += (hash << 15);

    return hash;
}