TinySHA1
========

A simple header only SHA1 implementation in C++ (no dependencies). Based on the implementation in boost::uuid::details.

SHA1 Wikipedia Page: http://en.wikipedia.org/wiki/SHA-1

Usage
=====
Just include the header file to use in your code. There are no dependencies other than standard C++. 

```cpp
#include "TinySHA1.hpp"

void testSHA1(const std::string& val) {
  sha1::SHA1 s;
	s.processBytes(val.c_str(), val.size());
	uint32_t digest[5];
	s.getDigest(digest);	
	char tmp[48];
	snprintf(tmp, 45, "%08x %08x %08x %08x %08x", digest[0], digest[1], digest[2], digest[3], digest[4]);
	std::cout<<"Calculated : (\""<<val<<"\") = "<<tmp<<std::endl;
}
```

License
=======
TinySHA1 - a header only implementation of the SHA1 algorithm. Based
on the implementation in boost::uuid::details

Copyright (c) 2012-22 SAURAV MOHAPATRA <mohaps@gmail.com>

 Permission to use, copy, modify, and distribute this software for any
 purpose with or without fee is hereby granted, provided that the above
 copyright notice and this permission notice appear in all copies.
 
 THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 
