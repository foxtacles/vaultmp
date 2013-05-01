#include "ScriptFunction.h"

using namespace std;

ScriptFunction::ScriptFunction(ScriptFunc fCpp, const string& def) : fCpp(fCpp), def(def), pawn(false)
{

}

ScriptFunction::ScriptFunction(ScriptFuncPAWN fPawn, AMX* amx, const string& def) : fPawn(fPawn), amx(amx), def(def), pawn(true)
{

}

unsigned long long ScriptFunction::Call(const vector<boost::any>& args)
{
	unsigned long long result = 0x0000000000000000;

	if (def.length() != args.size())
		throw VaultException("Script call: Number of arguments does not match definition").stacktrace();

	if (pawn)
		result = PAWN::Call(amx, fPawn.c_str(), def.c_str(), args);

	else
	{
		// cdecl convention
		string::reverse_iterator it;
		vector<boost::any>::const_reverse_iterator it2;
		vector<unsigned int> data;

		for (it = def.rbegin(), it2 = args.rbegin(); it != def.rend(); ++it, ++it2)
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
					data.push_back(*reinterpret_cast<unsigned int*>((unsigned) &value + 4));
					data.push_back(*reinterpret_cast<unsigned int*>(&value));
					break;
				}

				case 'f':
				{
					double value = boost::any_cast<double>(*it2);
					data.push_back(*reinterpret_cast<unsigned int*>((unsigned) &value + 4));
					data.push_back(*reinterpret_cast<unsigned int*>(&value));
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
					const string& value = boost::any_cast<string>(*it2);
					data.push_back(reinterpret_cast<unsigned int>(value.c_str()));
					break;
				}

				default:
					throw VaultException("C++ call: Unknown argument identifier %02X", *it).stacktrace();
			}
		}


		unsigned int low;
		unsigned int high;
		unsigned int size = data.size();
		unsigned int* _data = &data[0];

		for (unsigned int i = 0; i < size; ++i, ++_data)
		{
			asm(
				"PUSH %0\n"
				:
				: "m"(*_data)
				:
			);
		}

		size *= 4;

		asm(
			"CALL %2\n"
			"ADD ESP,%3\n"
			"MOV %0,EAX\n"
			"MOV %1,EDX\n"
			: "=m"(low), "=m"(high)
			: "m"(fCpp) , "m"(size)
			: "eax", "edx", "ecx", "cc"
		);

		*reinterpret_cast<unsigned int*>(&result) = low;
		*reinterpret_cast<unsigned int*>(((unsigned) &result) + 4) = high;
	}

	return result;
}
