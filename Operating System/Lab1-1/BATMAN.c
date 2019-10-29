#include <pthread.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#define MAX 100										// �������
typedef enum directionT {South, East, North, West} direction;						// ����ö��
typedef struct CarT										// �����ṹ��
{	
	int id;										// �������� 
	direction dir;									// ��������ķ���
} Car;

// �ĸ�����ĳ����ź�
pthread_cond_t firstNorth;
pthread_cond_t firstEast;
pthread_cond_t firstSouth;
pthread_cond_t firstWest;

// �ĸ������·����Դ�� �ͳ����źŶ�Ӧ
pthread_mutex_t source_north;
pthread_mutex_t source_east;
pthread_mutex_t source_south;
pthread_mutex_t source_west;

// �ĸ�����Ľ���׼������λ�õ��ź�
pthread_cond_t queueNorth;
pthread_cond_t queueEast;
pthread_cond_t queueSouth;
pthread_cond_t queueWest;

// �ĸ�����׼��ͨ���� ��׼�������źŶ�Ӧ
pthread_mutex_t crossingNorth;
pthread_mutex_t crossingEast;
pthread_mutex_t crossingSouth;
pthread_mutex_t crossingWest;

// �����������ź�
pthread_cond_t deadlock;
pthread_mutex_t wait_deadlock;

// �ĸ�����ĳ�����ͳ��
int count_North = 0;
int count_East = 0;
int count_South = 0;
int count_West = 0;

void *check_deadlock(void* c)									// �����ж��߳�
{
	while(1)
	{
		pthread_mutex_lock(&wait_deadlock);						// ����
		pthread_cond_wait(&deadlock, &wait_deadlock);					// �ȴ���������
		
		if(count_South > 0 && count_East > 0 && count_North > 0 && count_West > 0)		// �������г���˵������
		{
			printf("DEADLOCK: car jam detected, signalling North to go.\n");		// ��ӡ������ʾ
			usleep(1);
			pthread_cond_signal(&firstNorth);					// �ñ��ߵĳ�������
		}
		
		pthread_mutex_unlock(&wait_deadlock);						// ����
	}
}

// ע�ͱ��߳������� ��������ͬ��
void *car_from_north(void *c)
{
	pthread_mutex_lock(&crossingNorth);							// �����÷���׼��ͨ��
	Car *car = (Car*)c;									// �����������ת��
	pthread_cond_wait(&queueNorth, &crossingNorth);					// �ȴ�ͨ���ź�
	count_North += 1;									// ���³�����Ŀͳ��
	printf("car %d from north arrives crossing\n", car->id);					// ��ӡ������ʾ
	usleep(1);
	pthread_mutex_unlock(&crossingNorth);							// ����÷���׼��ͨ������ ��ʼͨ��
	pthread_mutex_lock(&source_north);							// �����÷���ͨ����Դ��
	if (count_West > 0)
	{
		pthread_cond_signal(&deadlock);						// ����ұ��г� ����������ж�
		pthread_cond_wait(&firstNorth, &source_north);					// ����ұ��г� �ȴ��ұ߳�ͨ�����ѱ���
	}
	count_North--;									// ���³�����Ŀͳ��
	printf("car %d from north leaving crossing\n", car->id);					// ��ӡ�뿪��ʾ
	pthread_mutex_unlock(&source_north);							// ����÷���ͨ����Դ�� �뿪·��
	pthread_cond_signal(&firstEast);							// �����źŻ��� ��ߵĳ�������У�
	usleep(1);
	pthread_cond_signal(&queueNorth);							// ����ͬ������е���һ����
	pthread_exit(NULL);									// �뿪�߳�
}

void *car_from_east(void *c)
{
	pthread_mutex_lock(&crossingEast);
	Car *car = (Car*)c;
	pthread_cond_wait(&queueEast, &crossingEast);
	count_East += 1;
	printf("car %d from east arrives crossing\n", car->id);
	usleep(1);
	pthread_mutex_unlock(&crossingEast);
	pthread_mutex_lock(&source_east);
	if (count_North > 0)
	{
		pthread_cond_signal(&deadlock);
		pthread_cond_wait(&firstEast, &source_east);
	}
	count_East--;
	printf("car %d from east leaving crossing\n", car->id);
	pthread_mutex_unlock(&source_east);
	pthread_cond_signal(&firstSouth);
	usleep(1);
	pthread_cond_signal(&queueEast);
	pthread_exit(NULL);
}

void *car_from_south(void *c)
{
	pthread_mutex_lock(&crossingSouth);
	Car *car = (Car*)c;
	pthread_cond_wait(&queueSouth, &crossingSouth);
	count_South += 1;
	printf("car %d from south arrives crossing.\n", car->id);
	usleep(1);
	pthread_mutex_unlock(&crossingSouth);
	pthread_mutex_lock(&source_south);
	if (count_East > 0)
	{
		pthread_cond_signal(&deadlock);
		pthread_cond_wait(&firstSouth, &source_south);
	}
	count_South--;
	printf("car %d from south leaving crossing.\n", car->id);
	pthread_mutex_unlock(&source_south);
	pthread_cond_signal(&firstWest);
	usleep(1);
	pthread_cond_signal(&queueSouth);
	pthread_exit(NULL);
}

void *car_from_west(void *c)
{
	pthread_mutex_lock(&crossingWest);
	Car *car = (Car*)c;
	pthread_cond_wait(&queueWest, &crossingWest);
	count_West += 1;
	printf("car %d from west arrives crossing\n", car->id);
	usleep(1);
	pthread_mutex_unlock(&crossingWest);
	pthread_mutex_lock(&source_west);
	if (count_South > 0)
	{
		pthread_cond_signal(&deadlock);
		pthread_cond_wait(&firstWest, &source_west);
	}
	count_West--;
	printf("car %d from west leaving crossing\n", car->id);
	pthread_mutex_unlock(&source_west);
	pthread_cond_signal(&firstNorth);
	usleep(1);
	pthread_cond_signal(&queueWest);
	pthread_exit(NULL);
}

int main(int argc, char *argv[])
{
	// ��ʼ��
	pthread_mutex_init(&source_north, NULL);
	pthread_mutex_init(&source_east, NULL);
	pthread_mutex_init(&source_south, NULL);
	pthread_mutex_init(&source_west, NULL);

	pthread_mutex_init(&crossingNorth, NULL);
	pthread_mutex_init(&crossingEast, NULL);
	pthread_mutex_init(&crossingSouth, NULL);
	pthread_mutex_init(&crossingWest, NULL);

	pthread_cond_init(&firstNorth, NULL);
	pthread_cond_init(&firstEast, NULL);
	pthread_cond_init(&firstSouth, NULL);
	pthread_cond_init(&firstWest, NULL);

	pthread_cond_init(&queueNorth, NULL);
	pthread_cond_init(&queueEast, NULL);
	pthread_cond_init(&queueSouth, NULL);
	pthread_cond_init(&queueWest, NULL);
	
	pthread_mutex_init(&wait_deadlock, NULL);
	pthread_cond_init(&deadlock, NULL);
	
	Car cars[MAX];
	pthread_t car_threads[MAX];

	pthread_t check;
	pthread_create(&check, NULL, check_deadlock, NULL);					// ������������߳�
	
	for (int i = 0; i < strlen(argv[1]); i++)							// ���������ַ�����������Ӧ�߳�
	{
		cars[i].id = i + 1;
		switch (argv[1][i]) 
		{
			case 'n':
				cars[i].dir = North;
				pthread_create(&car_threads[i], NULL, car_from_north, (void*)&cars[i]);
				break;
			case 'e':
				cars[i].dir = East;
				pthread_create(&car_threads[i], NULL, car_from_east, (void*)&cars[i]);
				break;
			case 's':
				cars[i].dir = South;
				pthread_create(&car_threads[i], NULL, car_from_south, (void *)&cars[i]);
				break;
			case 'w':
				cars[i].dir = West;
				pthread_create(&car_threads[i], NULL, car_from_west, (void*)&cars[i]);
				break;
			default:
				printf("Wrong Input!\n");
				break;
		}
		usleep(1);
	}
	usleep(1);

	// �ṩ��ʼ�ź�
	pthread_cond_signal(&queueNorth);
	pthread_cond_signal(&queueEast);	
	pthread_cond_signal(&queueSouth);	
	pthread_cond_signal(&queueWest);	
	
	for (int i = 0; i < strlen(argv[1]); i++) 
		pthread_join(car_threads[i], NULL);						// �ȴ�ȫ�������߳̽���
}