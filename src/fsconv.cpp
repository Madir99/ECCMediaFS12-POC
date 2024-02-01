#include "common.hpp"
#include <iomanip>

const char old_fsType[9] = "EMFS12  ";

int main(int argc, char ** argv) {
	if (argc < 2) {
		cout << argv[0] << " <disk image>\n";
		return -1;
	}

	fstream disk(argv[1], ios::binary | ios::in | ios::out);
	if (!disk.is_open()) {
		cerr << "Cannot open disk image " << argv[1] << endl;
		return -1;
	}
	
	bootSector bs;
	disk.read(reinterpret_cast<char*>(&bs), sizeof(bootSector));

	if (memcmp(bs.vi.fsType, old_fsType, 8)) {
		memcpy(bs.vi.fsType, fsType, 8);
		cout << "Old FS type converted\n";
	}

	disk.seekg(0, ios::beg);
	disk.write(reinterpret_cast<char*>(&bs), sizeof(bootSector));

	disk.close();
	return 0;
}
