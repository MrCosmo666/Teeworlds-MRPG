#include <mutex>

#include <base/system.h>
#include <engine/shared/config.h>
#include "sql_connect_pool.h"

std::mutex tlock;

// ЗАМЕНИ ПОТОМ ЭТО ВСЕ БЛЯТЬ ХУЙНЮ ЭТУ ЕБАНУЯ ЧТО БЕСПОЛЕЗНАЯ
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
	if (m_Instance.get() == 0)
		m_Instance.reset(new CConectionPool());

	return *m_Instance.get();
}

Connection* CConectionPool::CreateConnection()
{
	tlock.lock();
	//

	Connection *pConn = NULL;
	while(pConn == NULL)
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
			pConn = NULL;
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
	pConn = NULL;

	//
	tlock.unlock();
}

void CConectionPool::UD(const char *Table, const char *Buffer, ...)
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
	aBuf[sizeof(aBuf) - 1] = '\0';

	std::string Buf = "UPDATE " + std::string(Table) + " SET " + std::string(aBuf) + ";";
	dbg_msg("sql", "%s", Buf.c_str());

	std::thread t([Buf]()
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
			if (pConn)
				SJK.DisconnectConnection(pConn);

			dbg_msg("sql", "%s", e.what());
		}
	});
	t.detach();
}

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
	dbg_msg("sql", "%s", Buf.c_str());

	std::thread t([Buf]()
	{
		Connection* pConn = NULL;
		try
		{
			pConn = SJK.CreateConnection();
			std::shared_ptr<Statement> STMT(pConn->createStatement());

			STMT->execute(Buf.c_str());
			SJK.DisconnectConnection(pConn);
		}
		catch (sql::SQLException & e)
		{
			if (pConn)
				SJK.DisconnectConnection(pConn);

			dbg_msg("sql", "%s", e.what());
		}
	});
	t.detach();
}

void CConectionPool::ID(const char *Table, const char *Buffer, ...)
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
	aBuf[sizeof(aBuf) - 1] = '\0';

	std::string Buf = "INSERT INTO " + std::string(Table) + " " + std::string(aBuf) + ";";
	dbg_msg("sql", "%s", Buf.c_str());

	std::thread t([Buf]()
	{
		Connection* pConn = NULL;
		try
		{
			pConn = SJK.CreateConnection();
			std::shared_ptr<Statement> STMT(pConn->createStatement());

			STMT->execute(Buf.c_str());
			SJK.DisconnectConnection(pConn);
		}
		catch (sql::SQLException & e)
		{
			if (pConn)
				SJK.DisconnectConnection(pConn);

			dbg_msg("sql", "%s", e.what());
		}
	});
	t.detach();
}

ResultSet *CConectionPool::SD(const char *Select, const char *Table, const char *Buffer, ...)
{
	va_list VarArgs;
	va_start(VarArgs, Buffer);
	#if defined(CONF_FAMILY_WINDOWS)
	char aBuf[2048];
	_vsnprintf(aBuf, sizeof(aBuf), Buffer, VarArgs);
	#else
	char aBuf[str_length(Buffer) + 64];
	vsnprintf(aBuf, sizeof(aBuf), Buffer, VarArgs);
	#endif  
	va_end(VarArgs);

	aBuf[sizeof(aBuf) - 1] = '\0';
	std::string Buf = "SELECT " + std::string(Select) + " FROM " + std::string(Table) + " " + std::string(aBuf) + ";";
	dbg_msg("sql", "%s", Buf.c_str());

	Connection* pConn = NULL;
	ResultSet* m_results = NULL;

	try
	{
		pConn = SJK.CreateConnection();
		std::shared_ptr<Statement> STMT(pConn->createStatement());

		m_results = STMT->executeQuery(Buf.c_str());
		SJK.DisconnectConnection(pConn);			
	}
	catch (sql::SQLException & e)
	{
		if (pConn)
			SJK.DisconnectConnection(pConn);

		dbg_msg("sql", "%s", e.what());
	}
	return m_results;
}
