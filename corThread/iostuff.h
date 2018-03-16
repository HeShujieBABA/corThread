#ifndef	IOSTUFF_INCLUDE_H
#define	IOSTUFF_INCLUDE_H

#ifdef	__cplusplus
extern "C" {
#endif

#define CLOSE_ON_EXEC   1  /**< 标志位, 调用 exec 后自动关闭打开的描述字 */
#define PASS_ON_EXEC    0

#define BLOCKING        0  /**< 阻塞读写标志位 */
#define NON_BLOCKING    1  /**< 非阻塞读写标志位 */

	/**
	* 设置套接口为阻塞或非阻塞
	* @param fd {int} 套接字
	* @param on {int} 是否设置该套接字为非阻塞, BLOCKING 或 NON_BLOCKING
	* @return {int} >= 0: 成功, 返回值 > 0 表示设置之前的标志位; -1: 失败
	*/
	int non_blocking(int fd, int on);

	/**
	* 设定当前进程可以打开最大文件描述符值
	* @param limit {int} 设定的最大值
	* @return {int} >=0: ok; -1: error
	*/
	int open_limit(int limit);

	
	void tcp_nodelay(int fd, int onoff);


#ifdef	__cplusplus
}
#endif

#endif
