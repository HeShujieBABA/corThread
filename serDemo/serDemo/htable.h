#ifndef HTABLE_H
#define HTABLE_H


#ifdef  __cplusplus
extern "C" {
#endif

#include "contain.h"

	typedef struct HTABLE		HTABLE;
	typedef struct HTABLE_INFO 	HTABLE_INFO;

	struct HTABLE {
		int     size;                  
		int     init_size;              
		int     used;                   
		HTABLE_INFO **data;            
		int     status;                

		void *(*iter_head)(ITER*, struct HTABLE*);
		
		void *(*iter_next)(ITER*, struct HTABLE*);
	
		void *(*iter_tail)(ITER*, struct HTABLE*);
		
		void *(*iter_prev)(ITER*, struct HTABLE*);
		
		HTABLE_INFO *(*iter_info)(ITER*, struct HTABLE*);
	};

	
	struct HTABLE_INFO {
		char *key;
		void *value;			
		unsigned hash;			
		struct HTABLE_INFO *next;	
		struct HTABLE_INFO *prev;	
	};

	HTABLE *htable_create(int size);

	int htable_errno(HTABLE *table);
#define	HTABLE_STAT_OK          0  /**< 状态正常 */
#define	HTABLE_STAT_INVAL       1  /**< 无效参数 */
#define	HTABLE_STAT_DUPLEX_KEY  2  /**< 重复键 */

	
	void htable_set_errno(HTABLE *table, int error);

	HTABLE_INFO *htable_enter(HTABLE *table, const char *key, void *value);

	HTABLE_INFO *htable_locate(HTABLE *table, const char *key);

	void *htable_find(HTABLE *table, const char *key);

	int htable_delete(HTABLE *table, const char *key, void(*free_fn) (void *));

	void htable_delete_entry(HTABLE *table, HTABLE_INFO *ht, void(*free_fn) (void *));

	void htable_free(HTABLE *table, void(*free_fn) (void *));

	int htable_reset(HTABLE *table, void(*free_fn) (void *));

	void htable_walk(HTABLE *table, void(*walk_fn) (HTABLE_INFO *, void *), void *arg);

	int htable_size(const HTABLE *table);

	int htable_used(const HTABLE *table);

	HTABLE_INFO **htable_list(const HTABLE *table);

	void htable_stat(const HTABLE *table);
#define	htable_stat_r	htable_stat


#define	HTABLE_ITER_KEY(iter)	((iter).ptr->key.c_key)
#define	htable_iter_key		HTABLE_ITER_KEY

#define	HTABLE_ITER_VALUE(iter)	((iter).ptr->value)
#define	htable_iter_value	HTABLE_ITER_VALUE

#ifdef  __cplusplus
}
#endif

#endif
