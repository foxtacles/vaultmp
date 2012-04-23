#include "Public.h"

map<string, Public*> Public::publics;

Public::Public(ScriptFunc _public, string name, string def) : ScriptFunction(_public, def)
{
	publics.insert(pair<string, Public*>(name, this));
}

Public::Public(ScriptFuncPAWN _public, AMX* amx, string name, string def) : ScriptFunction(_public, amx, def)
{
	publics.insert(pair<string, Public*>(name, this));
}

Public::~Public()
{

}

unsigned long long Public::Call(string name, const vector<boost::any>& args)
{
	map<string, Public*>::iterator it;
	it = publics.find(name);

	if (it == publics.end())
		throw VaultException("Public with name %s does not exist", name.c_str());

	return it->second->ScriptFunction::Call(args);
}

string Public::GetDefinition(string name)
{
	map<string, Public*>::iterator it;
	it = publics.find(name);

	if (it == publics.end())
		throw VaultException("Public with name %s does not exist", name.c_str());

	return it->second->def;
}

void Public::DeleteAll()
{
	map<string, Public*>::iterator it;

	for (it = publics.begin(); it != publics.end(); publics.erase(it++))
	{
		Public* _public = it->second;
		delete _public;
	}
}
