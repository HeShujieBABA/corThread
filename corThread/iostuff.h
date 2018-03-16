#ifndef	IOSTUFF_INCLUDE_H
#define	IOSTUFF_INCLUDE_H

#ifdef	__cplusplus
extern "C" {
#endif

#define CLOSE_ON_EXEC   1  /**< ��־λ, ���� exec ���Զ��رմ򿪵������� */
#define PASS_ON_EXEC    0

#define BLOCKING        0  /**< ������д��־λ */
#define NON_BLOCKING    1  /**< ��������д��־λ */

	/**
	* �����׽ӿ�Ϊ�����������
	* @param fd {int} �׽���
	* @param on {int} �Ƿ����ø��׽���Ϊ������, BLOCKING �� NON_BLOCKING
	* @return {int} >= 0: �ɹ�, ����ֵ > 0 ��ʾ����֮ǰ�ı�־λ; -1: ʧ��
	*/
	int non_blocking(int fd, int on);

	/**
	* �趨��ǰ���̿��Դ�����ļ�������ֵ
	* @param limit {int} �趨�����ֵ
	* @return {int} >=0: ok; -1: error
	*/
	int open_limit(int limit);

	
	void tcp_nodelay(int fd, int onoff);


#ifdef	__cplusplus
}
#endif

#endif
