#ifndef REFERENCETYPES_H
#define REFERENCETYPES_H

const unsigned int ID_REFERENCE        = 0x00000001;
const unsigned int ID_OBJECT           = ID_REFERENCE << 1;
const unsigned int ID_ITEMLIST         = ID_OBJECT << 1;
const unsigned int ID_ITEM             = ID_ITEMLIST << 1;
const unsigned int ID_CONTAINER        = ID_ITEM << 1;
const unsigned int ID_ACTOR            = ID_CONTAINER << 1;
const unsigned int ID_PLAYER           = ID_ACTOR << 1;
const unsigned int ID_WINDOW           = ID_PLAYER << 1;
const unsigned int ID_BUTTON           = ID_WINDOW << 1;
const unsigned int ID_TEXT             = ID_BUTTON << 1;
const unsigned int ID_EDIT             = ID_TEXT << 1;
const unsigned int ID_CHECKBOX         = ID_EDIT << 1;
const unsigned int ID_RADIOBUTTON      = ID_CHECKBOX << 1;
const unsigned int ID_LISTITEM         = ID_RADIOBUTTON << 1;
const unsigned int ID_LIST             = ID_LISTITEM << 1;

const unsigned int ALL_REFERENCES      = (ID_REFERENCE | ID_OBJECT | ID_ITEM | ID_CONTAINER | ID_ACTOR | ID_PLAYER);
const unsigned int ALL_OBJECTS         = (ID_OBJECT | ID_ITEM | ID_CONTAINER | ID_ACTOR | ID_PLAYER);
const unsigned int ALL_ITEMLISTS       = (ID_ITEMLIST | ID_CONTAINER | ID_ACTOR | ID_PLAYER);
const unsigned int ALL_CONTAINERS      = (ID_CONTAINER | ID_ACTOR | ID_PLAYER);
const unsigned int ALL_ACTORS          = (ID_ACTOR | ID_PLAYER);
const unsigned int ALL_WINDOWS         = (ID_WINDOW | ID_BUTTON | ID_TEXT | ID_EDIT | ID_CHECKBOX | ID_RADIOBUTTON | ID_LIST);

template<typename T> struct rBases;
template<typename T> struct rTypes;
template<typename T> struct rTypesToken;

#endif
