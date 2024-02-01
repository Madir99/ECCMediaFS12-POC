#include "common.hpp"

#define BOOT_SIGN			0xAA55

bootSector bootSec;

int main(int argc, char * argv[]) {
	if (argc < 2) {
		cout << argv[0] << " <disk image> [partision name] [partision size in nodes] [boot sector file]" << endl;
		return -1;
	}

	if (argc > 3) {
		if (atoi(argv[3]) < 2) {
			cerr << "Minimal size in nodes is 2" << endl;
			return -1;
		}
	}

	if (argc > 4) {
		ifstream bootSector(argv[4], ios::binary);
		if (!bootSector.is_open()) {
			cerr << "Cannot open boot sector file " << argv[4] << endl;
			return -1;
		}
		bootSector.seekg(0, ios::end);
		size_t fsize = bootSector.tellg();
		bootSector.seekg(0, ios::beg);
		if (fsize > BOOTCODE_SIZE) {
			cerr << "Boot sector file is too big" << endl;
			bootSector.close();
			return -1;
		}
		if (!bootSector.read(bootCode, fsize)) {
			cerr << "Could not read boot sector file" << endl;
			bootSector.close();
			return -1;
		}
		bootSector.close();
	}

	ofstream disk(argv[1], ios::binary);
	if (!disk.is_open()) {
		cerr << "Cannot open disk image " << argv[1] << endl;
		return -1;
	}

	//boot sector
	bootSec.vi.driveNumber = 0; //0x00 floppy, 0x80 drive
	if (argc > 2) {
		QWORD len = strlen(argv[2]);
		memcpy(bootSec.vi.volumeLabel, "           ", 11);
		if (len <= 11 && len > 0) {
			memcpy(bootSec.vi.volumeLabel, argv[2], len);
		} else {
			cerr << "Invalid partition name" << endl;
			return -1;
		}
	} else {
		memcpy(bootSec.vi.volumeLabel, "NO NAME    ", 11);
	}
	memcpy(bootSec.vi.fsType, fsType, 8);

	memcpy(bootSec.bootJump, bootJump, 3);
	memcpy(bootSec.systemID, "@R80N   ", 8);
	bootSec.nodeSize = 1;
	/*if(argc > 5) {
		if (atoi(argv[5]) > 255) {
			cerr << "Node size can not be bigger than 255" << endl;
			disk.close();
			return -1;
		}
		bootSec.nodeSize = atoi(argv[5]);
	}*/
	bootSec.sectors = 128;
	if(argc > 3) bootSec.sectors = atoi(argv[3]);
	bootSec.sectors *= bootSec.nodeSize;
	bootSec.secsTrack = 0;
	bootSec.heads = 0;
	bootSec.hidden = 0;
	memcpy(bootSec.bootCode, bootCode, BOOTCODE_SIZE);
	bootSec.bootSign = BOOT_SIGN;
	disk.write(reinterpret_cast<char*>(&bootSec), sizeof(bootSector));
	//DWORD sector_size = 512 * bootSec.nodeSize;

	//node 0 aka root dir entry
	nodeEntry directoryEntries;
	directoryEntries.NextNodeAndAttributes = 0xFFF0 | USED | DIR;
	directoryEntry rootDirectory[27];
	for (BYTE i = 0; i < 27; i++) {
		for (BYTE j = 0; j < 11; j++) rootDirectory[i].Name[j] = 0;
		rootDirectory[i].Attributes = 0;
		rootDirectory[i].FirstNode = 0;
		rootDirectory[i].Size = 0;
	}
	if (argc > 2) {
		memcpy(rootDirectory[0].Name, bootSec.vi.volumeLabel, 11);
		rootDirectory[0].Attributes = 0x08;
	}
	for (WORD i = 0; i < 500; i++) directoryEntries.Data[i] = 0;
	for (BYTE i = 0; i < 10; i++) directoryEntries.ECC[i] = 0;
	memcpy(directoryEntries.Data, rootDirectory, 486);
	eccCalc(directoryEntries);
	disk.write(reinterpret_cast<char*>(&directoryEntries), sizeof(nodeEntry));

	//fill
	BYTE sec[512];
	for(WORD i = 0; i < 512; i++) sec[i]=0;
	DWORD missing = bootSec.sectors - (1 + bootSec.nodeSize);
	//missing -= missing % bootSec.nodeSize;
	for(DWORD i = 0; i < missing; i++) disk.write(reinterpret_cast<char*>(&sec), 512);

	disk.close();
	return 0;
}
