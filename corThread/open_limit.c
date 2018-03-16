#include "stdafx.h"
#include "msg.h"
#include "iostuff.h"

int open_limit(int limit)
{
	if (limit <= 0)
		limit = 10240;
	return limit;
}
