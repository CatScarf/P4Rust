Some notes:

- This code is licensed as CC0 1.0 Universal

- It is built conditionally as part of the delta transfer code for
  'p4 submit'.  This feature requires a few C++11 pieces of code,
  e.g. support/fastcdc.h, so if we're not building one piece because
  of a lack of platform support, we don't build any of it, including this.

- Its only usage is as part of support/chunkmap.cc and support/blake3digester.h.
  It becomes part of libblake3/libp4api.  The libblake3 is basically
  libsupp, but separate because a customer using the P4API might have
  BLAKE3 already linked in their application and would need to avoid
  linking it twice.

- This location (p4/blake3/...) is inconsistent with other
  3rd-party usage in that it's in the global p4/* layer/scope
  rather than limited to a particular library under one of
  the subdirectories. but it got moved along the way for reasons.

  Some other examples of 3rd party code that is scoped according to
  usage/library:

	support/strbuf.cc StrPtr::NCompare 
	support/varqsort.cc
	support/fastcdc.h
	support/mangle.cc
	support/tinyxml2.*
	support/json.hpp
	support/regexp.cc
	support/PID_v1.h

	sys/gzip.*
	sys/unzip.*
	sys/any.h
	sys/optional.h
	sys/crypt.h
	sys/ioapi.h

	script/libs/... (most of these files)

  The files under p4/mem/... and p4/zlib/... are global or used across
  multiple modules, so they're at the top level.
