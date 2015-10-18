/*  Copyright (C) 2011-2014 Dmitry Shatrov - All Rights Reserved
    e-mail: info@momentvideo.org

    Unauthorized copying of this file or any part of its contents, 
    via any medium is strictly prohibited.

    Proprietary and confidential.
 */


#include <libmary/libmary.h>

#include <scruffy/file_byte_stream.h>


// Internal
#define DEBUG_INT(a)


using namespace M;

namespace Scruffy {
    
ByteStream::ByteResult
FileByteStream::getNextByte (char *c_ret)
    throw (InternalException)
{
/*
    errf->print ("Scruffy.FileByteStream.getNextByte: "
		 "start_offset: ")
	 .print ((unsigned long long) start_offset)
	 .pendl ();
*/

//    in_file->seekSet (start_offset);
    if (!in_file->seek (start_offset, SeekOrigin::Beg)) {
      #ifndef LIBMARY_NO_EXCEPTIONS
        throw InternalException (InternalException::BackendError);
      #endif
    }

    unsigned char c;
    {
        IoResult const res = in_file->readFull (Memory::forObject (c), NULL /* ret_nread */);
        if (res == IoResult::Error) {
          #ifndef LIBMARY_NO_EXCEPTIONS
            throw InternalException (InternalException::BackendError);
          #endif
        }

        if (res == IoResult::Eof) {
//            errs->println ("--- FileByteStream: EOF");
            return ByteEof;
        }

        assert (res == IoResult::Normal);
    }

    if (!in_file->tell (&start_offset)) {
      #ifndef LIBMARY_NO_EXCEPTIONS
        throw InternalException (InternalException::BackendError);
      #endif
    }

    if (c_ret != NULL)
	*c_ret = (char) c;

/*
    errf->print ("Scruffy.FileByteStream.getNextByte: "
		 "retuning byte: ")
	 .print ((unsigned long) (unsigned char) c)
	 .pendl ();
*/

//    errs->println ("--- FileByteStream: NORMAL");

    return ByteNormal;
}

StRef<ByteStream::PositionMarker>
FileByteStream::getPosition ()
    throw (InternalException)
{
    StRef<ByteStream::PositionMarker> const ret_pmark =
            st_grab (static_cast <ByteStream::PositionMarker*> (new (std::nothrow) PositionMarker));

    PositionMarker *pmark = static_cast <PositionMarker*> (ret_pmark.ptr ());
    pmark->offset = start_offset;

/*
    errf->print ("Scruffy.FileByteStream.getPosition: "
		 "returning ")
	 .print ((unsigned long long) start_offset)
	 .pendl ();
*/

    return ret_pmark;
}

void
FileByteStream::setPosition (ByteStream::PositionMarker *_pmark)
    throw (InternalException)
{
    PositionMarker *pmark = static_cast <PositionMarker*> (_pmark);
    start_offset = pmark->offset;

/*
    errf->print ("Scruffy.FileByteStream.setPosition: "
		 "position set to ")
	 .print ((unsigned long long) pmark->offset)
	 .pendl ();
*/
}

FileByteStream::FileByteStream (File *in_file)
    throw (InternalException)
{
    assert (in_file);

    this->in_file = in_file;
    if (!in_file->tell (&start_offset)) {
      #ifndef LIBMARY_NO_EXCEPTIONS
        throw InternalException (InternalException::BackendError);
      #endif
    }
}

}

