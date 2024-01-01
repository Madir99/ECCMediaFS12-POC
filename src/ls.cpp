#include "common.hpp"
#include <iomanip>

int main(int argc, char ** argv) {
	if (argc < 2) {
		cout << argv[0] << " <disk image>" << endl;
		return -1;
	}

	fstream disk(argv[1], ios::binary | ios::in | ios::out);
	if (!disk.is_open()) {
		cerr << "Cannot open disk image " << argv[1] << endl;
		return -1;
	}
	
	bootSector bs;
	disk.read(reinterpret_cast<char*>(&bs), sizeof(bootSector));

	if (memcmp(bs.vi.fsType, fsType, 8)) {
		cerr << "EMFD12 is not detected" << endl;
		disk.close();
		return -2;
	}

	nodeEntry de;

	if (readNode(disk, de)) {
		disk.close();
		return -1;
	}

	if (de.NextNodeAndAttributes != (de.NextNodeAndAttributes | USED | DIR)) {
		cerr << "File system is intensionally irreversibly corrupted" << endl;
		disk.close();
		return -1;
	}

	directoryEntry dev[27];
	memcpy(dev, de.Data, 486);

	BYTE name[12];
	name[11] = 0;
	memcpy(name, bs.vi.volumeLabel, 11);
	BYTE OEM[9];
	memcpy(OEM, bs.systemID, 8);
	OEM[8] = 0;

	cout << "Partition named " << setw(11) << name << " is formated by " << setw(8) << OEM << " and contais these files:" << setw(0) << endl;
	cout << "FILE NAME    EXTENSION    SIZE" << endl;

	QWORD used = 0;

	for (BYTE i = 0; i < 27; i++) {
		if (dev[i].Attributes == 0x20) {
			BYTE fname[9];
			BYTE fext[4];
			memcpy(fname, dev[i].Name, 8);
			memcpy(fext, dev[i].Name + 8, 3);
			fname[8] = 0;
			fext[3] = 0;
			cout << fname << "     " << fext << "          " << dev[i].Size << endl;
		}
		used += (dev[i].Size + 499) / 500;
	}
	if (used == 0){
		cout << "No Files Found" << endl;
	}
	used *= 512;
	used += disk.tellg();

	disk.seekg(0, ios::end);
	QWORD fsize = disk.tellg();
	disk.seekg(0, ios::beg);

	cout << used << " bytes from " << fsize << " bytes is used (" << ((used * 100) / fsize) << "%)" << endl;

	disk.close();

	return 0;
}