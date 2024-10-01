#include "process.h"
#include "platform.h"

namespace iro
{

Process Process::spawn(
    str        file, 
    Slice<str> args, 
    Stream     streams[3], 
    str        cwd)
{
  Process out = {};
  if (!platform::processSpawn(&out.handle, file, args, streams, cwd))
    return nil;
  return out;
}

Process Process::spawnpty(str file, Slice<str> args, fs::File* stream)
{
  Process out = {};
  if (!platform::processSpawnPTY(&out.handle, file, args, stream))
    return nil;
  return out;
}

void Process::checkStatus()
{
  assert(handle);

  if (status == Status::Running)
  {
    switch (platform::processCheck(handle, &exit_code))
    {
    case platform::ProcessCheckResult::Exited:
      status = Status::ExitedNormally;
      break;
    case platform::ProcessCheckResult::Error:
      status = Status::ExitedFatally;
      break;
    }
  }
}

b8 Process::stop(s32 exit_code)
{
  assert(handle);
  
  if (ispty)
    return platform::stopProcessPTY(handle, exit_code);
  else if (!terminated)
    return platform::stopProcess(handle, exit_code);
  return false;
}

}

