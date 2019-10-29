#pragma once
using BYTE = unsigned char;
const static size_t ALIGN = 1024;
const static size_t FREELIST_NUM = 16;
const static size_t MAX_UNIT_SIZE = ALIGN * FREELIST_NUM;

struct unit
{
	unit* ptr_next;
};

static unit* freeList_Array[FREELIST_NUM] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
static BYTE* ptr_start;
static BYTE* ptr_end;

template <class T>
class My_Allocator
{
public:
	typedef T				value_type;
	typedef T*				pointer;
	typedef T&				reference;
	typedef const T*		const_pointer;
	typedef const T&		const_reference;
	typedef size_t			size_type;
	typedef ptrdiff_t		difference_type;

	My_Allocator() = default;
	~My_Allocator() = default;

	template <class U>
	My_Allocator(const My_Allocator<U>& other) noexcept : My_Allocator()
	{

	}

	template <class U>
	struct rebind
	{
		using other = My_Allocator<U>;
	};

	inline pointer address(reference x) const
	{
		return &x;
	}

	inline const_pointer address(const_reference x) const
	{
		return &x;
	}

	pointer allocate(size_type size)
	{
		size_type totalBytes = size * sizeof(value_type);
		if (totalBytes > MAX_UNIT_SIZE)
		{
			return (pointer)(::operator new (totalBytes));
		}
		int index = getListIndex(totalBytes);
		unit* tmp = freeList_Array[index];
		if (tmp == nullptr)
			return (pointer)refill(roundUp(totalBytes));
		freeList_Array[index] = tmp->ptr_next;
		return (pointer)tmp;
	}

	void deallocate(pointer ptr, size_type size)
	{
		size_type totalBytes = size * sizeof(value_type);
		if (totalBytes > MAX_UNIT_SIZE)
		{
			::operator delete(ptr);
			return;
		}
		int index = getListIndex(totalBytes);
		unit* tmp = (unit*)ptr;
		tmp->ptr_next = freeList_Array[index];
		freeList_Array[index] = tmp;
	}

	template <class U>
	inline void destroy(U* ptr)
	{
		ptr ->~U();
	}

	template <class U, class... Args>
	inline void construct(U* ptr, Args&&... argv)
	{
		new (ptr) U(std::forward<Args>(argv)...);
	}

private:
	inline static size_t roundUp(size_t bytes)
	{
		return (bytes + ALIGN - 1) & ~(ALIGN - 1); // û����allocate(0)�����
	}

	inline static size_t getListIndex(size_t bytes)
	{
		return (bytes - 1) / ALIGN;
	}

	static void* refill(size_t bytes)
	{
		int nUnits = 20;
		BYTE* block = blockAllocate(bytes, nUnits);
		unit* currentUnit;
		unit* nextUnit;
		if (nUnits == 1)
			return block;
		int index = getListIndex(bytes);
		currentUnit = (unit*)block;
		freeList_Array[index] = (unit*)((BYTE*)currentUnit + bytes); // ���ɽڵ�ŵ��ڶ������ĵط�
		for (int i = 0; i < nUnits - 1; ++i)
		{
			nextUnit = (unit*)((BYTE*)currentUnit + bytes);
			currentUnit->ptr_next = nextUnit;
			currentUnit = nextUnit; // ��Ϊ����һ�����Կյ�ֱ������һ��
		}
		currentUnit->ptr_next = nullptr; // ���һ������һ��Ϊ��
		return block;
	}

	static BYTE* blockAllocate(size_t size, int& nUnits)
	{
		BYTE* result;
		size_t totalBytes = size * nUnits; // ����һ���ܴ�С
		size_t bytesLeft = ptr_end - ptr_start; // �ڴ����δ����Ĵ�С
		if (bytesLeft >= totalBytes)
		{
			result = ptr_start;
			ptr_start += totalBytes;
			return result;
		}
		else if (bytesLeft >= size)
		{
			nUnits = bytesLeft / size;
			totalBytes = size * nUnits;
			result = ptr_start;
			ptr_start += totalBytes;
			return result;
		}
		else
		{
			size_t byteGet = 3 * totalBytes; // ����������ʽ��ĿǰΪ�����С������
			if (bytesLeft > 0)
			{
				int index = getListIndex(bytesLeft);
				((unit*)ptr_start)->ptr_next = freeList_Array[index];
				freeList_Array[index] = (unit*)ptr_start;
			}
			ptr_start = (BYTE*)(::operator new (byteGet));
			ptr_end = ptr_start + byteGet;
			return blockAllocate(size, nUnits);
		}
	}
};