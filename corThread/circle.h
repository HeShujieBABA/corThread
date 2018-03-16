#ifndef	RING_INCLUDE_H
#define	RING_INCLUDE_H

#ifdef  __cplusplus
extern "C" {
#endif

#include <stddef.h>
	typedef struct RING RING;
	struct RING {
		RING   *succ;      
		RING   *pred;        

		RING   *parent;         
		int   len;                 
	};

	typedef struct RING_ITER {
		RING *ptr;
	} RING_ITER;

	void ring_init(RING *ring);

	int  ring_size(const RING *ring);

	void ring_prepend(RING *ring, RING *entry);

	void ring_append(RING *ring, RING *entry);

	void ring_detach(RING *entry);

	RING *ring_pop_head(RING *ring);

	
	RING *ring_pop_tail(RING *ring);

	
#define RING_SUCC(c) ((c)->succ)
#define	ring_succ	RING_SUCC

	
#define RING_PRED(c) ((c)->pred)
#define	ring_pred	RING_PRED

#define RING_TO_APPL(ring_ptr, app_type, ring_member) \
	((app_type *)(((char *)(ring_ptr)) - offsetof(app_type, ring_member)))

#define	ring_to_appl	RING_TO_APPL

	
#define	RING_FOREACH(iter, head_ptr) \
	for ((iter).ptr = ring_succ((head_ptr)); (iter).ptr != (head_ptr);  \
	(iter).ptr = ring_succ((iter).ptr))

#define	ring_foreach		RING_FOREACH

#define	RING_FOREACH_REVERSE(iter, head_ptr) \
	for ((iter).ptr = ring_pred((head_ptr)); (iter).ptr != (head_ptr);  \
	(iter).ptr = ring_pred((iter).ptr))

#define	ring_foreach_reverse	RING_FOREACH_REVERSE

#define RING_FIRST(head) \
	(ring_succ(head) != (head) ? ring_succ(head) : 0)

#define	ring_first		RING_FIRST

#define RING_FIRST_APPL(head, app_type, ring_member) \
	(ring_succ(head) != (head) ? \
	RING_TO_APPL(ring_succ(head), app_type, ring_member) : 0)

#define	ring_first_appl	RING_FIRST_APPL


#define RING_LAST(head) \
	(ring_pred(head) != (head) ? ring_pred(head) : 0)

#define	ring_last		RING_LAST

#define RING_LAST_APPL(head, app_type, ring_member) \
	(ring_pred(head) != (head) ? \
	RING_TO_APPL(ring_pred(head), app_type, ring_member) : 0)

#define	ring_last_appl	RING_LAST_APPL

	
#define	RING_APPEND(ring_in, entry_in) do {  \
	RING *ring_ptr = (ring_in), *entry_ptr = (entry_in);  \
	entry_ptr->succ = ring_ptr->succ;  \
	entry_ptr->pred = ring_ptr;  \
	entry_ptr->parent = ring_ptr->parent;  \
	ring_ptr->succ->pred = entry_ptr;  \
	ring_ptr->succ = entry_ptr;  \
	ring_ptr->parent->len++;  \
	} while (0)

	
#define	RING_PREPEND(ring_in, entry_in) do {  \
	RING *ring_ptr = (ring_in), *entry_ptr = (entry_in);  \
	entry_ptr->pred = ring_ptr->pred;  \
	entry_ptr->succ = ring_ptr;  \
	entry_ptr->parent = ring_ptr->parent;  \
	ring_ptr->pred->succ = entry_ptr;  \
	ring_ptr->pred = entry_ptr;  \
	ring_ptr->parent->len++;  \
	} while (0)

	
#define	RING_DETACH(entry_in) do {  \
	RING   *succ, *pred, *entry_ptr = (entry_in);  \
	succ = entry_ptr->succ;  \
	pred = entry_ptr->pred;  \
	if (succ != NULL && pred != NULL) { 	\
	pred->succ = succ;  \
	succ->pred = pred;  \
	entry_ptr->parent->len--;  \
	entry_ptr->succ = entry_ptr->pred = NULL;  \
	}  \
	} while (0)

#ifdef  __cplusplus
}
#endif

#endif

