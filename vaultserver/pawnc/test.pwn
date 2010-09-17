#include <vaultmp>

main()
{
print("Text aus dem ersten VaultScript ueberhaupt.\n", 1, -1, 1);
new Float: asdf;
asdf = 0.5666;
printf("float test: %f\n", asdf);
new blub[32] = "asdfkokolores";
if (strcmp(blub, "asdfkokolores") == 0)
	print("string test: asdf!\n");
}

public OnClientAuthenticate(const name[], const pwd[])
{
	printf("OnClientAuthenticate: name: %s, pwd: %s\n", name, pwd);
	return 1;
}