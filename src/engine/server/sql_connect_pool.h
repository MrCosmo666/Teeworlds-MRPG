#ifndef ENGINE_SERVER_SQL_CONNECTIONPOOL_H
#define ENGINE_SERVER_SQL_CONNECTIONPOOL_H

#include <mysql_connection.h>
#include <cppconn/exception.h>
#include <cppconn/driver.h>
#include <cppconn/statement.h>
#include <functional>

using namespace sql;
#define SJK CConectionPool::GetInstance()

class CConectionPool 
{
	CConectionPool();
	
	static std::shared_ptr<CConectionPool> m_Instance;
	std::list<Connection*>m_ConnList;
	Driver *m_pDriver;

	void InsertFormated(int Milliseconds, const char *Table, const char *Buffer, va_list args);
	void UpdateFormated(int Milliseconds, const char *Table, const char *Buffer, va_list args);
	void DeleteFormated(int Milliseconds, const char *Table, const char *Buffer, va_list args);

public:
	~CConectionPool();

	class Connection* CreateConnection();
	void DisconnectConnection(Connection* pConnection);
	void DisconnectConnectionHeap();
	static CConectionPool& GetInstance();

	// функция просто вставляет данные
	void ID(const char *Table, const char *Buffer, ...);
	void IDS(int Milliseconds, const char *Table, const char *Buffer, ...);
	
	// функция просто обновит данные что будут указаны
	void UD(const char *Table, const char *Buffer, ...);
	void UDS(int Milliseconds, const char *Table, const char *Buffer, ...);
	
	// функция просто удаляет данные что будут указаны
	void DD(const char *Table, const char *Buffer, ...);
	void DDS(int Milliseconds, const char *Table, const char *Buffer, ...);

	// функция выборка с бд данных
	ResultSet* SD(const char *Select, const char *Table, const char *Buffer = "", ...);
	
	// функция выборка с бд данных в потоке через лямбду
	void SDT(const char* Select, const char* Table, std::function<void(ResultSet*)> func, const char* Buffer = "", ...);
};

#endif
