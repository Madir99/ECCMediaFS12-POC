#include "common.hpp"

int main(int argc, char ** argv) {
	if (argc < 2) {
		cout << argv[0] << " <disk image> <file to read>" << endl;
		return -1;
	}


	//check + get boot sector data
	fstream disk(argv[1], ios::binary | ios::in | ios::out);
	if (!disk.is_open()) {
		cerr << "Cannot open disk image " << argv[1] << endl;
		return -1;
	}
	
	bootSector bs;
	disk.read(reinterpret_cast<char*>(&bs), sizeof(bootSector));

	if (memcmp(bs.vi.fsType, fsType, 8)) {
		cerr << "EMFS12 is not detected" << endl;
		disk.close();
		return -2;
	}

	//check file name
	char* fx = strrchr(argv[2], '.') + 1;


	if ((!fx && (strlen(argv[2]) > 8)) || (fx && (strlen(argv[2]) > 12 || strlen(fx) > 3))) {
		cerr << "File name is not in 8.3 format" << endl;
		disk.close();
		return -1;
	}

	//convert file name to root dir entry name value
	BYTE den[11];
	for (BYTE i = 0; i < 11;i++) den[i] = 0x20;
	if (fx) {
		memcpy(den, argv[2], fx - argv[2] - 1);
		memcpy(den + 8, fx, strlen(fx));
	} else {
		memcpy(den, argv[2], strlen(argv[2]));
	}

	//additional variables
	//DWORD sector_size = bs.nodeSize * 512;

	//name check and remove + get root dir entry
	nodeEntry de;
	readNode(disk, de);
	if (de.NextNodeAndAttributes != (de.NextNodeAndAttributes | USED | DIR)) {
		cerr << "File system is intensionally irreversibly corrupted" << endl;
		disk.close();
		return -1;
	}

	directoryEntry dev[27];
	memcpy(dev, de.Data, 486);
	WORD NodeN = 0;
	WORD lastRead = 0;
	for (BYTE i = 0; i < 27; i++) {
		if (!memcmp(reinterpret_cast<char*>(dev[i].Name), reinterpret_cast<char*>(den), 11)) {
			NodeN = dev[i].FirstNode;
			lastRead = dev[i].Size % 500;
			break;
		}
	}
	if (!NodeN) {
		cerr << "File not found" <<endl;
		disk.close();
		return -1;
	}

	//file read (add file checks)
	ofstream file(argv[2], ios::binary);
	while (true) {
		disk.seekg((512*NodeN)+512, ios::beg);
		readNode(disk, de);
		NodeN = de.NextNodeAndAttributes >> 4;
		if (NodeN == 0xFFF) break;
		file.write(reinterpret_cast<char*>(de.Data), 500);
	} 
	file.write(reinterpret_cast<char*>(de.Data), lastRead);

	file.close();
	disk.close();
	return 0;
}