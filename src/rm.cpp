#include "common.hpp"

int main(int argc, char ** argv) {
	if (argc < 2) {
		cout << argv[0] << " <disk image> <file to remove>" << endl;
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
	for (BYTE i = 0; i < 27; i++) {
		if (!memcmp(reinterpret_cast<char*>(dev[i].Name), reinterpret_cast<char*>(den), 11)) {
			NodeN = dev[i].FirstNode;
			for (BYTE ii = 0; ii < 18; ii++) {
				(reinterpret_cast<char*>(dev))[(18*i)+ii] = 0;
			}
			break;
		}
	}
	if (!NodeN) {
		cerr << "File not found" <<endl;
		disk.close();
		return -1;
	}
	memcpy(de.Data, dev, 486);
	eccCalc(de);

	//write root dir entry
	disk.seekg( 512, ios::beg);
	disk.write(reinterpret_cast<char*>(&de), 512);

	//mark nodes as free (add file checks)
	//wfs_node_entry * node = calloc(1, sector_size);
	while (NodeN != 0xFFF) {
		disk.seekg((512*NodeN)+512, ios::beg);
		readNode(disk, de);
		disk.seekg((512*NodeN)+512, ios::beg);
		NodeN = de.NextNodeAndAttributes >> 4;
		de.NextNodeAndAttributes = 0;
		eccCalc(de);
		disk.write(reinterpret_cast<char*>(&de), 512);
	} 

	disk.close();
	return 0;
}