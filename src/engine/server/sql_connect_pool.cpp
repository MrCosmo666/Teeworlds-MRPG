#include <mutex>
#include <thread>
#include <stdarg.h>

#include <mysql_connection.h>
#include <cppconn/exception.h>
#include <cppconn/driver.h>

#include <base/system.h>
#include <engine/shared/config.h>
#include "sql_connect_pool.h"

/*
	I don't see the point in using SELECT operations in the thread, 
	since this will lead to unnecessary code, which may cause confusion, 
	and by calculations if (SQL server / server) = localhoset, 
	this will not do any harm (but after the release is complete, 
	it is advisable to use the Thread function with Callback)

	And in General, you should review the SQL system, 
	it works (and has been tested by time and tests), 
	but this implementation is not very narrowly focused
*/

std::mutex tlock;
std::shared_ptr<CConectionPool> CConectionPool::m_Instance;
CConectionPool::CConectionPool()
{
	try
	{
		m_pdriver = get_driver_instance();
	}
	catch(sql::SQLException &e) 
	{ 
		dbg_msg("Sql Exception", "%s", e.what());
		exit(0);
	}
}

CConectionPool::~CConectionPool()
{
	DisconnectConnectionHeap();
}

CConectionPool& CConectionPool::GetInstance()
{
	if (m_Instance.get() == nullptr)
		m_Instance.reset(new CConectionPool());
	return *m_Instance.get();
}

Connection* CConectionPool::CreateConnection()
{
	tlock.lock();
	//

	Connection *pConn = nullptr;
	while(pConn == nullptr)
	{
		try
		{
			pConn = m_pdriver->connect(g_Config.m_SvMySqlHost, g_Config.m_SvMySqlLogin, g_Config.m_SvMySqlPassword);
			pConn->setClientOption("OPT_CHARSET_NAME", "utf8mb4");
			pConn->setSchema(g_Config.m_SvMySqlDatabase);
		}
		catch(sql::SQLException &e) 
		{
			dbg_msg("Sql Exception", "%s", e.what());
			DisconnectConnection(pConn);
			pConn = nullptr;
		}
	}
	m_connlist.push_back(pConn);
	
	//
	tlock.unlock();
	return pConn;
}

void CConectionPool::DisconnectConnectionHeap()
{
	for(auto& iconn : m_connlist)
		DisconnectConnection(iconn);
		
	m_connlist.clear();
}

void CConectionPool::DisconnectConnection(Connection *pConn)
{
	tlock.lock();
	//

	if (pConn)
	{
		try
		{
			pConn->close();
		}
		catch(sql::SQLException &e) 
		{ 
			dbg_msg("Sql Exception", "%s", e.what());		
		}	
	}
	m_connlist.remove(pConn);
	delete pConn;
	pConn = nullptr;

	//
	tlock.unlock();
}

// выполнить операцию INSERT без задержки по времени
void CConectionPool::ID(const char *Table, const char *Buffer, ...)
{
	va_list args;
	va_start(args, Buffer);
	IDS(0, Table, Buffer, args);
	va_end(args);
}

// выполнить операцию INSERT после определенного времени
void CConectionPool::IDS(int Milliseconds, const char *Table, const char *Buffer, ...)
{
	char aBuf[1024];
	va_list VarArgs; 
	va_start(VarArgs, Buffer);
	#if defined(CONF_FAMILY_WINDOWS)
	_vsnprintf(aBuf, sizeof(aBuf), Buffer, VarArgs);
	#else
	vsnprintf(aBuf, sizeof(aBuf), Buffer, VarArgs);
	#endif
	va_end(VarArgs);
	aBuf[sizeof(aBuf) - 1] = '\0';
	std::string Buf = "INSERT INTO " + std::string(Table) + " " + std::string(aBuf) + ";";

	std::thread t([Buf, Milliseconds]()
	{
		if(Milliseconds > 0)
			std::this_thread::sleep_for(std::chrono::milliseconds(Milliseconds));

		Connection* pConn = nullptr;
		try
		{
			pConn = SJK.CreateConnection();
			std::shared_ptr<Statement> STMT(pConn->createStatement());
			STMT->execute(Buf.c_str());
		}
		catch (sql::SQLException & e)
		{
			dbg_msg("sql", "%s", e.what());
		}
		SJK.DisconnectConnection(pConn);
	});
	t.detach();
}

// выполнить операцию UPDATE без задержки по времени
void CConectionPool::UD(const char *Table, const char *Buffer, ...)
{
	va_list args;
	va_start(args, Buffer);
	UDS(0, Table, Buffer, args);
	va_end(args);
}

// выполнить операцию UPDATE после определенного времени
void CConectionPool::UDS(int Milliseconds, const char *Table, const char *Buffer, ...)
{
	char aBuf[1024];
	va_list VarArgs;
	va_start(VarArgs, Buffer);
	#if defined(CONF_FAMILY_WINDOWS)
	_vsnprintf(aBuf, sizeof(aBuf), Buffer, VarArgs);
	#else
	vsnprintf(aBuf, sizeof(aBuf), Buffer, VarArgs);
	#endif
	va_end(VarArgs);
	aBuf[sizeof(aBuf) - 1] = '\0';
	std::string Buf = "UPDATE " + std::string(Table) + " SET " + std::string(aBuf) + ";";
	
	std::thread t([Buf, Milliseconds]()
	{
		if(Milliseconds > 0)
			std::this_thread::sleep_for(std::chrono::milliseconds(Milliseconds));
		
		Connection* pConn = nullptr;
		try
		{
			pConn = SJK.CreateConnection();
			std::shared_ptr<Statement> STMT(pConn->createStatement());
			STMT->execute(Buf.c_str());
		}
		catch(sql::SQLException &e)
		{
			dbg_msg("sql", "%s", e.what());
		}
		SJK.DisconnectConnection(pConn);
	});
	t.detach();
}

// выполнить операцию DELETE без задержки по времени
void CConectionPool::DD(const char *Table, const char *Buffer, ...)
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
	aBuf[sizeof(aBuf) - 1] = '\0';
	std::string Buf = "DELETE FROM " + std::string(Table) + " " + std::string(aBuf) + ";";

	std::thread t([Buf]()
	{
		Connection* pConn = nullptr;
		try
		{
			pConn = SJK.CreateConnection();
			std::shared_ptr<Statement> STMT(pConn->createStatement());
			STMT->execute(Buf.c_str());
		}
		catch (sql::SQLException & e)
		{
			dbg_msg("sql", "%s", e.what());
		}
		SJK.DisconnectConnection(pConn);
	});
	t.detach();
}

// выполнить операцию SELECT без потоков
ResultSet *CConectionPool::SD(const char *Select, const char *Table, const char *Buffer, ...)
{
	char aBuf[1024];
	va_list VarArgs;
	va_start(VarArgs, Buffer);
	#if defined(CONF_FAMILY_WINDOWS)
	_vsnprintf(aBuf, sizeof(aBuf), Buffer, VarArgs);
	#else
	vsnprintf(aBuf, sizeof(aBuf), Buffer, VarArgs);
	#endif  
	va_end(VarArgs);
	aBuf[sizeof(aBuf) - 1] = '\0';
	std::string Buf = "SELECT " + std::string(Select) + " FROM " + std::string(Table) + " " + std::string(aBuf) + ";";

	Connection* pConn = nullptr;
	ResultSet* m_results = nullptr;
	try
	{
		pConn = SJK.CreateConnection();
		std::shared_ptr<Statement> STMT(pConn->createStatement());
		m_results = STMT->executeQuery(Buf.c_str());
	}
	catch (sql::SQLException & e)
	{
		dbg_msg("sql", "%s", e.what());
	}
	SJK.DisconnectConnection(pConn);
	return m_results;
}