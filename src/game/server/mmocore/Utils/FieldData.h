/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_SERVER_UTILS_FIELD_DATA_H
#define GAME_SERVER_UTILS_FIELD_DATA_H

// There is an idea about automatic unloading from the database to save changes in the fields.
// With the help of a list. In the future I think it will be necessary to achieve this

// TODO: add exceptions

// ref declaration
template<typename T> struct refval { static typename std::decay<T>::type value; };
template<typename T> typename std::decay<T>::type refval<T>::value = T {};

// field data
enum FieldTypes
{
	FIELD_TYPE_UNSET,
	FIELD_TYPE_INTEGER,
	FIELD_TYPE_FLOAT,
	FIELD_TYPE_STRING,
};

// getter type
template < typename Type >
struct GetterType
{
	// constexpr for to control at compile time
	static constexpr int Get()
	{
		if(typeid(Type) == typeid(int))
			return FIELD_TYPE_INTEGER;
		if(typeid(Type) == typeid(float))
			return FIELD_TYPE_FLOAT;
		return FIELD_TYPE_STRING;
	}
};

// idk not it uncomplete type need overloading operators
union VariantUnion
{
	int m_Integer;
	float m_Floating;
	std::string m_String;

	VariantUnion() {}
	~VariantUnion() {}
	explicit VariantUnion(const int Value) :m_Integer(Value) {}
	explicit VariantUnion(const float Value) :m_Floating(Value) {}
	explicit VariantUnion(std::string Value) : m_String(std::move(Value)) {}
};

class FieldData
{
	template < int Typechecker >
	void reformat()
	{
		if(Typechecker == FIELD_TYPE_STRING || Typechecker == FIELD_TYPE_UNSET)
			return;

		if(m_Type != Typechecker)
		{
			if(Typechecker == FIELD_TYPE_FLOAT)
			{
				if(m_Type == FIELD_TYPE_INTEGER)
				{
					float Reserve = (float)m_Data.m_Integer;
					m_Data.m_Floating = Reserve;
				}
				else if(m_Type == FIELD_TYPE_STRING)
				{
					float Reserve = (float)std::strtol(m_Data.m_String.c_str(), nullptr, 0);
					m_Data.m_Floating = Reserve;
				}
			}
			else if(Typechecker == FIELD_TYPE_INTEGER)
			{
				if(m_Type == FIELD_TYPE_FLOAT)
				{
					int Reserve = (int)m_Data.m_Floating;
					m_Data.m_Integer = Reserve;
				}
				else if(m_Type == FIELD_TYPE_STRING)
				{
					int Reserve = (int)std::strtol(m_Data.m_String.c_str(), nullptr, 0);
					m_Data.m_Integer = Reserve;
				}
			}
		}
	}

	template < int Newtype >
	void normalize()
	{
		if(Newtype != FIELD_TYPE_STRING)
			clear();
		m_Type = Newtype;
	}

	void clear()
	{
		if(m_Type == FIELD_TYPE_STRING)
			m_Data.m_String.~basic_string();
	}

	// data for base
	int m_Type;
	std::string m_Field;
	std::string m_Name;
	VariantUnion m_Data;

public:
	FieldData() : m_Type(FIELD_TYPE_UNSET) { }
	FieldData(int Type, const char* pField, const char* pName) : m_Type(Type), m_Field(pField), m_Name(pName) { }
	FieldData(const FieldData& pField) { *this = pField; }
	~FieldData() { clear(); }

	FieldData& operator=(const FieldData& pField)
	{
		m_Field = pField.m_Field;
		m_Name = pField.m_Name;
		m_Type = pField.m_Type;
		if(m_Type == FIELD_TYPE_INTEGER)
			m_Data.m_Integer = pField.m_Data.m_Integer;
		else if(m_Type == FIELD_TYPE_FLOAT)
			m_Data.m_Floating = pField.m_Data.m_Floating;
		else if(m_Type == FIELD_TYPE_STRING)
			m_Data.m_String = pField.m_Data.m_String;
		return *this;
	}

	// overloading TODO: refactor it
	operator int() const
	{
		if(m_Type == FIELD_TYPE_FLOAT) // to integer
			return static_cast<int>(m_Data.m_Floating);
		if(m_Type == FIELD_TYPE_STRING) // to string
			return (int)std::strtol(m_Data.m_String.c_str(), nullptr, 0);
		return m_Data.m_Integer;
	}
	operator float() const
	{
		if(m_Type == FIELD_TYPE_INTEGER)
			return static_cast<float>(m_Data.m_Integer);
		if(m_Type == FIELD_TYPE_STRING)
			return (float)std::strtol(m_Data.m_String.c_str(), nullptr, 0);
		return m_Data.m_Floating;
	}
	operator const char* () const
	{
		if(m_Type == FIELD_TYPE_INTEGER)
		{
			refval<std::string>::value = std::to_string(m_Data.m_Integer);
			return refval<std::string>::value.c_str();
		}
		if(m_Type == FIELD_TYPE_FLOAT)
		{
			refval<std::string>::value = std::to_string(m_Data.m_Floating);
			return refval<std::string>::value.c_str();
		}
		return m_Data.m_String.c_str();
	}

	// setters overloading
	FieldData& operator =(const int& Value)
	{
		normalize < FIELD_TYPE_INTEGER >();
		m_Data.m_Integer = Value;
		return *this;
	}
	FieldData& operator =(const float& Value)
	{
		normalize < FIELD_TYPE_FLOAT >();
		m_Data.m_Floating = Value;
		return *this;
	}
	FieldData& operator =(const char* Value)
	{
		normalize < FIELD_TYPE_STRING >();
		new (&(m_Data.m_String)) std::string(Value);
		return *this;
	}

	int& operator+=(const int& Value)
	{
		reformat < FIELD_TYPE_INTEGER >();
		normalize < FIELD_TYPE_INTEGER >();
		m_Data.m_Integer += Value;
		return m_Data.m_Integer;
	}

	int& operator-=(const int& Value)
	{
		reformat < FIELD_TYPE_INTEGER >();
		normalize < FIELD_TYPE_INTEGER >();
		m_Data.m_Integer -= Value;
		return m_Data.m_Integer;
	}

	int operator++(int)
	{
		reformat < FIELD_TYPE_INTEGER >();
		normalize < FIELD_TYPE_INTEGER >();
		m_Data.m_Integer++;
		return m_Data.m_Integer;
	}

	int* operator&()
	{
		reformat < FIELD_TYPE_INTEGER >();
		normalize < FIELD_TYPE_INTEGER >();
		return &m_Data.m_Integer;
	}

	const char* getFieldName() const { return m_Field.c_str(); }
	const char* getName() const { return m_Name.c_str(); }
};

// container
class CFieldsData
{
	class CNode
	{
	public:
		int m_Index;
		FieldData m_Data;
		CNode* m_pNext;
		explicit CNode(const int Index, const FieldData& Data) : m_Index(Index), m_Data(Data), m_pNext(nullptr) {}
	};
	CNode* m_pHead;

public:
	CFieldsData();
	~CFieldsData();

	template < typename Type, int Index >
	void add_field(const char* pField, const char* pName);

	FieldData& get_field(int Index) const;

	// overload operators by index []
	FieldData& operator[](int Index) const { return get_field(Index); }
	FieldData& operator[](const char* FieldName) const
	{
		CNode* pHead = m_pHead;
		while(pHead)
		{
			if(pHead->m_Data.getFieldName() == FieldName)
				return pHead->m_Data;
			pHead = pHead->m_pNext;
		}
		// TODO: fix it xd
		return m_pHead ? m_pHead->m_Data : refval<FieldData>::value;
	}
};

inline CFieldsData::CFieldsData()
{
	m_pHead = nullptr;
}

inline CFieldsData::~CFieldsData()
{
	// we delete lv pNext use rv
	CNode* pHead = m_pHead;
	while(pHead)
	{
		CNode* pNextElement = pHead->m_pNext;
		delete pHead;
		pHead = pNextElement;
	}
}

template < typename Type, int Index >
void CFieldsData::add_field(const char* pField, const char* pName)
{
	const FieldData Field(GetterType<Type>::Get(), pField, pName);
	if(m_pHead == nullptr)
	{
		m_pHead = new CNode(Index, Field);
		return;
	}

	CNode* pHead = m_pHead;
	while(pHead)
	{
		if(pHead->m_Index == Index)
			return;

		if(!pHead->m_pNext)
		{
			pHead->m_pNext = new CNode(Index, Field);
			return;
		}
		pHead = pHead->m_pNext;
	}
}

inline FieldData& CFieldsData::get_field(const int Index) const
{
	CNode* pHead = m_pHead;
	while(pHead)
	{
		if(pHead->m_Index == Index)
			return pHead->m_Data;
		pHead = pHead->m_pNext;
	}

	// TODO: Fix it xd
	return m_pHead ? m_pHead->m_Data : refval<FieldData>::value;
}

#endif
