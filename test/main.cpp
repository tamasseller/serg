#include <cstdlib>

int runMlzTests();
int runRansTests();
int runSergTests();

int main()
{
	std::srand(0x1337);

	runMlzTests();
	runRansTests();
	runSergTests();
	return 0;
}
