#include "ScriptFunction.h"

ScriptFunction::ScriptFunction(ScriptFunc fCpp, string def) : fCpp(fCpp), def(def), pawn(false)
{

}

ScriptFunction::ScriptFunction(ScriptFuncPAWN fPawn, AMX* amx, string def) : fPawn(fPawn), amx(amx), def(def), pawn(true)
{

}

unsigned long long ScriptFunction::Call(const vector<boost::any>& args)
{
	unsigned long long result = 0x0000000000000000;

	if (def.length() != args.size())
		throw VaultException("Script call: Number of arguments does not match definition");

	if (pawn)
		result = PAWN::Call(amx, fPawn.c_str(), def.c_str(), args);

	else
	{
		// cdecl convention
		unsigned int size = 0;
		string::reverse_iterator it;
		vector<boost::any>::const_reverse_iterator it2;

		try
		{
			asm(
				"SUB ESP,8\n"
				:
				:
				:
			);

			for (it = def.rbegin(), it2 = args.rbegin(); it != def.rend(); ++it, ++it2)
			{
				switch (*it)
				{
					case 'i':
					{
						unsigned int value = boost::any_cast<unsigned int>(*it2);

						asm(
							"ADD ESP,8\n"
							"PUSH %0\n"
							"SUB ESP,8\n"
							:
							: "m"(value)
							:
						);

						size += sizeof(unsigned int);
						break;
					}

					case 'l':
					{
						unsigned long long value = boost::any_cast<unsigned long long>(*it2);

						asm(
							"FILD %0\n"
							"FISTP QWORD PTR [ESP]\n"
							"SUB ESP,8\n"
							:
							: "m"(value)
							:
						);

						size += sizeof(unsigned long long);
						break;
					}

					case 'f':
					{
						double value = boost::any_cast<double>(*it2);

						asm(
							"FLD %0\n"
							"FSTP QWORD PTR [ESP]\n"
							"SUB ESP,8\n"
							:
							: "m"(value)
							:
						);

						size += sizeof(double);
						break;
					}

					case 'p':
					{
						void* value = boost::any_cast<void*>(*it2);

						asm(
							"ADD ESP,8\n"
							"PUSH %0\n"
							"SUB ESP,8\n"
							:
							: "m"(value)
							:
						);

						size += sizeof(void*);
						break;
					}

					case 's':
					{
						const string& value = boost::any_cast<string>(*it2);

						asm(
							"ADD ESP,8\n"
							"PUSH %0\n"
							"SUB ESP,8\n"
							:
							: "r"(value.c_str())
							:
						);

						size += sizeof(const char*);
						break;
					}

					default:
						throw VaultException("C++ call: Unknown argument identifier %02X", *it);
				}
			}
		}

		catch (...)
		{
			asm(
				"ADD ESP,%0\n"
				:
				: "r"(size + 8)
				:
			);
			throw;
		}

		unsigned int low;
		unsigned int high;

		asm(
			"ADD ESP,8\n"
			"CALL %2\n"
			"ADD ESP,%3\n"
			"MOV %0,EAX\n"
			"MOV %1,EDX\n"
			: "=m"(low), "=m"(high)
			: "m"(fCpp) , "m"(size)
			: "eax", "edx"
		);

		*reinterpret_cast<unsigned int*>(&result) = low;
		*reinterpret_cast<unsigned int*>(((unsigned) &result) + 4) = high;
	}

	return result;
}
