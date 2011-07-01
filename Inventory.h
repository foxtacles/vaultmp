#ifndef INVENTORY_H
#define INVENTORY_H

#include <windows.h>

#include <string>

using namespace std;

class Inventory {

      private:
            static string Fallout3Items[][2];

      public:
            Inventory();
            ~Inventory();


};

#endif
