#ifndef __LINKIO_HPP
#define __LINKIO_HPP

#include "linka.hpp"

int ReadLinks(const char *filename, LinkArray &la);
int WriteLinks(const char *filename, const LinkArray &la);

#endif

