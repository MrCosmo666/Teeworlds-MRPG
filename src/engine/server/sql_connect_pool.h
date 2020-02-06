#ifndef ENGINE_SERVER_SQL_CONNECTIONPOOL_H
#define ENGINE_SERVER_SQL_CONNECTIONPOOL_H

#include <thread>
#include <stdarg.h>
#include <base/detect.h>

#include <mysql_connection.h>

#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/statement.h>

#define START_THREAD() \
std::thread t([&]() \
{ \

#define END_THREAD() \
}); \
t.detach(); \


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

	// шаблоная функция после ошибки в Sql использует lambda функцию
	template <typename T> 
	void UDS(const char *Table, T Exception, const char *Buffer, ...)
	{
		char aBuf[512];
		va_list VarArgs;
		va_start(VarArgs, Buffer);
		#if defined(CONF_FAMILY_WINDOWS)
		_vsnprintf(aBuf, sizeof(aBuf), Buffer, VarArgs);
		#else
		vsnprintf(aBuf, sizeof(aBuf), Buffer, VarArgs);
		#endif
		va_end(VarArgs);

		std::string Buf = "UPDATE " + std::string(Table) + " SET " + std::string(aBuf) + ";";
		std::thread t([Buf, Exception]()
		{
			Connection* pConn = NULL;
			try
			{
				pConn = SJK.CreateConnection();
				std::shared_ptr<Statement> STMT(pConn->createStatement());

				STMT->execute(Buf.c_str());
				SJK.DisconnectConnection(pConn);
			}
			catch(sql::SQLException &e)
			{
				Exception();
				if (pConn)
					SJK.DisconnectConnection(pConn);

				dbg_msg("sql", "%s", e.what());
			}
		});
		t.detach();
	};

	template <typename T> 
	void DDS(const char *Table, T Exception, const char *Buffer, ...)
	{
		char aBuf[256];
		va_list VarArgs;
		va_start(VarArgs, Buffer);
		#if defined(CONF_FAMILY_WINDOWS)
		_vsnprintf(aBuf, sizeof(aBuf), Buffer, VarArgs);
		#else
		vsnprintf(aBuf, sizeof(aBuf), Buffer, VarArgs);
		#endif
		va_end(VarArgs);

		std::string Buf = "DELETE FROM " + std::string(Table) + " " + std::string(aBuf) + ";";
		std::thread t([Buf, Exception]()
		{
			Connection* pConn = NULL;
			try
			{
				pConn = SJK.CreateConnection();
				std::shared_ptr<Statement> STMT(pConn->createStatement());

				STMT->execute(Buf.c_str());
				SJK.DisconnectConnection(pConn);
			}
			catch(sql::SQLException &e)
			{
				Exception();
				if (pConn)
					SJK.DisconnectConnection(pConn);

				dbg_msg("sql", "%s", e.what());
			}
		});
		t.detach();
	};

	template <typename T> 
	void IDS(const char *Table, T Exception, const char *Buffer, ...)
	{
		char aBuf[512];
		va_list VarArgs; 
		va_start(VarArgs, Buffer);
		#if defined(CONF_FAMILY_WINDOWS)
		_vsnprintf(aBuf, sizeof(aBuf), Buffer, VarArgs);
		#else
		vsnprintf(aBuf, sizeof(aBuf), Buffer, VarArgs);
		#endif
		va_end(VarArgs);

		std::string Buf = "INSERT INTO " + std::string(Table) + " " + std::string(aBuf) + ";";
		std::thread t([Buf, Exception]()
		{
			Connection* pConn = NULL;
			try
			{
				pConn = SJK.CreateConnection();
				std::shared_ptr<Statement> STMT(pConn->createStatement());

				STMT->execute(Buf.c_str());
				SJK.DisconnectConnection(pConn);
			}
			catch(sql::SQLException &e)
			{
				Exception();
				if (pConn)
					SJK.DisconnectConnection(pConn);

				dbg_msg("sql", "%s", e.what());
			}
		});
		t.detach();
	}

};

#endif
