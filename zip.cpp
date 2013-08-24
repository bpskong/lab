#include <zip.h>
#include <zlib.h>
#include <map>
#include <vector>
#include <memory.h>
#include <assert.h>

#define BUFSIZE 4096
typedef std::vector<byte> ByteArray;

#pragma pack(1)
struct Zip64ExtraField
{
	uint16_t tag;
	uint16_t sizeOfZip64ExtraField;
	uint64_t uncompressedSize;
	uint64_t compressedSize;
	uint64_t offsetOfLocalHeader;
	uint32_t diskNum;

	Zip64ExtraField()
	{
		::memset(this, 0, 0);
	}
	bool parse(DataInput* input)
	{
		return ReadData(input, *this);
	}
};

struct Zip64EndOfCentralDirectoryLocator
{
	uint32_t signature;
	uint32_t numOfDisk;
	uint64_t relativeOffsetOfCentralDirectory;
	uint16_t totalNumOfDisk;

	Zip64EndOfCentralDirectoryLocator()
	{
		::memset(this, 0, 0);
	}
	bool parse(DataInput* input)
	{
		return ReadData(input, *this);
	}
};

struct Zip64EndOfCentralDirectory
{
	uint32_t signature;
	uint64_t sizeOfZip64EndOfCentralDirectory;
	uint16_t versionMadeBy;
	uint16_t versionNeededToExtract;
	uint32_t diskNum;
	uint32_t diskNumOfCentralDirectory;
	uint64_t totalEntriesOnThisDisk;
	uint64_t totalEntries;
	uint64_t sizeOfCentralDirectory;
	uint64_t startOfCentralDirectory;

	Zip64EndOfCentralDirectory()
	{
		::memset(this, 0, 0);
	}

	bool parse(DataInput* input)
	{
		return ReadData(input, *this);
	}
};

struct EndOfCentralDirectory
{
	uint32_t signature;
	uint16_t diskNum;
	uint16_t diskNumOfCentralDirectory;
	uint16_t totalEntriesOnThisDisk;
	uint16_t totalEntries;
	uint32_t sizeOfCentralDirectory;
	uint32_t startOfCentralDirectory;
	uint16_t fileCommentLength;

	EndOfCentralDirectory()
	{
		signature = 0x06054B50;
		diskNum = 0;
		diskNumOfCentralDirectory = 0;
		totalEntriesOnThisDisk = 0;
		totalEntries = 0;
		sizeOfCentralDirectory = 0;
		startOfCentralDirectory = 0;
		fileCommentLength = 0;
	}

	bool parse(DataInput* input)
	{
		return ReadData(input, *this);
	}

	bool write(DataOutput* output)
	{
		return WriteData(output, *this);
	}
};

struct LocalFileHeader
{
	uint32_t signature;
	uint16_t versionNeededToExtract;
	uint16_t generalPurposeBitFlag;
	uint16_t compressionMethod;
	uint32_t lastModFileDateTime;
	uint32_t crc32;
	uint32_t compressedSize;
	uint32_t uncompressedSize;
	uint16_t fileNameLength;
	uint16_t extraFieldLength;

	LocalFileHeader(bool floder = false)
	{
		signature = 0x04034B50;
		versionNeededToExtract = floder ? 10 : 20;
		generalPurposeBitFlag = 0;
		compressionMethod = 0;
		lastModFileDateTime = 0x40E24E87;
		crc32 = 0;
		compressedSize = 0;
		uncompressedSize = 0;
		fileNameLength = 0;
		extraFieldLength = 0;
	}

	bool parse(DataInput* input)
	{
		return ReadData(input, *this);
	}

	bool write(DataOutput* output)
	{
		return WriteData(output, *this);
	}
};

struct CentralDirectoryFileHeader
{
	uint32_t signature;
	uint16_t versionMadeBy;
	uint16_t versionNeededToExtract;
	uint16_t generalPurposeBitFlag;
	uint16_t compressionMethod;
	uint32_t lastModFileDateTime;
	uint32_t crc32;
	uint32_t compressedSize;
	uint32_t uncompressedSize;
	uint16_t fileNameLength;
	uint16_t extraFieldLength;
	uint16_t fileCommentLength;
	uint16_t diskNumberStart;
	uint16_t internalFileAttributes;
	uint32_t externalFileAttributes;
	uint32_t relativeOffsetOfLocalHeader;

	CentralDirectoryFileHeader(bool floder = false)
	{
		signature = 0x2014B50;
		versionMadeBy = 20;
		versionNeededToExtract = floder ? 10 : 20;
		generalPurposeBitFlag = 0;
		compressionMethod = floder ? 0 : Z_DEFLATED;
		lastModFileDateTime = 0x40E24E87;
		crc32 = 0;
		compressedSize = 0;
		uncompressedSize = 0;
		fileNameLength = 0;
		extraFieldLength = 0;
		fileCommentLength = 0;
		diskNumberStart = 0;
		internalFileAttributes = floder ? 0 : 1;
		externalFileAttributes = floder ? 0x10 : 0x20;
		relativeOffsetOfLocalHeader = 0;
	}

	bool parse(DataInput* input)
	{
		return ReadData(input, *this);
	}

	bool write(DataOutput* output)
	{
		return WriteData(output, *this);
	}

	bool isFloder()
	{
		return (internalFileAttributes == 0);
	}
};
#pragma pack()

typedef std::map<str, CentralDirectoryFileHeader*> FileHeaders;

class ZipOutput
	: public DataOutput
{
public:
	ZipOutput(DataOutput* output, CentralDirectoryFileHeader* header, EndOfCentralDirectory* endOfCentralDirectory, uint32_t begin)
	{
		_alreadyFlush = false;
		_header = header;
		_dstOutput = output;
		_endOfCentralDirectory = endOfCentralDirectory;
		_cbDeflated = 0;
		_begin = begin;
		_buffer.resize(BUFSIZE);
		::memset(&_zlibStream, 0, sizeof(z_stream));
		::deflateInit2(&_zlibStream, Z_DEFAULT_COMPRESSION,
			Z_DEFLATED, -MAX_WBITS, MAX_MEM_LEVEL, Z_DEFAULT_STRATEGY);
		_zlibStream.next_out = (Bytef*)&_buffer[0];
		_zlibStream.avail_out = (uInt)BUFSIZE;
	}

	~ZipOutput()
	{
	}

public:
	long write(const byte *data, long len)
	{
		_deflate(data, len, false);
		return len;
	}

	void flush()
	{
		if (_alreadyFlush)
			return;
		_deflate(0, 0, true);

		//
		// update current local file header record
		//
		_dstOutput->seek(_begin + _header->relativeOffsetOfLocalHeader);
		LocalFileHeader localFileHeader;
		localFileHeader.crc32 = _header->crc32;
		localFileHeader.compressedSize = _header->compressedSize;
		localFileHeader.uncompressedSize = _header->uncompressedSize;
		localFileHeader.compressionMethod = _header->compressionMethod;
		localFileHeader.fileNameLength = _header->fileNameLength;
		localFileHeader.write(_dstOutput);

		//
		// update offset of start of central directory
		//
		_dstOutput->skip(_header->fileNameLength + _header->compressedSize);
		_endOfCentralDirectory->startOfCentralDirectory += _header->compressedSize;

		::deflateEnd(&_zlibStream);
		_alreadyFlush = true;
	}

private:
	bool _deflate(const void *pv, long cb, bool flush)
	{
		_zlibStream.next_in = (Bytef*)pv;
		_zlibStream.avail_in = (uInt)cb;

		if (pv != 0 && cb > 0)
			_header->crc32 = crc32(_header->crc32, (Bytef*)pv, (uInt)cb);

		bool finished = false;
		do
		{
			uint32_t outBefore = _zlibStream.total_out;
			int err = deflate(&_zlibStream, flush ? Z_FINISH : Z_NO_FLUSH);
			uint32_t outAfter = _zlibStream.total_out;
			_cbDeflated += (outAfter - outBefore);

			if (flush || _zlibStream.avail_out == 0)
			{
				if (_cbDeflated > 0)
				{
					_dstOutput->write(&_buffer[0], _cbDeflated);
					_header->compressedSize += _cbDeflated;
					_header->uncompressedSize += _zlibStream.total_in;
					_zlibStream.total_in = 0;
					_cbDeflated = 0;
				}
				_zlibStream.next_out = (Bytef*)&_buffer[0];
				_zlibStream.avail_out = (uInt)BUFSIZE;
			}

			if (_zlibStream.avail_in != 0 || (flush && err != Z_STREAM_END))
				finished = false;
			else
				finished = true;

		} while (!finished);

		return true;
	}

	bool _alreadyFlush;
	uint32_t _cbDeflated;
	uint32_t _begin;
	z_stream _zlibStream;
	DataOutput* _dstOutput;
	ByteArray _buffer;
	EndOfCentralDirectory* _endOfCentralDirectory;
	CentralDirectoryFileHeader* _header;
};

class ZipWritterImpl
	: public ZipWritter
{
public:
	ZipWritterImpl(const wstr& fname)
	{
		_srcOffset = 0;
		_fileName = fname;
		_alreadyFlush = false;
	}

	ZipWritterImpl(DataOutput* output)
	{
		assert(output && output->seekable());
		_srcOffset = output->position();
		_dstOutput = output;
		_alreadyFlush = false;
	}

	~ZipWritterImpl()
	{
		flush();
		FileHeaders::iterator it = _fileHeaders.begin();
		for (; it != _fileHeaders.end(); it++)
			delete it->second;
	}

	WeakPtr<DataOutput> addItem(const wstr& name)
	{
		WeakPtr<DataOutput> wpItem;
		if (name.empty())
			return wpItem;
		if (!_dstOutput)
			_dstOutput = CreateFile(_fileName);
		str path = ws2s(name);
		_addFloders(path);
		if (path.at(path.length() - 1) != '/')
		{
			_flushItem();
			wpItem = _addItem(path, false);
		}
		return wpItem;
	}

	void flush()
	{
		if (_alreadyFlush || !_dstOutput)
			return;
		_flushItem();
		FileHeaders::iterator it = _fileHeaders.begin();
		for (; it != _fileHeaders.end(); it++)
		{
			str name = it->first;
			CentralDirectoryFileHeader* header = it->second;
			header->write(_dstOutput.get());
			WriteData(_dstOutput.get(), &name[0], name.length());
			_endOfCentralDirectory.sizeOfCentralDirectory +=
				(sizeof(CentralDirectoryFileHeader) +
				header->fileNameLength +
				header->extraFieldLength +
				header->fileCommentLength);
			_endOfCentralDirectory.totalEntriesOnThisDisk++;
			_endOfCentralDirectory.totalEntries++;
		}
		_endOfCentralDirectory.write(_dstOutput.get());
		_alreadyFlush = true;
	}

private:
	void _flushItem()
	{
		if (_currentItem.get())
		{
			_currentItem->flush();
			_currentItem.clear();
		}
	}

	void _addFloders(str name)
	{
		str folder;
		str::size_type pos = name.find("/");
		while (pos != str::npos)
		{
			folder += name.substr(0, pos + 1);
			_addItem(folder);
			name = name.substr(pos + 1, name.length() - folder.length());
			pos = name.find("/");
		}
	}

	DataOutput* _addItem(const str& name, bool floder = true)
	{
		if (_fileHeaders.find(name) != _fileHeaders.end())
			return NULL;
		LocalFileHeader local(floder);
		local.fileNameLength = name.length();
		if (!local.write(_dstOutput.get()) || !WriteData(_dstOutput.get(), &name[0], name.length()))
			return NULL;
		CentralDirectoryFileHeader* fileHeader = new CentralDirectoryFileHeader(floder);
		fileHeader->fileNameLength = name.length();
		fileHeader->relativeOffsetOfLocalHeader = _endOfCentralDirectory.startOfCentralDirectory;
		_fileHeaders.insert(std::make_pair(str(name.begin(), name.end()), fileHeader));
		_endOfCentralDirectory.startOfCentralDirectory +=
			(sizeof(LocalFileHeader) + local.fileNameLength + local.extraFieldLength);
		if (floder)
			return NULL;
		_currentItem = new ZipOutput(_dstOutput.get(), fileHeader, &_endOfCentralDirectory, _srcOffset);
		return _currentItem.get();
	}

private:
	bool _alreadyFlush;
	long _srcOffset;
	EndOfCentralDirectory _endOfCentralDirectory;
	FileHeaders _fileHeaders;
	StrongPtr<ZipOutput> _currentItem;
	StrongPtr<DataOutput> _dstOutput;
	wstr _fileName;
};

class ZipInput
	: public DataInput
{
public:
	ZipInput(DataInput* input, CentralDirectoryFileHeader* header, uint32_t begin)
	{
		_srcInput = input;
		_begin = begin;
		_header = header;
		_offset = 0;
		_restCompressed = _header->compressedSize;
		_restUnCompressed = _header->uncompressedSize;
		_buffer.resize(BUFSIZE);
		::memset(&_zlibStream, 0, sizeof(z_stream));
		::inflateInit2(&_zlibStream, -MAX_WBITS);
	}

	~ZipInput()
	{
		::inflateEnd(&_zlibStream);
	}

public:
	long read(byte *data, long len)
	{
		assert(_header->compressionMethod == 8);
		_zlibStream.next_out = (Bytef*)data;
		_zlibStream.avail_out = std::min((uint32_t)len, _restUnCompressed);
		uint32_t cbReaded = 0;
		while (_zlibStream.avail_out > 0)
		{
			if (_zlibStream.avail_in == 0 && _restCompressed > 0)
			{
				uint32_t cb = std::min((uint32_t)BUFSIZE, _restCompressed);
				_srcInput->seek(_begin + _offset);
				long cbInput = _srcInput->read(&_buffer[0], cb);
				_offset += cbInput;
				_restCompressed -= cbInput;
				_zlibStream.next_in = &_buffer[0];
				_zlibStream.avail_in = cbInput;
			}
			uint32_t outBefore = _zlibStream.total_out;
			int err = ::inflate(&_zlibStream, Z_SYNC_FLUSH);
			uint32_t outAfter = _zlibStream.total_out;
			uint32_t currentSize = outAfter - outBefore;
			_restUnCompressed -= currentSize;
			cbReaded += currentSize;
			if (err == Z_STREAM_END)
				break;
		}
		return cbReaded;
	}

	long size() const
	{
		return _header->uncompressedSize;
	}

private:
	z_stream _zlibStream;
	DataInput* _srcInput;
	uint32_t _begin;
	uint32_t _offset;
	uint32_t _restCompressed;
	uint32_t _restUnCompressed;
	CentralDirectoryFileHeader* _header;
	ByteArray _buffer;
};

class ZipReaderImpl
	: public ZipReader
{
public:
	ZipReaderImpl(const wstr& fname)
	{
		_fileName = fname;
		_vaild = -1;
		_srcOffset = 0;
	}

	ZipReaderImpl(DataInput* input)
	{
		assert(input && input->seekable());
		_srcInput = input;
		_vaild = -1;
		_srcOffset = input->position();
	}

	~ZipReaderImpl()
	{
		FileHeaders::iterator it = _fileHeaders.begin();
		for (; it != _fileHeaders.end(); it++)
			delete it->second;
		_srcInput.clear();
	}

	bool good()
	{
		return _ensureValid();
	}

	StrongPtr<DataInput> item(const wstr& name)
	{
		CentralDirectoryFileHeader* header = _fileHeader(name);
		if (!header)
			return NULL;
		_srcInput->seek(_srcOffset + header->relativeOffsetOfLocalHeader);
		LocalFileHeader localFileHeader;
		if (!localFileHeader.parse(_srcInput.get()))
			return NULL;

		uint32_t offsetOfExtra =
				_srcOffset +
				header->relativeOffsetOfLocalHeader +
				sizeof(LocalFileHeader) +
				localFileHeader.fileNameLength;

		if (localFileHeader.versionNeededToExtract == 45)
		{
			_srcInput->seek(offsetOfExtra);
			Zip64ExtraField extraField;
			extraField.parse(_srcInput.get());
			header->compressedSize = extraField.compressedSize;
			header->uncompressedSize = extraField.uncompressedSize;
		}

		uint32_t dataOffset = offsetOfExtra + localFileHeader.extraFieldLength;
		_srcInput->seek(dataOffset);
		return new ZipInput(_srcInput.get(), header, dataOffset);
	}

	bool exist(const wstr& name)
	{
		return (_fileHeader(name) != NULL);
	}

private:
	CentralDirectoryFileHeader* _fileHeader(const wstr& name)
	{
		if (!_ensureValid())
			return NULL;
		FileHeaders::iterator it = _fileHeaders.find(ws2s(name));
		if (it == _fileHeaders.end())
			return NULL;
		return it->second;
	}

	bool _seekEndOfCentralDirectory(DataInput* input)
	{
		byte buffer[BUFSIZE + 16];
		long length = input->size();
		for (long i = length; i > 0; i -= BUFSIZE)
		{
			long offset = _srcOffset + std::max((long)0, (i - BUFSIZE));
			input->seek(offset);
			input->read(buffer, BUFSIZE + 0x16);
			long bufferOffsetFromEndOfStream = length - offset;
			for (int j = BUFSIZE - 0x16; j >= 0; j--)
			{
				if (buffer[j + 0] ==  0x50 && buffer[j + 1] ==  0x4b &&
					buffer[j + 2] ==  0x05 && buffer[j + 3] ==  0x06)
				{
					long n1 = buffer[j + 0x16 - 2] + (buffer[j + 0x16 - 1] << 8);
					long n2 = bufferOffsetFromEndOfStream - j - 0x16;
					if (n1 == n2)
					{
						input->seek(offset + j);
						return true;
					}
				}
			}
		}
		return false;
	};

	bool _ensureValid()
	{
		if (_vaild != -1)
			return (_vaild == 1);
		_vaild = 0;
		if (_srcInput == NULL)
			_srcInput = OpenFile(_fileName);
		if (!_seekEndOfCentralDirectory(_srcInput.get()))
			return false;
		long pos = _srcInput->position();
		if (!_endOfCentralDirectory.parse(_srcInput.get()))
			return false;

		uint16_t totalEntries = 0;
		uint32_t startOfCentralDirectory = 0;
		if (_endOfCentralDirectory.totalEntries == 0xffff)
		{
			Zip64EndOfCentralDirectoryLocator zip64Locator;
			Zip64EndOfCentralDirectory zip64EndOfCentralDirectory;

			_srcInput->seek(pos - 20);
			if (!zip64Locator.parse(_srcInput.get()))
				return false;
			_srcInput->seek(zip64Locator.relativeOffsetOfCentralDirectory);
			if (!zip64EndOfCentralDirectory.parse(_srcInput.get()))
				return false;

			totalEntries = zip64EndOfCentralDirectory.totalEntries;
			startOfCentralDirectory = zip64EndOfCentralDirectory.startOfCentralDirectory;

		}
		else
		{
			totalEntries = _endOfCentralDirectory.totalEntries;
			startOfCentralDirectory = _endOfCentralDirectory.startOfCentralDirectory;
		}

		_srcInput->seek(_srcOffset + startOfCentralDirectory);
		for (uint16_t i = 0; i < totalEntries; i++)
		{
			CentralDirectoryFileHeader header;
			if (!header.parse(_srcInput.get()))
				continue;
			ByteArray name(header.fileNameLength);
			if (!ReadData(_srcInput.get(), &name[0], name.size()))
				continue;
			_srcInput->skip(header.extraFieldLength);
			_srcInput->skip(header.fileCommentLength);
			CentralDirectoryFileHeader* fileHeader = new CentralDirectoryFileHeader();
			*fileHeader = header;
			_fileHeaders.insert(std::make_pair(str(name.begin(), name.end()), fileHeader));
		}
		_vaild = 1;
		return true;
	}

private:
	int _vaild;
	int _srcOffset;
	EndOfCentralDirectory _endOfCentralDirectory;
	FileHeaders _fileHeaders;
	StrongPtr<DataInput> _srcInput;
	wstr _fileName;
};

StrongPtr<ZipReader> ZipReader::open(const wstr &name)
{
	return new ZipReaderImpl(name);
}

StrongPtr<ZipReader> ZipReader::open(DataInput* input)
{
	if (!input || !input->seekable())
		return NULL;
	return new ZipReaderImpl(input);
}

StrongPtr<ZipWritter> ZipWritter::create(const wstr &name)
{
	return new ZipWritterImpl(name);
}

StrongPtr<ZipWritter> ZipWritter::create(DataOutput* output)
{
	if (!output || !output->seekable())
		return NULL;
	return new ZipWritterImpl(output);
}
