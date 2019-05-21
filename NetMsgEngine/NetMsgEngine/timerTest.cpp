#include<iostream>
#include<string>
#include"Timer.hpp"
#include<thread>
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
	std::chrono::milliseconds dura(2000);
	std::this_thread::sleep_for(dura);
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