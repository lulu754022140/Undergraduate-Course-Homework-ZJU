Vector<T>
�����������Ժͷ���
public:
Vector(); Ĭ�Ϲ��캯��
Vector(int size); ������С�Ĺ��캯��
Vector(const Vector& r); �������캯��
virtual ~Vector(); ��������
T& operator[](int index) : throw(IndexOutofBounds); ��������� ����ֵ �±�Խ����׳��쳣IndexOutofBounds
int size(); ����Vector�Ĵ�С
int inflate(int addSize); ����Vector ����addSize��Ԫ�ؿռ�
private:
T *m_pElements;
int m_nSize;

��������main.cpp

ע�������һ���������û�м�catch�ʳ����ֹͣ������