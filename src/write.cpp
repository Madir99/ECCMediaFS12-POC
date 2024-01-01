#include "common.hpp"

int main(int argc, char ** argv) {
	if (argc < 2) {
		cout << argv[0] << " <disk image> <file to write>" << endl;
		return -1;
	}

	//fs check + get boot sector data
	fstream disk(argv[1], ios::binary | ios::in | ios::out);
	if (!disk.is_open()) {
		cerr << "Cannot open disk image " << argv[1] << endl;
		return -1;
	}
	
	bootSector bs;
	disk.read(reinterpret_cast<char*>(&bs), 512);

	if (memcmp(bs.vi.fsType, fsType, 8)) {
		cerr << "EMFD12 is not detected" << endl;
		disk.close();
		return -2;
	}

	//separate file name from path
	char* fn = nullptr;
	char* fx = nullptr;
	#ifdef _WIN32
		fn = strrchr(argv[2], '\\') + 1;
	#else
		fn = strrchr(argv[2], '/') + 1;
	#endif
	if (reinterpret_cast<QWORD>(fn) == 1) fn = argv[2];
	fx = strrchr(fn, '.') + 1;

	//check file name
	if ((!fx && (strlen(fn) > 8)) || (fx && (!strlen(fn) || strlen(fn) > 12 || strlen(fx) > 3))) {
		cerr << "File name is not in 8.3 format" << endl;
		disk.close();
		return -1;
	}

	//convert file name to root dir entry name value
	uint8_t den[11];
	for (uint8_t i = 0; i < 11;i++) den[i] = 0x20;
	if (fx) {
		memcpy(den, fn, fx - fn - 1);
		memcpy(den + 8, fx, strlen(fx));
	} else {
		memcpy(den, fn, strlen(fn));
	}

	//file access
	ifstream file(argv[2], ios::binary);
	if (!file.is_open()) {
		cerr << "Cannot read file " << argv[2] << endl;
		disk.close();
		return -1;
	}

	//get size file info
	file.seekg(0, ios::end);
	QWORD fileS = file.tellg();
	file.seekg(0, ios::beg);

	//read file content
	QWORD nodes = (fileS+499)/500;
	nodeEntry * dataD = reinterpret_cast<nodeEntry*>(malloc(nodes*sizeof(nodeEntry)));
	for (QWORD i = 0; i < (nodes-1); i++) {
		file.read(reinterpret_cast<char*>(dataD[i].Data),sizeof(nodeEntry::Data));
	}
	file.read(reinterpret_cast<char*>(dataD[nodes-1].Data),fileS - file.tellg());
	dataD[nodes-1].NextNodeAndAttributes = 0xFFF0 | USED;
	eccCalc(dataD[nodes-1]);
	file.close();

	// check max file system size
	if (0xffffffff < fileS) {
		cerr << "File is too big for filesystem" << endl;
		free(dataD);
		disk.close();
		return -1;
	}

	//free space and name check + get root dir entry
	disk.seekg(0, ios::end);
	QWORD diskS = disk.tellg();
	disk.seekg(512, ios::beg);
	nodeEntry de;
	if (readNode(disk, de)) {
		free(dataD);
		disk.close();
		return -1;
	}
	if (de.NextNodeAndAttributes != (de.NextNodeAndAttributes | USED | DIR)) {
		cerr << "File system is intensionally irreversibly corrupted" << endl;
		free(dataD);
		disk.close();
		return -1;
	}
	directoryEntry dev[27];
	memcpy(&dev, &(de.Data), 486);
	QWORD used = 0;
	for (uint8_t i = 0; i < 27; i++) {
		used += (dev[i].Size + 499) / 500;
		if (!memcmp(dev[i].Name, den, 11)) {
			cerr << "File exists" << endl;
			free(dataD);
			disk.close();
			return -1;
		}
	}
	used *= 512;
	used += disk.tellg();
	if ((diskS - used) < fileS) {
		cerr << "Not enough space for file" << endl;
		free(dataD);
		disk.close();
		return -1;

	}

	//get first free node
	uint16_t firstNode = 1;
	nodeEntry node;
	if (readNode(disk, node)) {
		free(dataD);
		disk.close();
		return -1;
	}
	while (node.NextNodeAndAttributes != 0) {
		disk.read(reinterpret_cast<char*>(&node), sizeof(nodeEntry));
		if (readNode(disk, node)) {
			free(dataD);
			disk.close();
			return -1;
		}
		firstNode++;
	}

	//modify root dir entry
	for (uint8_t i = 0; i < 27; i ++) {
		if (dev[i].FirstNode == 0 && dev[i].Attributes != 0x08) {
			dev[i].FirstNode = firstNode;
			dev[i].Size = (DWORD)fileS;
			dev[i].Attributes = 0x20;
			memcpy(dev[i].Name, den, 11);
			break;
		}
	}
	memcpy(de.Data, dev, 486);
	for (BYTE i = 0; i < 10; i++) de.ECC[i] = 0;
	eccCalc(de);

	//write root dir entry
	disk.seekg(512, ios::beg);
	disk.write(reinterpret_cast<char*>(&de), 512);

	//write file content to partition image
	nodeEntry seekN;
	WORD currNode = firstNode;
	for (DWORD i = 0; i < (nodes-1); i++) {
		disk.seekg(512 * (currNode + 1), ios::beg);
		if (readNode(disk, seekN)) {
			free(dataD);
			disk.close();
			return -1;
		}
		dataD[i].NextNodeAndAttributes = currNode + 1;
		while (seekN.NextNodeAndAttributes != 0) {
			if (readNode(disk, seekN)) {
				free(dataD);
				disk.close();
				return -1;
			}
			dataD[i].NextNodeAndAttributes++;
		}
		currNode = dataD[i].NextNodeAndAttributes;
		dataD[i].NextNodeAndAttributes = dataD[i].NextNodeAndAttributes << 4 | USED;
		eccCalc(dataD[i]);
	}

	currNode = firstNode;
	for (DWORD i = 0; i < nodes; i++) {
		disk.seekg(512*(currNode+1), ios::beg);
		currNode = dataD[i].NextNodeAndAttributes >> 4;
		disk.write(reinterpret_cast<char*>(dataD+i), sizeof(nodeEntry));
	}

	free(dataD);
	disk.close();
	return 0;
}