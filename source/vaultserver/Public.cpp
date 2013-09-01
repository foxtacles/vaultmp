#include "Public.hpp"
#include "VaultException.hpp"

using namespace std;

unordered_map<string, Public*> Public::publics;

Public::Public(ScriptFunc _public, const string& name, const string& def) : ScriptFunction(_public, def)
{
	publics.emplace(name, this);
}

Public::Public(ScriptFuncPAWN _public, AMX* amx, const string& name, const string& def) : ScriptFunction(_public, amx, def)
{
	publics.emplace(name, this);
}

Public::~Public()
{

}

unsigned long long Public::Call(const string& name, const vector<boost::any>& args)
{
	auto it = publics.find(name);

	if (it == publics.end())
		throw VaultException("Public with name %s does not exist", name.c_str()).stacktrace();

	return it->second->ScriptFunction::Call(args);
}

const string& Public::GetDefinition(const string& name)
{
	auto it = publics.find(name);

	if (it == publics.end())
		throw VaultException("Public with name %s does not exist", name.c_str()).stacktrace();

	return it->second->def;
}

bool Public::IsPAWN(const string& name)
{
	auto it = publics.find(name);

	if (it == publics.end())
		throw VaultException("Public with name %s does not exist", name.c_str()).stacktrace();

	return it->second->pawn;
}

void Public::DeleteAll()
{
	for (auto it = publics.begin(); it != publics.end(); publics.erase(it++))
	{
		Public* _public = it->second;
		delete _public;
	}
}
