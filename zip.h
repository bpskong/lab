#ifndef BPSLAB_ZIP_H
#define BPSLAB_ZIP_H

#include <io.h>

class ZipReader
	: public Refable
{
public:
	virtual ~ZipReader() {}
	virtual bool good() = 0;
	virtual bool exist(const wstr& name) = 0;
	virtual StrongPtr<DataInput> item(const wstr& name) = 0;
public:
	static StrongPtr<ZipReader> open(const wstr& name);
	static StrongPtr<ZipReader> open(DataInput* input);
};

class ZipWritter
	: public Refable
{
public:
	virtual ~ZipWritter() {}
	virtual WeakPtr<DataOutput> addItem(const wstr& name) = 0;
	virtual void flush() = 0;
public:
	static StrongPtr<ZipWritter> create(const wstr& name);
	static StrongPtr<ZipWritter> create(DataOutput* output);
};

#endif // BPSLAB_ZIP_H
