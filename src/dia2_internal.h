#pragma once

extern "C" const IID IID_IDiaDataSource10;
MIDL_INTERFACE("5c7e382a-93b4-4677-a6b5-cc28c3accb96")
IDiaDataSource10: public IDiaDataSource
{
public:
	virtual HRESULT STDMETHODCALLTYPE getRawPDBPtr(void** pppdb) = 0;
	virtual HRESULT STDMETHODCALLTYPE loadDataFromRawPDBPtr(void* ppdb) = 0;
	virtual HRESULT STDMETHODCALLTYPE getStreamSize(LPWSTR stream, unsigned long* pcb) = 0;
	virtual HRESULT STDMETHODCALLTYPE getStreamRawData(LPWSTR stream, unsigned long cbRead, unsigned char* pbData) = 0;
	// ...
};

class PDB1
{
public:
	virtual int32_t QueryInterfaceVersion() = 0;
	virtual int32_t QueryImplementationVersion() = 0;
	virtual BOOL QueryLastError() = 0;
	virtual PWSTR* QueryPDBName() = 0;
	virtual uint32_t QuerySignature() = 0;
	virtual uint32_t QueryAge() = 0;
	virtual void CreateDBI() = 0;
	virtual void OpenDBI() = 0;
	virtual void OpenTpi() = 0;
	virtual void OpenIpi() = 0;
	virtual void Commit() = 0;
	virtual void Close() = 0;
	virtual void OpenStream() = 0;
	virtual void GetEnumStreamNameMap() = 0;
	virtual void GetRawBytes() = 0;
	virtual void QueryPdbImplementationVersion() = 0;
	virtual void OpenDBIEx() = 0;
	virtual void CopyTo() = 0;
	virtual void OpenSrc() = 0;
	virtual void QueryLastErrorExW() = 0;
	virtual void QueryPDBNameExW() = 0;
	virtual BOOL QuerySignature2(GUID* guid) = 0;
	// ...
};