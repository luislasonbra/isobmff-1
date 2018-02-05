
#include "MP4Atoms.h"
#include <stdlib.h>
#include <string.h>


static void destroy(MP4AtomPtr s)
{
	MP4Err err;
	MP4SchemeTypeAtomPtr self = (MP4SchemeTypeAtomPtr)s;
	err = MP4NoErr;

	if (self == NULL)
		BAILWITHERROR(MP4BadParamErr);

		// if there is a scheme_url field, free it
	if (self->scheme_url) {
		free(self->scheme_url);
		self->scheme_url = NULL;
	}

	if (self->super)
		self->super->destroy(s);
bail:
	TEST_RETURN(err);

	return;
}

static MP4Err serialize(struct MP4Atom* s, char* buffer)
{
	MP4Err err;
	MP4SchemeTypeAtomPtr self = (MP4SchemeTypeAtomPtr)s;
	err = MP4NoErr;

	err = MP4SerializeCommonFullAtomFields((MP4FullAtomPtr)s, buffer); if (err) goto bail;
	buffer += self->bytesWritten;
	PUT32(scheme_type);
	PUT32(scheme_version);
	if ((self->flags & 1) == 1)
	{
		u32 len = strlen(self->scheme_url) + 1;
		PUTBYTES(self->scheme_url, len);
	}

	assert(self->bytesWritten == self->size);
bail:
	TEST_RETURN(err);

	return err;
}

static MP4Err calculateSize(struct MP4Atom* s)
{
	MP4Err err;
	MP4SchemeTypeAtomPtr self = (MP4SchemeTypeAtomPtr)s;
	err = MP4NoErr;

	err = MP4CalculateFullAtomFieldSize((MP4FullAtomPtr)s); if (err) goto bail;
	self->size += 8;  // 4 bytes (scheme_type) + 4 bytes (scheme_version)
	if ((self->scheme_url) && (strlen(self->scheme_url) > 0))
	{
		self->flags = 1;
		self->size += 1 + strlen(self->scheme_url); // strlen ignores \0
	}
	else self->flags = 0;

bail:
	TEST_RETURN(err);

	return err;
}

static MP4Err createFromInputStream(MP4AtomPtr s, MP4AtomPtr proto, MP4InputStreamPtr inputStream)
{
	MP4Err err;
	long bytesToRead;
	char debugstr[256];
	MP4SchemeTypeAtomPtr self = (MP4SchemeTypeAtomPtr)s;

	err = MP4NoErr;
	if (self == NULL)	BAILWITHERROR(MP4BadParamErr)
		err = self->super->createFromInputStream(s, proto, (char*)inputStream); if (err) goto bail;
	GET32(scheme_type);
	GET32(scheme_version);
	bytesToRead = self->size - self->bytesRead;
	if (bytesToRead < 0)
		BAILWITHERROR(MP4BadDataErr)
	if (bytesToRead > 0)
	{
		if ((self->flags & 1) != 1) { err = MP4BadDataErr; goto bail; }
		self->scheme_url = (char*)calloc(1, bytesToRead);
		if (self->scheme_url == NULL)
			BAILWITHERROR(MP4NoMemoryErr)
			GETBYTES(bytesToRead, scheme_url);
		if (bytesToRead < 200)
		{
			sprintf(debugstr, "Scheme URL location is \"%s\"", self->scheme_url);
			DEBUG_MSG(debugstr);
		}
	}
	else
	{
		if ((self->flags & 1) != 0) { err = MP4BadDataErr; goto bail; }
	}

	assert(self->bytesRead == self->size);

bail:
	TEST_RETURN(err);

	return err;
}

MP4Err MP4CreateSchemeTypeAtom(MP4SchemeTypeAtomPtr *outAtom)
{
	MP4Err err;
	MP4SchemeTypeAtomPtr self;

	self = (MP4SchemeTypeAtomPtr)calloc(1, sizeof(MP4SchemeTypeAtom));
	TESTMALLOC(self);

	err = MP4CreateFullAtom((MP4AtomPtr)self);
	if (err) goto bail;
	self->type = MP4SchemeTypeAtomType;
	self->name = "SchemeTypeBox";
	self->createFromInputStream = (cisfunc)createFromInputStream;
	self->destroy = destroy;
	self->calculateSize = calculateSize;
	self->serialize = serialize;

	*outAtom = self;
bail:
	TEST_RETURN(err);

	return err;
}

