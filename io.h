#ifndef BPSLAB_IO_H
#define BPSLAB_IO_H

#include <global.h>
#include <str.h>
#include <ref.h>

class DataInput
	: public Refable
{
public:
	virtual ~DataInput() {}
	virtual long read(byte *data, long len) { return -1; }
	virtual long seek(long pos, int whence = 0) { return -1; }
	virtual bool seekable() const { return false; }
	virtual long skip(long n) { return -1; }
	virtual long position() const { return -1; }
	virtual long size() const { return -1; }
};

class DataOutput
	: public Refable
{
public:
	virtual ~DataOutput() {}
	virtual long write(const byte *data, long len) { return -1; }
	virtual long seek(long pos, int whence = 0) { return -1; }
	virtual bool seekable() const { return false; }
	virtual long skip(long n) { return -1; }
	virtual long position() const { return -1; }
	virtual void flush() {}
};

template<class tp>
inline static bool ReadData(DataInput* input, tp& data)
{
	long cb = input->read((byte*)&data, sizeof(tp));
	if (cb == -1 || sizeof(tp) != cb)
		return false;
	return true;
}

template<class tp>
inline static bool ReadData(DataInput* input, tp* data, int num)
{
	long cb = input->read(data, num * sizeof(tp));
	if (cb == -1 || num * sizeof(tp) != cb)
		return false;
	return true;
}

template<class tp>
inline static bool WriteData(DataOutput* output, const tp& data)
{
	long cb = output->write((const byte*)&data, sizeof(tp));
	if (cb == -1 || sizeof(tp) != cb)
		return false;
	return true;
}

template<class tp>
inline static bool WriteData(DataOutput* output, const tp* data, int num)
{
	long cb = output->write((const byte*)data, num * sizeof(tp));
	if (cb == -1 || num * sizeof(tp) != cb)
		return false;
	return true;
}

StrongPtr<DataInput> OpenFile(const wstr&);
StrongPtr<DataOutput> CreateFile(const wstr&);

#endif // BPSLAB_IO_H
