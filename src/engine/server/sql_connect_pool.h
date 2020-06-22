#ifndef ENGINE_SERVER_SQL_CONNECTIONPOOL_H
#define ENGINE_SERVER_SQL_CONNECTIONPOOL_H

#include <boost/scoped_ptr.hpp>
#include <cppconn/statement.h>

using namespace sql;
#define SJK CConectionPool::GetInstance()

class CConectionPool 
{
	static std::shared_ptr<CConectionPool> m_Instance;
	std::list<class Connection*>m_connlist;
	class Driver *m_pdriver;

	CConectionPool();

	
public:
	~CConectionPool();

	class Connection* CreateConnection();
	void DisconnectConnection(class Connection* pConn);
	void DisconnectConnectionHeap();
	static CConectionPool& GetInstance();

	// функция выборка с бд данных
	class ResultSet* SD(const char *Select, const char *Table, const char *Buffer = "", ...);

	// функция просто обновит данные что будут указаны
	void UD(const char *Table, const char *Buffer, ...);
	void UDS(int Milliseconds, const char *Table, const char *Buffer, ...);

	// функция удаляет что либо из бд
	void DD(const char *Table, const char *Buffer, ...);

	// функция просто вставляет данные
	void ID(const char *Table, const char *Buffer, ...);
	void IDS(int Milliseconds, const char *Table, const char *Buffer, ...);
};

#endif
