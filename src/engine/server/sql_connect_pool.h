#ifndef ENGINE_SERVER_SQL_CONNECTIONPOOL_H
#define ENGINE_SERVER_SQL_CONNECTIONPOOL_H

#include <thread>
#include <stdarg.h>
#include <base/detect.h>

#include <mysql_connection.h>

#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/statement.h>

using namespace sql;
#define SJK CConectionPool::GetInstance()

class CConectionPool 
{
	static std::shared_ptr<CConectionPool> m_Instance;
	std::list<Connection*>m_connlist;
	Driver *m_pdriver;

	CConectionPool();

	
public:
	~CConectionPool();

	Connection* CreateConnection();
	void DisconnectConnection(Connection* pConn);
	void DisconnectConnectionHeap();
	static CConectionPool& GetInstance();

	// функция выборка с бд данных
	ResultSet* SD(const char *Select, const char *Table, const char *Buffer = "", ...);

	// функция просто обновит данные что будут указаны
	void UD(const char *Table, const char *Buffer, ...);

	// функция удаляет что либо из бд
	void DD(const char *Table, const char *Buffer, ...);

	// функция просто вставляет данные
	void ID(const char *Table, const char *Buffer, ...);
};

#endif
