#pragma once
#include <iostream>
#include <fstream>
#include <cstring>
#include "common/typedef.hpp"

using namespace std;
const char fsType[9] = "EMFS12  ";

BYTE bootJump[3] = { 0xeb, 0x29, 0x90 };

char bootCode[BOOTCODE_SIZE] =
	"\x0e"				/* push cs */
	"\x1f"				/* pop ds */
	"\xbe\x48\x7c"		/* mov si, offset message_txt */
	/* write_msg: */
	"\xac"				/* lodsb */
	"\x22\xc0"			/* and al, al */
	"\x74\x0b"			/* jz key_press */
	"\x56"				/* push si */
	"\xb4\x0e"			/* mov ah, 0eh */
	"\xbb\x07\x00"		/* mov bx, 0007h */
	"\xcd\x10"			/* int 10h */
	"\x5e"				/* pop si */
	"\xeb\xf0"			/* jmp write_msg */
	/* key_press: */
	"\x32\xe4"			/* xor ah, ah */
	"\xcd\x16"			/* int 16h */
	"\xcd\x19"			/* int 19h */
	"\xeb\xfe"			/* foo: jmp foo */
	/* message_txt: */
	"This is not a bootable disk. Please insert a bootable floppy and\r\n"
	"press any key to try again ... \r\n";

void eccCalc(nodeEntry &ne);
bool eccCheck(const nodeEntry &ne);
bool eccFix(nodeEntry &ne);
bool readNode(fstream &disk, nodeEntry &ne);
