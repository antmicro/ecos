// This file is part of the uSTL library, an STL implementation.
//
// Copyright (c) 2005 by Mike Sharov <msharov@users.sourceforge.net>
// This file is free software, distributed under the MIT License.

#include "stdtest.h"

#ifdef CYGIMP_USTL_TESTS_BVT25_ROMFS
# include <cyg/fileio/fileio.h>
# if (CYG_BYTEORDER == CYG_LSBFIRST)
#  include <cyg/ustl/bvt25fs_le.h>
# else
#  include <cyg/ustl/bvt25fs_be.h>
# endif
MTAB_ENTRY(romfs_mte1, "/", "romfs", "", (CYG_ADDRWORD) &filedata[0]);
#endif

void TestFStream (void)
{
    fstream fs ("bvt/bvt25.std", ios::in | ios::nocreate);
    if (!fs && !(fs.open("bvt25.std", ios::in | ios::nocreate),fs))
	cout << "Failed to open bvt25.std" << endl;
    string buf;
    buf.resize (fs.size());
    if (buf.size() != 71)
	cout << "fstream.size() returned " << buf.size() << endl;
    fs.read (buf.begin(), buf.size());
    cout << buf;
    fs.close();
}

StdBvtMain (TestFStream)
