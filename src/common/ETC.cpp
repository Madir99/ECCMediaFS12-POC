#include "typedef.hpp"
#include <iostream>
#include <fstream>
#include <cstring>

using namespace std;

bool eccCheck(const nodeEntry &ne);
bool eccFix(nodeEntry &ne);

//read node entry and do ECC checks & fixes. Return ture at fail.
bool readNode(fstream &disk, nodeEntry &ne) {
	disk.read(reinterpret_cast<char*>(&ne), sizeof(nodeEntry));
	if (eccCheck(ne)) {
		cout << "Detected file system corruption" << endl;
		if (!eccFix(ne)) {
			cerr << "File system is irreversibly corrupted" << endl;
			return true;
		}
		disk.seekg(-512, ios::cur);
		disk.write(reinterpret_cast<char*>(&ne), 512);
		cout << "File system corruption fixed" << endl;
	}
    return false;
}