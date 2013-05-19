#include <io.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

class FileOutput
	: public DataOutput
{
public:
	FileOutput(const wstr& filePath)
	{
		str fn = ws2s(filePath);
		_fd = ::open(fn.c_str(), O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);
	}
	~FileOutput()
	{
		::close(_fd);
	}
public:
	long write(const byte *data, long len)
	{
		return ::write(_fd, data, len);
	}
	long seek(long pos, int whence = SEEK_SET)
	{
		return ::lseek(_fd, pos, whence);
	}
	long skip(long n)
	{
		return ::lseek(_fd, n, SEEK_CUR);
	}
	long position() const
	{
		return ::lseek(_fd, 0, SEEK_CUR);
	}
	bool seekable() const
	{
		return true;
	}
private:
	int _fd;
};

class FileInput
	: public DataInput
{
public:
	FileInput(const wstr& filePath)
	{
		str fn = ws2s(filePath);
		_fd = open(fn.c_str(), O_RDONLY);
		struct stat buf = {0};
		stat(fn.c_str(), &buf);
		_flen = buf.st_size;
	}
	~FileInput()
	{
		::close(_fd);
	}
public:
	long read(byte *data, long len)
	{
		return ::read(_fd, data, len);
	}
	long seek(long pos, int whence = SEEK_SET)
	{
		return ::lseek(_fd, pos, whence);
	}
	long skip(long n)
	{
		return ::lseek(_fd, n, SEEK_CUR);
	}
	long position() const
	{
		return ::lseek(_fd, 0, SEEK_CUR);
	}
	long size() const
	{
		return _flen;
	}
	bool seekable() const
	{
		return true;
	}
private:
	int _fd;
	long _flen;
};

StrongPtr<DataInput> OpenFile(const wstr& name)
{
	return new FileInput(name);
}

StrongPtr<DataOutput> CreateFile(const wstr& name)
{
	return new FileOutput(name);
}
