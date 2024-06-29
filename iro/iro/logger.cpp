#include "logger.h"

#include "time.h"

namespace iro
{

  Log log;

  /* --------------------------------------------------------------------------
   */
  b8 Log::init()
  {
    // make sure we're not already initialized 
    if (notnil(destinations))
      return true;

    destinations = Array<Dest>::create(4);
    return true;
  }

  /* --------------------------------------------------------------------------
   */
  void Log::deinit()
  {
    destinations.destroy();
  }

  /* --------------------------------------------------------------------------
   */
  void Log::newDestination(str name, io::IO* d, Dest::Flags flags)
  {
    destinations.push({name, flags, d});
  }

  /* --------------------------------------------------------------------------
   */
  b8 Logger::init(str name, Verbosity verbosity)
  {
    this->name = name;
    this->verbosity = verbosity;
    need_prefix = true;
    return true;
  }


  /* --------------------------------------------------------------------------
   */
  str tryColor(Log::Dest& dest, color::Color color)
  {
    if (!dest.flags.test(Log::Dest::Flag::AllowColor))
      return ""_str;

    return color::getColor(color);
  }

  /* --------------------------------------------------------------------------
   */
  void Logger::writePrefix(Verbosity v, Log::Dest& d)
  {
    using enum Log::Dest::Flag;

    if (d.flags.test(ShowDateTime))
    {
      char buffer[64];
      time_t t = time(0);
      struct tm* tm = localtime(&t);
      size_t written = strftime(buffer, 64, "[%Y-%m-%d %H:%M:%S] ", tm);
      io::format(d.io, str{(u8*)buffer, written});
    }

    if (d.flags.test(ShowCategoryName))
    {
      if (d.flags.test(TrackLongestName))
      {
        if (name.len > iro::log.max_name_len)
          iro::log.max_name_len = name.len;
        for (s32 i = 0; i < iro::log.max_name_len - name.len; i++)
          io::format(d.io, " ");
      }

      io::formatv(d.io, name, ": ");
    }

    if (d.flags.test(ShowVerbosity))
    {
      using enum Verbosity;
      using enum color::Color;

      if (d.flags.test(PadVerbosity))
      {
        switch (v)
        {
        case Trace:  io::formatv(d.io, color::   cyan(" trace"_str)); break;
        case Debug:  io::formatv(d.io, color::  green(" debug"_str)); break;
        case Info:   io::formatv(d.io, color::   blue("  info"_str)); break;
        case Notice: io::formatv(d.io, color::magenta("notice"_str)); break;
        case Warn:   io::formatv(d.io, color:: yellow("  warn"_str)); break;
        case Error:  io::formatv(d.io, color::    red(" error"_str)); break;
        case Fatal:  io::formatv(d.io, color::    red(" fatal"_str)); break;
        }
      }
      else
      {
        switch (v)
        {
        case Trace:  io::formatv(d.io, color::   cyan("trace"_str));  break;
        case Debug:  io::formatv(d.io, color::  green("debug"_str));  break;
        case Info:   io::formatv(d.io, color::   blue("info"_str));   break;
        case Notice: io::formatv(d.io, color::magenta("notice"_str)); break;
        case Warn:   io::formatv(d.io, color:: yellow("warn"_str));   break;
        case Error:  io::formatv(d.io, color::    red("error"_str));  break;
        case Fatal:  io::formatv(d.io, color::    red("fatal"_str));  break;
        }
      }

      io::format(d.io, ": ");
    }

    for (s32 i = 0; i < iro::log.indentation; i++)
      d.io->write(" "_str);
  }

}

/* ============================================================================
 *  C interface for use by logger.lua
 */
extern "C"
{
using namespace iro;

/* ----------------------------------------------------------------------------
 *  Report the size of a logger so that lua may allocate space for it 
 *  internally rather than us try and figure that out here.
 */
EXPORT_DYNAMIC
u64 iro_loggerSize() { return sizeof(Logger); }

/* ----------------------------------------------------------------------------
 *  Fill out the allocated logger.
 */
EXPORT_DYNAMIC
void iro_initLogger(Logger* logger, str name, u32 verbosity)
{
  logger->init(name, Logger::Verbosity(verbosity));
}

/* ----------------------------------------------------------------------------
 *  Change the loggers name.
 */
EXPORT_DYNAMIC
void iro_loggerSetName(Logger* logger, str name)
{
  logger->name = name;
}

/* ----------------------------------------------------------------------------
 *  Test if this logger can output anything with its set verbosity and log the 
 *  first part if so.
 */
EXPORT_DYNAMIC
b8 iro_logFirst(Logger* logger, u32 verbosity, str s)
{
  if ((u32)logger->verbosity > verbosity)
    return false;

  logger->log(Logger::Verbosity(verbosity), s);

  return true;
}

/* ----------------------------------------------------------------------------
 *  Log the remaining parts, if any.
 */
EXPORT_DYNAMIC
void iro_logTrail(Logger* logger, u32 verbosity, str s)
{
  // no check for verbosity as the call to logFirst tells us if we should 
  // continue
  logger->log(Logger::Verbosity(verbosity), s);
}

}
