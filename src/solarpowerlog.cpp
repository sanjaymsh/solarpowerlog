//============================================================================
// Name        : solarpowerlog.cpp
// Author      :
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <iostream>
#include "Registry.h"

using namespace std;

int main() {



	cout << "Generating Registry" << endl;
	if (Registry::Instance().LoadConfig("solarpowerlog.conf"))	{
		cout << "Loaded successfully" << endl;
	}
	else {
		cout << "Not successful" << endl;
	}

	libconfig::Setting &set = Registry::Configuration()->getRoot();
	cout << " has the Path \"" << set.getPath() << "\" and Type " <<
		set.getType() << " and has a Lenght of " <<  set.getLength() << endl;

	for ( int i=0; i < set.getLength() ; i ++) {
		libconfig::Setting &s2 = set[i];
		cout << s2.getName() << '\t';
		if (s2.isAggregate()) cout << "is aggregate ";
		if (s2.isArray()) cout << "is array ";
		if (s2.isGroup()) cout << "is group ";
		if (s2.isList()) cout << "is list ";
		if (s2.isNumber()) cout << "is number ";
		if (s2.isScalar())cout << "is scalar ";

		cout << endl;
	}

	return 0;
}
