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

	This approach works if the old query is not executed before,
	 a new query it will create a reserve.

	 It may seem that it does not use Pool, 
	 but in fact it is and is created as a reserve when running
	 <tlock>
*/

// #####################################################
// SQL CONNECTION POOL
// #####################################################
std::mutex ThreadLock;
std::shared_ptr<CConectionPool> CConectionPool::m_Instance;
CConectionPool::CConectionPool()
{
	try
	{
		m_pDriver = get_driver_instance();
	}
	catch(SQLException &e) 
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
	ThreadLock.lock();
	Connection *pConnection = nullptr;
	while(pConnection == nullptr)
	{
		try
		{
			pConnection = m_pDriver->connect(g_Config.m_SvMySqlHost, g_Config.m_SvMySqlLogin, g_Config.m_SvMySqlPassword);
			pConnection->setClientOption("OPT_CHARSET_NAME", "utf8mb4");
			pConnection->setSchema(g_Config.m_SvMySqlDatabase);
		}
		catch(SQLException &e) 
		{
			dbg_msg("Sql Exception", "%s", e.what());
			DisconnectConnection(pConnection);
			pConnection = nullptr;
		}
	}
	m_ConnList.push_back(pConnection);
	ThreadLock.unlock();
	return pConnection;
}

void CConectionPool::DisconnectConnectionHeap()
{
	for(auto& iconn : m_ConnList)
		DisconnectConnection(iconn);
		
	m_ConnList.clear();
}

void CConectionPool::DisconnectConnection(Connection *pConnection)
{
	ThreadLock.lock();
	if (pConnection)
	{
		try
		{
			pConnection->close();
		}
		catch(SQLException &e) 
		{ 
			dbg_msg("Sql Exception", "%s", e.what());		
		}	
	}
	m_ConnList.remove(pConnection);
	delete pConnection;
	pConnection = nullptr;
	ThreadLock.unlock();
}

// #####################################################
// INSERT SQL
// #####################################################
void CConectionPool::ID(const char *Table, const char *Buffer, ...)
{
	va_list Arguments;
	va_start(Arguments, Buffer);
	InsertFormated(0, Table, Buffer, Arguments);
	va_end(Arguments);
}

void CConectionPool::IDS(int Milliseconds, const char *Table, const char *Buffer, ...)
{
	va_list Arguments;
	va_start(Arguments, Buffer);
	InsertFormated(Milliseconds, Table, Buffer, Arguments);
	va_end(Arguments);	
}

void CConectionPool::InsertFormated(int Milliseconds, const char *Table, const char *Buffer, va_list args)
{
	char aBuf[1024];
	#if defined(CONF_FAMILY_WINDOWS)
	_vsnprintf(aBuf, sizeof(aBuf), Buffer, args);
	#else
	vsnprintf(aBuf, sizeof(aBuf), Buffer, args);
	#endif
	aBuf[sizeof(aBuf) - 1] = '\0';
	std::string Buf = "INSERT INTO " + std::string(Table) + " " + std::string(aBuf) + ";";
	std::thread Thread([Buf, Milliseconds]()
	{
		if(Milliseconds > 0)
			std::this_thread::sleep_for(std::chrono::milliseconds(Milliseconds));

		Connection* pConnection = nullptr;
		try
		{
			pConnection = SJK.CreateConnection();
			std::shared_ptr<Statement> STMT(pConnection->createStatement());
			STMT->execute(Buf.c_str());
		}
		catch (SQLException & e)
		{
			dbg_msg("sql", "%s", e.what());
		}
		SJK.DisconnectConnection(pConnection);
	});
	Thread.detach();
}

// #####################################################
// UPDATE SQL
// #####################################################
void CConectionPool::UD(const char *Table, const char *Buffer, ...)
{
	va_list Arguments;
	va_start(Arguments, Buffer);
	UpdateFormated(0, Table, Buffer, Arguments);
	va_end(Arguments);
}

void CConectionPool::UDS(int Milliseconds, const char *Table, const char *Buffer, ...)
{
	va_list Arguments;
	va_start(Arguments, Buffer);
	UpdateFormated(Milliseconds, Table, Buffer, Arguments);
	va_end(Arguments);
}

void CConectionPool::UpdateFormated(int Milliseconds, const char *Table, const char *Buffer, va_list args)
{
	char aBuf[1024];
	#if defined(CONF_FAMILY_WINDOWS)
	_vsnprintf(aBuf, sizeof(aBuf), Buffer, args);
	#else
	vsnprintf(aBuf, sizeof(aBuf), Buffer, args);
	#endif
	aBuf[sizeof(aBuf) - 1] = '\0';
	std::string Query = "UPDATE " + std::string(Table) + " SET " + std::string(aBuf) + ";";
	std::thread Thread([Query, Milliseconds]()
	{
		if(Milliseconds > 0)
			std::this_thread::sleep_for(std::chrono::milliseconds(Milliseconds));
		
		Connection* pConnection = nullptr;
		try
		{
			pConnection = SJK.CreateConnection();
			std::shared_ptr<Statement> STMT(pConnection->createStatement());
			STMT->execute(Query.c_str());
		}
		catch(SQLException &e)
		{
			dbg_msg("sql", "%s", e.what());
		}
		SJK.DisconnectConnection(pConnection);
	});
	Thread.detach();
}

// #####################################################
// DELETE SQL
// #####################################################
void CConectionPool::DD(const char *Table, const char *Buffer, ...)
{
	va_list Arguments;
	va_start(Arguments, Buffer);
	DeleteFormated(0, Table, Buffer, Arguments);
	va_end(Arguments);
}

void CConectionPool::DDS(int Milliseconds, const char *Table, const char *Buffer, ...)
{
	va_list Arguments;
	va_start(Arguments, Buffer);
	DeleteFormated(Milliseconds, Table, Buffer, Arguments);
	va_end(Arguments);
}

void CConectionPool::DeleteFormated(int Milliseconds, const char *Table, const char *Buffer, va_list args)
{
	char aBuf[256];
	#if defined(CONF_FAMILY_WINDOWS)
	_vsnprintf(aBuf, sizeof(aBuf), Buffer, args);
	#else
	vsnprintf(aBuf, sizeof(aBuf), Buffer, args);
	#endif
	aBuf[sizeof(aBuf) - 1] = '\0';
	std::string Query = "DELETE FROM " + std::string(Table) + " " + std::string(aBuf) + ";";
	std::thread Thread([Query, Milliseconds]()
	{
		if(Milliseconds > 0)
			std::this_thread::sleep_for(std::chrono::milliseconds(Milliseconds));
		
		Connection* pConnection = nullptr;
		try
		{
			pConnection = SJK.CreateConnection();
			std::shared_ptr<Statement> STMT(pConnection->createStatement());
			STMT->execute(Query.c_str());
		}
		catch (SQLException & e)
		{
			dbg_msg("sql", "%s", e.what());
		}
		SJK.DisconnectConnection(pConnection);
	});
	Thread.detach();
}

// #####################################################
// SELECT SQL
// #####################################################
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
	std::string Query = "SELECT " + std::string(Select) + " FROM " + std::string(Table) + " " + std::string(aBuf) + ";";
	Connection* pConnection = nullptr;
	ResultSet* pResult = nullptr;
	try
	{
		pConnection = SJK.CreateConnection();
		std::shared_ptr<Statement> STMT(pConnection->createStatement());
		pResult = STMT->executeQuery(Query.c_str());
	}
	catch (SQLException & e)
	{
		dbg_msg("sql", "%s", e.what());
	}
	SJK.DisconnectConnection(pConnection);
	return pResult;
}

void CConectionPool::SDT(const char* Select, const char* Table, std::function<void(ResultSet*)> func, const char* Buffer, ...)
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
	const std::string Query = "SELECT " + std::string(Select) + " FROM " + std::string(Table) + " " + std::string(aBuf) + ";";
	std::thread Thread([Query, func]()
	{
		Connection* pConnection = nullptr;
		try
		{
			pConnection = SJK.CreateConnection();
			std::shared_ptr<Statement> STMT(pConnection->createStatement());
			func(STMT->executeQuery(Query.c_str()));
		}
		catch(SQLException& e)
		{
			dbg_msg("sql", "%s", e.what());
		}
		SJK.DisconnectConnection(pConnection);
	});
	Thread.detach();
}
