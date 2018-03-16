#ifdef __cplusplus
extern "C" {
#endif

#include "contain.h"

	typedef struct FIFO_INFO FIFO_INFO;
	typedef struct FIFO_ITER FIFO_ITER;
	typedef struct FIFO FIFO;

	struct FIFO_INFO {
		void *data;
		FIFO_INFO *prev;
		FIFO_INFO *next;
	};

	struct FIFO_ITER {
		FIFO_INFO *ptr;
	};

	struct FIFO {
		FIFO_INFO *head;
		FIFO_INFO *tail;
		int   cnt;

		/* 添加及弹出 */

		/* 向队列尾部添加动态对象 */
		void(*push_back)(struct FIFO*, void*);
		/* 向队列头部添加动态对象 */
		void(*push_front)(struct FIFO*, void*);
		/* 弹出队列尾部动态对象 */
		void *(*pop_back)(struct FIFO*);
		/* 弹出队列头部动态对象 */
		void *(*pop_front)(struct FIFO*);

		/* for iterator */

		/* 取迭代器头函数 */
		void *(*iter_head)(ITER*, struct FIFO*);
		/* 取迭代器下一个函数 */
		void *(*iter_next)(ITER*, struct FIFO*);
		/* 取迭代器尾函数 */
		void *(*iter_tail)(ITER*, struct FIFO*);
		/* 取迭代器上一个函数 */
		void *(*iter_prev)(ITER*, struct FIFO*);
		/* 取迭代器关联的当前容器成员结构对象 */
		FIFO_INFO *(*iter_info)(ITER*, struct FIFO*);
	};

	void fifo_init(FIFO *fifo);

	FIFO *fifo_new(void);

	int fifo_delete(FIFO *fifo, const void *data);
	void fifo_delete_info(FIFO *fifo, FIFO_INFO *info);

	void fifo_free(FIFO *fifo, void(*free_fn)(void *));
	void fifo_free2(FIFO *fifo, void(*free_fn)(FIFO_INFO *));

	
	FIFO_INFO *fifo_push_back(FIFO *fifo, void *data);
#define fifo_push	fifo_push_back
	void fifo_push_info_back(FIFO *fifo, FIFO_INFO *info);
#define fifo_push_info	fifo_push_info_back
	FIFO_INFO *fifo_push_front(FIFO *fifo, void *data);


	void *fifo_pop_front(FIFO *fifo);
#define fifo_pop	fifo_pop_front
	FIFO_INFO *fifo_pop_info(FIFO *fifo);


	void *fifo_pop_back(FIFO *fifo);

	void *fifo_head(FIFO *fifo);
	FIFO_INFO *fifo_head_info(FIFO *fifo);

	
	void *fifo_tail(FIFO *fifo);
	FIFO_INFO *fifo_tail_info(FIFO *fifo);


	int fifo_size(FIFO *fifo);

#ifdef __cplusplus
}
#endif

