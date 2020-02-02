#include <mutex>

#include <base/system.h>
#include <engine/shared/config.h>
#include "sql_connect_pool.h"

std::mutex tlock;

// ЗАМЕНИ ПОТОМ ЭТО ВСЕ БЛЯТЬ ХУЙНЮ ЭТУ ЕБАНУЯ ЧТО БЕСПОЛЕЗНАЯ
std::shared_ptr<CConectionPool> CConectionPool::m_Instance;
CConectionPool::CConectionPool(int maxSize) : m_MaxSize(maxSize), m_CurretSize(0)
{
	try
	{
		m_pdriver = get_driver_instance();
	}
	catch(sql::SQLException &e) 
	{
		dbg_msg("Sql Exception", "%s", e.what());
	}

#if defined(CONF_FAMILY_UNIX)
	InitConnection(m_MaxSize);
#endif
}

CConectionPool::~CConectionPool()
{
	DisconnectConnectionPool();
}

CConectionPool& CConectionPool::GetInstance()
{
	if (m_Instance.get() == 0)
		m_Instance.reset(new CConectionPool(g_Config.m_SvMySqlMaxPool));

	return *m_Instance.get();
}

void CConectionPool::InitConnection(int Size)
{
	for (int i = 0; i < Size; ++i)
	{
		tlock.lock();
		
		Connection *pConn = CreateConnection();
		m_connlist.push_back(pConn);
		++m_CurretSize;

		tlock.unlock();
	}
}

Connection* CConectionPool::CreateConnection()
{
	Connection *pConn = NULL;
	while(pConn == NULL || pConn && !pConn->isValid())
	{
		try
		{
			if (pConn && !pConn->isValid())
			{
				delete []pConn;
				pConn = NULL;
			}

			pConn = m_pdriver->connect(g_Config.m_SvMySqlHost, g_Config.m_SvMySqlLogin, g_Config.m_SvMySqlPassword);
			pConn->setClientOption("OPT_CONNECT_TIMEOUT", "10");
			pConn->setClientOption("OPT_READ_TIMEOUT", "10");
			pConn->setClientOption("OPT_WRITE_TIMEOUT", "20");
			pConn->setClientOption("OPT_RECONNECT", "1");
			pConn->setClientOption("OPT_CHARSET_NAME", "utf8mb4");
		}
		catch(sql::SQLException &e) 
		{ 
			dbg_msg("Sql Exception", "%s", e.what());
		}
	}
	pConn->setSchema(g_Config.m_SvMySqlDatabase);
	++m_CurretSize;
	return pConn;
}

Connection* CConectionPool::GetConnection()
{
	tlock.lock();
	//

	Connection* pConn = NULL;
	if (m_connlist.size() > 0)
	{
		// получаем первый элемент и вырезаем
		pConn = m_connlist.front();
		m_connlist.pop_front();

		// проверяем подключение
		if (pConn)
		{
			if (pConn->isClosed())
			{
				DisconnectConnection(pConn);
				pConn = CreateConnection();
			}
			if (!pConn->isValid())
				pConn->reconnect();
		}
		else
			pConn = CreateConnection();
		
		//
		tlock.unlock();
		return pConn;
	}
	pConn = CreateConnection();

	//
	tlock.unlock();
	return pConn;
}

void CConectionPool::ReleaseConnection(Connection* pConn)
{
	tlock.lock();
	//

	if(m_CurretSize > m_MaxSize)
		DisconnectConnection(pConn);
	else
	{
		// возращаем подключение в список
		m_connlist.push_back(pConn);
	}

	//
	tlock.unlock();
}

void CConectionPool::DisconnectConnectionPool()
{
	for(auto& iconn : m_connlist)
		DisconnectConnection(iconn);

	m_CurretSize = 0;
	m_connlist.clear();
}

void CConectionPool::DisconnectConnection(Connection *pConn)
{
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
	--m_CurretSize;
	delete pConn;
	pConn = NULL;
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
	
	std::thread t([Table, aBuf]()
	{
		try
		{
			Connection *iconn = SJK.GetConnection();
			boost::scoped_ptr<Statement> STMT(iconn->createStatement()); 

			std::string Buf = "UPDATE " + std::string(Table) + " SET " + std::string(aBuf) + ";";
			STMT->execute(Buf.c_str());
			SJK.ReleaseConnection(iconn);
		}
		catch(sql::SQLException &e)
		{
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

	std::thread t([Table, aBuf]() 
	{
		try
		{
			std::string Buf = "DELETE FROM " + std::string(Table) + " " + std::string(aBuf) + ";";
			Connection *iconn = SJK.GetConnection();
			boost::scoped_ptr<Statement> STMT(iconn->createStatement()); 

			STMT->execute(Buf.c_str());
			SJK.ReleaseConnection(iconn);
		}
		catch(sql::SQLException &e)
		{
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

	std::thread t([Table, aBuf]() 
	{
		try
		{
			std::string Buf = "INSERT INTO " + std::string(Table) + " " + std::string(aBuf) + ";";
			Connection *iconn = SJK.GetConnection();
			boost::scoped_ptr<Statement> STMT(iconn->createStatement()); 

			STMT->execute(Buf.c_str());
			SJK.ReleaseConnection(iconn);
		}
		catch(sql::SQLException &e)
		{
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

	ResultSet* m_results;
	aBuf[sizeof(aBuf) - 1] = '\0';
	std::string Buf = "SELECT " + std::string(Select) + " FROM " + std::string(Table) + " " + std::string(aBuf) + ";";

	try
	{
		Connection* iconn = SJK.GetConnection();
		boost::scoped_ptr<Statement> STMT(iconn->createStatement());
		m_results = STMT->executeQuery(Buf.c_str());
		SJK.ReleaseConnection(iconn);
	}
	catch(sql::SQLException &e)
	{
		dbg_msg("sql", "%s", e.what());
	}
	return m_results;
}
