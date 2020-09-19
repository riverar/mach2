#pragma once

struct PdbStreamHeader {
	uint32_t version;
	uint32_t signature;
	uint32_t age;
	GUID guid;
};