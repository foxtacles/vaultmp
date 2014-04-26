#include "ScriptFunction.hpp"
#include "VaultException.hpp"
#include "PAWN.hpp"

using namespace std;

ScriptFunction::ScriptFunction(ScriptFunc fCpp, const string& def) : fCpp(fCpp), def(def), pawn(false)
{

}

ScriptFunction::ScriptFunction(const ScriptFuncPAWN& fPawn, AMX* amx, const string& def) : fPawn({amx, fPawn}), def(def), pawn(true)
{

}

ScriptFunction::~ScriptFunction()
{
	if (pawn)
		fPawn.name.~ScriptFuncPAWN();
}

unsigned long long ScriptFunction::Call(const vector<boost::any>& args)
{
	unsigned long long result;

	if (def.length() != args.size())
		throw VaultException("Script call: Number of arguments does not match definition").stacktrace();

	if (pawn)
		result = PAWN::Call(fPawn.amx, fPawn.name.c_str(), def.c_str(), args);
	else
	{
		// cdecl convention
		string::iterator it;
		vector<boost::any>::const_iterator it2;
		vector<unsigned int> data;

		for (it = def.begin(), it2 = args.begin(); it != def.end(); ++it, ++it2)
		{
			switch (*it)
			{
				case 'i':
				{
					unsigned int value = boost::any_cast<unsigned int>(*it2);
					data.push_back(value);
					break;
				}

				case 'q':
				{
					unsigned int value = boost::any_cast<signed int>(*it2);
					data.push_back(value);
					break;
				}

				case 'l':
				{
					unsigned long long value = boost::any_cast<unsigned long long>(*it2);
					data.push_back(*reinterpret_cast<unsigned int*>(&value));
					data.push_back(*reinterpret_cast<unsigned int*>((unsigned) &value + 4));
					break;
				}

				case 'w':
				{
					signed long long value = boost::any_cast<signed long long>(*it2);
					data.push_back(*reinterpret_cast<unsigned int*>(&value));
					data.push_back(*reinterpret_cast<unsigned int*>((unsigned) &value + 4));
					break;
				}

				case 'f':
				{
					double value = boost::any_cast<double>(*it2);
					data.push_back(*reinterpret_cast<unsigned int*>(&value));
					data.push_back(*reinterpret_cast<unsigned int*>((unsigned) &value + 4));
					break;
				}

				case 'p':
				{
					void* value = boost::any_cast<void*>(*it2);
					data.push_back(reinterpret_cast<unsigned int>(value));
					break;
				}

				case 's':
				{
					const string* value = boost::any_cast<string>(&*it2);
					data.push_back(reinterpret_cast<unsigned int>(value->c_str()));
					break;
				}

				default:
					throw VaultException("C++ call: Unknown argument identifier %02X", *it).stacktrace();
			}
		}

		unsigned int low;
		unsigned int high;
		unsigned int* source = &data[0];
		unsigned int size = data.size() * 4;

		asm(
			"MOV EDI,ESP\n"
			"SUB EDI,%3\n"
			"MOV ESI,%4\n"
			"MOV ECX,%3\n"
			"PUSH DS\n"
			"POP ES\n"
			"CLD\n"
			"REP MOVSB\n"
			"MOV ESI,ESP\n"
			"SUB ESP,%3\n"
			"CALL %2\n"
			"MOV ESP,ESI\n"
			"MOV %0,EAX\n"
			"MOV %1,EDX\n"
			: "=m"(low), "=m"(high)
			: "m"(fCpp) , "m"(size), "m"(source)
			: "eax", "edx", "ecx", "esi", "edi", "cc"
		);

		*reinterpret_cast<unsigned int*>(&result) = low;
		*reinterpret_cast<unsigned int*>(((unsigned) &result) + 4) = high;
	}

	return result;
}
