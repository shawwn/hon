#include "k0_common.h"

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <assert.h>

#include "fileinterface.h"

#pragma warning(disable:4996) // Disabling stupid .NET deprecated warning.

#define DEFAULT_BUFFER_SIZE 1000000
#define DEFAULT_GROW_SIZE   2000000

#if defined(linux)
#   define _stricmp(a,b) strcasecmp((a),(b))                                                   
#endif

class FILE_INTERFACE
{
public:
	FILE_INTERFACE(const char *fname,const char *spec,void *mem,size_t len)
	{
		mMyAlloc = false;
		mRead = true; // default is read access.
		mFph = 0;
		mData = (char *) mem;
		mLen  = len;
		mLoc  = 0;

		if ( spec && _stricmp(spec,"wmem") == 0 )
		{
			mRead = false;
			if ( mem == 0 || len == 0 )
			{
				mData = new char[DEFAULT_BUFFER_SIZE];
				mLen  = DEFAULT_BUFFER_SIZE;
				mMyAlloc = true;
			}
		}

		if ( mData == 0 )
		{
			mFph = fopen(fname,spec);
		}

  	strncpy(mName,fname,512);
	}

  ~FILE_INTERFACE(void)
  {
  	if ( mMyAlloc )
  	{
  		delete mData;
  	}
  	if ( mFph )
  	{
  		fclose(mFph);
  	}
  }

  size_t read(char *data,size_t size)
  {
  	size_t ret = 0;
  	if ( (mLoc+size) <= mLen )
  	{
  		memcpy(data, &mData[mLoc], size );
  		mLoc+=size;
  		ret = 1;
  	}
    return ret;
  }

  size_t write(const char *data,size_t size)
  {
  	size_t ret = 0;

		if ( (mLoc+size) >= mLen && mMyAlloc ) // grow it
		{
			size_t newLen = mLen+DEFAULT_GROW_SIZE;
			if ( size > newLen ) newLen = size+DEFAULT_GROW_SIZE;

			char *data = new char[newLen];
			memcpy(data,mData,mLoc);
			delete mData;
			mData = data;
			mLen  = newLen;
		}

  	if ( (mLoc+size) <= mLen )
  	{
  		memcpy(&mData[mLoc],data,size);
  		mLoc+=size;
  		ret = 1;
  	}
  	return ret;
  }

	size_t read(void *buffer,size_t size,size_t count)
	{
		size_t ret = 0;
		if ( mFph )
		{
			ret = fread(buffer,size,count,mFph);
		}
		else
		{
			char *data = (char *)buffer;
			for (size_t i=0; i<count; i++)
			{
				if ( (mLoc+size) <= mLen )
				{
					read(data,size);
					data+=size;
					ret++;
				}
				else
				{
					break;
				}
			}
		}
		return ret;
	}

  size_t write(const void *buffer,size_t size,size_t count)
  {
  	size_t ret = 0;

  	if ( mFph )
  	{
  		ret = fwrite(buffer,size,count,mFph);
  	}
  	else
  	{
  		const char *data = (const char *)buffer;

  		for (size_t i=0; i<count; i++)
  		{
    		if ( write(data,size) )
				{
    			data+=size;
    			ret++;
    		}
    		else
    		{
    			break;
    		}
  		}
  	}
  	return ret;
  }

  size_t writeString(const char *str)
  {
  	size_t ret = 0;
  	if ( str )
  	{
  		size_t len = strlen(str);
  		ret = write(str,len, 1 );
  	}
  	return ret;
  }


  size_t  flush(void)
  {
  	size_t ret = 0;
  	if ( mFph )
  	{
  		ret = fflush(mFph);
  	}
  	return ret;
  }


  size_t seek(size_t loc,size_t mode)
  {
  	size_t ret = 0;
  	if ( mFph )
  	{
  		ret = fseek(mFph,loc,mode);
  	}
  	else
  	{
  		if ( mode == SEEK_SET )
  		{
  			if ( loc <= mLen )
  			{
  				mLoc = loc;
  				ret = 1;
  			}
  		}
  		else if ( mode == SEEK_END )
  		{
  			mLoc = mLen;
  		}
  		else
  		{
  			assert(0);
  		}
  	}
  	return ret;
  }

  size_t tell(void)
  {
  	size_t ret = 0;
  	if ( mFph )
  	{
  		ret = ftell(mFph);
  	}
  	else
  	{
  		ret = mLoc;
  	}
  	return ret;
  }

  size_t myputc(char c)
  {
  	size_t ret = 0;
  	if ( mFph )
  	{
  		ret = fputc(c,mFph);
  	}
  	else
  	{
  		ret = write(&c,1);
  	}
  	return ret;
  }

  size_t eof(void)
  {
  	size_t ret = 0;
  	if ( mFph )
  	{
  		ret = feof(mFph);
  	}
  	else
  	{
  		if ( mLoc >= mLen )
  			ret = 1;
  	}
  	return ret;
  }

  size_t  error(void)
  {
  	size_t ret = 0;
  	if ( mFph )
  	{
  		ret = ferror(mFph);
  	}
  	return ret;
  }


  FILE 	*mFph;
  char  *mData;
  size_t    mLen;
  size_t    mLoc;
  bool   mRead;
	char   mName[512];
	bool   mMyAlloc;

};

FILE_INTERFACE * fi_fopen(const char *fname,const char *spec,void *mem,size_t len)
{
	FILE_INTERFACE *ret = 0;

	ret = new FILE_INTERFACE(fname,spec,mem,len);

	if ( mem == 0 && ret->mData == 0)
  {
  	if ( ret->mFph == 0 )
  	{
  		delete ret;
  		ret = 0;
  	}
  }

	return ret;
}

void       fi_fclose(FILE_INTERFACE *file)
{
	delete file;
}

size_t        fi_fread(void *buffer,size_t size,size_t count,FILE_INTERFACE *fph)
{
	size_t ret = 0;
	if ( fph )
	{
		ret = fph->read(buffer,size,count);
	}
	return ret;
}

size_t        fi_fwrite(const void *buffer,size_t size,size_t count,FILE_INTERFACE *fph)
{
	size_t ret = 0;
	if ( fph )
	{
		ret = fph->write(buffer,size,count);
	}
	return ret;
}

size_t        fi_fprintf(FILE_INTERFACE *fph,const char *fmt,...)
{
	size_t ret = 0;

	char buffer[2048];
#ifdef _WIN32
	vsprintf(buffer, fmt, (char *)(&fmt+1));
#else
	va_list ap;
	va_start(ap, fmt);
	vsprintf(buffer, fmt, ap);
	va_end(ap);
#endif

	if ( fph )
	{
		ret = fph->writeString(buffer);
	}

	return ret;
}


size_t        fi_fflush(FILE_INTERFACE *fph)
{
	size_t ret = 0;
	if ( fph )
	{
		ret = fph->flush();
	}
	return ret;
}


size_t        fi_fseek(FILE_INTERFACE *fph,size_t loc,size_t mode)
{
	size_t ret = 0;
	if ( fph )
	{
		ret = fph->seek(loc,mode);
	}
	return ret;
}

size_t        fi_ftell(FILE_INTERFACE *fph)
{
	size_t ret = 0;
	if ( fph )
	{
		ret = fph->tell();
	}
	return ret;
}

size_t        fi_fputc(char c,FILE_INTERFACE *fph)
{
	size_t ret = 0;
	if ( fph )
	{
		ret = fph->myputc(c);
	}
	return ret;
}

size_t        fi_fputs(const char *str,FILE_INTERFACE *fph)
{
	size_t ret = 0;
	if ( fph )
	{
		ret = fph->writeString(str);
	}
	return ret;
}

size_t        fi_feof(FILE_INTERFACE *fph)
{
	size_t ret = 0;
	if ( fph )
	{
		ret = fph->eof();
	}
	return ret;
}

size_t        fi_ferror(FILE_INTERFACE *fph)
{
	size_t ret = 0;
	if ( fph )
	{
		ret = fph->error();
	}
	return ret;
}

void *     fi_getMemBuffer(FILE_INTERFACE *fph,size_t &outputLength)
{
	outputLength = 0;
	void * ret = 0;
	if ( fph )
	{
		ret = fph->mData;
		outputLength = fph->mLoc;
		fph->mMyAlloc = false;
	}
	return ret;
}

