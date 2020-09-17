#pragma once

const unsigned int PDB_FILE_INFO_MAGIC = 0x53445352 /* RSDS */;
struct PdbFileInfo
{
	DWORD magic; // RSDS
	GUID signature;
	DWORD age;
	// ...
};