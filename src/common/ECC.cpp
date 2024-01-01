#include "typedef.hpp"
#include <cstring>

const WORD power2[10] = {0, 1, 2, 4, 8, 16, 32, 64, 128, 256};

void ne2raw(const nodeEntry &ne, BYTE* data);
void raw2ne(const BYTE* data, nodeEntry &ne);

//calculate extended Hamming (511, 502) code for ECC entry of node entry
void eccCalc(nodeEntry &ne) {
	BYTE data[512];
	ne2raw(ne, data);

	//Hamming (511, 502) code parity 
	for (WORD dataP = 0; dataP < 512; dataP++) {
        for (WORD mask = 1; mask < 512; mask <<= 1) {
            if (dataP & mask) {
                data[mask] ^= data[dataP];
            }
        }
    } 
	
	//global parity
    for (WORD dataP = 0; dataP < 512; dataP++) data[0] ^= data[dataP];

	raw2ne(data,ne);
	return;
}

//Check for data corruption. Returns ture if corrupted.
bool eccCheck(const nodeEntry &ne) {
	BYTE data[512];
	BYTE ecc[10];

	ne2raw(ne, data);
	for (BYTE eccP = 0; eccP < 10; eccP++) {
		ecc[eccP] = 0;
	}

	//ECC reevalution
	for (WORD dataP = 1; dataP < 512; dataP++) {
		ecc[0] ^= data[dataP];
		for (BYTE eccP = 0; eccP < 9; eccP++) {
			WORD mask = 1 << eccP;
			if ((dataP & mask) && (dataP != mask)) {
				ecc[eccP+1] ^= data[dataP];
			}
		}
	}

	//ECC checking
	for (BYTE eccP = 0; eccP < 10; eccP++) {
		if (ecc[eccP] != ne.ECC[eccP]) {
			return true;
		}
	}

	return false;
}

//Tries to fix bit error. Returns true on success.
bool eccFix(nodeEntry &ne) {
	BYTE data[512];
	BYTE eccDiff[10];

	ne2raw(ne, data);
	for (BYTE eccP = 0; eccP < 10; eccP++) {
		eccDiff[eccP] = 0;
	}

	//ECC reevalution
	for (WORD dataP = 0; dataP < 512; dataP++) {
		eccDiff[0] ^= data[dataP];
		for (BYTE eccP = 0; eccP < 9; eccP++) {
			WORD mask = 1 << eccP;
			if ((dataP & mask)) {
				eccDiff[eccP+1] ^= data[dataP];
			}
		}
	}

	//calculation of address of bits that are fliped
	WORD eccAddress[8];
	for(BYTE i = 0; i < 8; i++) {
		BYTE bitMask = 1 << i;
		eccAddress[i] = 0;
		for (BYTE ii = 1; ii < 10; ii++) {
			if (eccDiff[ii] & bitMask) eccAddress[i] += power2[ii];
		}
		//Check for irrepairable state
		if (eccAddress[i] && !(eccDiff[0] & bitMask)) return false;
	}

	//repair of fliped bits
	bool fix = false;
	for (BYTE i = 0; i < 8; i++) {
		if (eccAddress[i]) {
			data[eccAddress[i]] ^= (1 << i);
			fix = true;
		}
	}

	if (fix) {
		//Check for irrepairable state after fix
		BYTE test = 0;
		for (WORD dataP = 0; dataP < 512; dataP++) test ^= data[dataP];
		if (test) return false;
	}

	//byte 0 recalculation because can't be fixed
	for(WORD dataPos = 1; dataPos < 512; dataPos++) {
		data[0] ^= data[dataPos];
	}
	raw2ne(data, ne);

	return true;
}

//converts node Entry to raw data that are formated for extended Hamming (511, 502) code
void ne2raw(const nodeEntry &ne, BYTE* data) {
	WORD dataPos    = 0;
	BYTE eccPos     = 0;
	BYTE power2Pos  = 0;

	for (WORD outPos = 0; outPos < 512; outPos++) {
		if (power2[power2Pos] == outPos) {
			power2Pos++;
			data[outPos] = ne.ECC[eccPos];
			eccPos++;
		} else {
			if (dataPos < 2) {
				data[outPos] = (static_cast<const BYTE*>(static_cast<const void*>(&ne.NextNodeAndAttributes)))[dataPos];
//				^                                 C++ type casting in nutshell XD KEKW                                 ^
			} else {
				data[outPos] = ne.Data[dataPos-2];
			}
			dataPos++;
		}
	}
}

//converts raw data that are formated for extended Hamming (511, 502) code to node entry
void raw2ne(const BYTE* data, nodeEntry &ne) {
	WORD dataPos    = 0;
	BYTE eccPos     = 0;
	BYTE power2Pos  = 0;

	for (WORD inPos = 0; inPos < 512; inPos++) {
		if (power2[power2Pos] == inPos) {
			power2Pos++;
			ne.ECC[eccPos] = data[inPos];
			eccPos++;
		} else {
			if (dataPos < 2) {
				(reinterpret_cast<BYTE*>(&ne.NextNodeAndAttributes))[dataPos] = data[inPos];
			} else {
				ne.Data[dataPos-2] = data[inPos];
			}
			dataPos++;
		}
	}
}