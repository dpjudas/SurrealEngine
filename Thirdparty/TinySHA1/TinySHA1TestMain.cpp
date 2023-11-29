/* 
 *
 * TinySHA1 - a header only implementation of the SHA1 algorithm. Based
 * on the implementation in boost::uuid::details
 * 
 * Copyright (c) 2012-22 SAURAV MOHAPATRA <mohaps@gmail.com>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */
 
#include "TinySHA1.hpp"
#include <iostream>
#include <string>
/**
 * Test function that prints the input string and its SHA1 hash
 */
void testSHA1(const std::string& val) {
 	sha1::SHA1 s;
	s.processBytes(val.c_str(), val.size());
	uint32_t digest[5];
	s.getDigest(digest);	
	char tmp[48];
	snprintf(tmp, 45, "%08x %08x %08x %08x %08x", digest[0], digest[1], digest[2], digest[3], digest[4]);
	std::cout<<"Calculated : (\""<<val<<"\") = "<<tmp<<std::endl;
}
// test set (from http://en.wikipedia.org/wiki/SHA-1#Example_hashes)
//SHA1("The quick brown fox jumps over the lazy dog") = 2fd4e1c6 7a2d28fc ed849ee1 bb76e739 1b93eb12
//SHA1("The quick brown fox jumps over the lazy cog") = de9f2c7f d25e1b3a fad3e85a 0bd17d9b 100db4b3
//SHA1("") = da39a3ee 5e6b4b0d 3255bfef 95601890 afd80709
int main(int argc, char **argv) {
	testSHA1("The quick brown fox jumps over the lazy dog");
	testSHA1("The quick brown fox jumps over the lazy cog");
	testSHA1("");
	return 0;
}
