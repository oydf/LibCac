#include<iostream>
#include<string>
#include"Timer.hpp"

using namespace std;

int main()
{

	Timer t;
	cout << t.getMicrosecondsInterval() << endl;
	cout << "+----------------------------------+" << endl;
	cout << t.getMillsecondsInterval() << endl;
	cout << "+----------------------------------+" << endl;
	cout << t.getSecondsInterval() << endl;
	cout << "+----------------------------------+" << endl;
	cout << t.getMicrosecondsInterval() << endl;
	cout << "+----------------------------------+" << endl;
	cout << t.getSecondsInterval() << endl;
	t.reset();
	cout << t.getMicrosecondsInterval() << endl;
	cout << "+----------------------------------+" << endl;
	cout << t.getMillsecondsInterval() << endl;
	cout << "+----------------------------------+" << endl;
	cout << t.getSecondsInterval() << endl;
	cout << "+----------------------------------+" << endl;
	cout << t.getMicrosecondsInterval() << endl;
	cout << "+----------------------------------+" << endl;
	cout << t.getSecondsInterval() << endl;
	system("pause");
	return 0;
}