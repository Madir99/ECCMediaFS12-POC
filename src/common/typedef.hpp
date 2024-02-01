#pragma once
#define BOOTCODE_SIZE		467

typedef unsigned char		BYTE;
typedef unsigned short		WORD;
typedef unsigned int		DWORD;
typedef unsigned long long	QWORD;

#pragma pack(push, 1)
typedef struct {
	BYTE 	driveNumber;		/* BIOS drive number */					//1
	BYTE 	volumeLabel[11];	/* Volume label */						//11
	BYTE 	fsType[8];			/* FDFS padded with space */			//8
} volumeInfo;															//20

typedef struct {
	BYTE		bootJump[3];	/* Boot strap short or near jump */		//3
	BYTE		systemID[8];	/* Name - can be used to special case
								partition manager volumes */			//8
	BYTE		nodeSize;		/* sectors/node */						//1
	DWORD		sectors;		/* number of sectors */					//4
	WORD		secsTrack;		/* sectors per track */					//2
	BYTE		heads;			/* number of heads */					//1
	DWORD		hidden;			/* hidden sectors (unused) */			//4
	volumeInfo	vi;														//20
	BYTE 		bootCode[BOOTCODE_SIZE];								//512-(20+8+2*4+3+2*2+2*1)=512-45=467
	WORD 		bootSign;												//2
} bootSector;															//512

typedef struct {
	BYTE 	Name[11];	//11
	BYTE 	Attributes;	//1
	WORD 	FirstNode;	//2
	DWORD 	Size;		//4
} directoryEntry;		//18

typedef struct {
	WORD 	NextNodeAndAttributes;	//2
	BYTE 	Data[500];				//500
	BYTE	ECC[10];				//10
} nodeEntry;						//512
#pragma pack(pop)

enum {
	USED = 0x01,
	DIR = 0x02,
	BAD = 0x04,
	COMPRESSED = 0x08
};
