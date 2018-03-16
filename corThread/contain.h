#ifndef	__ITERATOR_INCLUDE_H__
#define	__ITERATOR_INCLUDE_H__

typedef struct ITER ITER;

struct ITER {
	void *ptr;		
	void *data;		
	int   dlen;		
	const char *key;	
	int   klen;		
	int   i;		
	int   size;		
};


#define	FOREACH(iter, container)  \
for ((container)->iter_head(&(iter), (container));  \
	(iter).ptr;  \
	(container)->iter_next(&(iter), (container)))


#define	FOREACH_REVERSE(iter, container)  \
for ((container)->iter_tail(&(iter), (container));  \
	(iter).ptr;  \
	(container)->iter_prev(&(iter), (container)))


#define	ITER_INFO(iter, container)  \
	(container)->iter_info(&(iter), (container))

#define	foreach_reverse	FOREACH_REVERSE
#define	foreach		FOREACH

#endif