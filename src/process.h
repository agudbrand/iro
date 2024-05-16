/*
 *  API for creating and iteracting with OS processes.
 */

#ifndef _iro_process_h
#define _iro_process_h

#include "common.h"
#include "unicode.h"
#include "containers/slice.h"

#include "nil.h"

#include "fs/fs.h"

namespace iro
{

/* ================================================================================================ Process
 */
struct Process
{
	typedef void* Handle;

	Handle handle;
	b8 terminated;
	s32 exit_code;

    /* ============================================================================================ Process:Stream
	 *  A file stream that may replace one of the process' stdio streams.
     */
	struct Stream
	{
		// prevent blocking on reads/writes 
		b8 non_blocking;
		fs::File* file;
	};
	
	// Spawns a new process from 'file', passing 'args' and optionally replacing
	// its stdio streams with the Streams given.
	//
	// A File given in a Stream must be nil, and will be initialized according
	// to its position in the 'streams' array. In position 0, the file will
	// act as a pipe to the process's stdin stream allowing the parent 
	// process to write to it. In position 1 or 2, the file will act as a pipe 
	// to the process's stdout and stderr streams, respectively, allowing the 
	// parent process to read from it. If the given stream is marked as 
	// non_blocking, reading or writing to the child process will be done 
	// in an async manner (eg. if the child process does not have any buffered
	// data on stdout, the parent process wont wait for data like it normally does)
	static Process spawn(str file, Slice<str> args, Stream streams[3]);

	// Checks the status of this process and sets 'terminated' and 'exit_code'
	// if the process has finished.
	void checkStatus();
};

}

DefineNilValue(iro::Process, {nullptr}, { return x.handle == nullptr; });
DefineMove(iro::Process, { to.handle = from.handle; });

#endif