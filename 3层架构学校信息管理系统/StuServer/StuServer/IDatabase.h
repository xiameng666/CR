#pragma once

#include <mysql.h>
#include <stdio.h>
#include <memory>
#include <afx.h>
#pragma comment(lib, "libmysql.lib")

struct StmtResult {
    MYSQL_STMT* stmt;
    MYSQL_BIND* bind;
    unsigned int fields;

    StmtResult(MYSQL_STMT* s) : stmt(s), bind(nullptr), fields(0) {}

    ~StmtResult() {
        if (bind) {
            for (unsigned int i = 0; i < fields; i++) {
                if (bind[i].buffer) free(bind[i].buffer);
            }
            delete[] bind;
        }
    }

    // 绑定结果集
    bool BindResult(unsigned int maxLength = 1024) {
        fields = mysql_stmt_field_count(stmt);
        bind = new MYSQL_BIND[fields];
        memset(bind, 0, sizeof(MYSQL_BIND) * fields);

        // 为每列分配缓冲区
        for (unsigned int i = 0; i < fields; i++) {
            bind[i].buffer_type = MYSQL_TYPE_STRING;
            bind[i].buffer = malloc(maxLength);
            bind[i].buffer_length = maxLength;
        }

        // 绑定结果集
        return mysql_stmt_bind_result(stmt, bind) == 0;
    }

    // 获取数据
    bool Fetch() {
        return mysql_stmt_fetch(stmt) == 0;
    }

    const char* GetString(size_t col) {
        if (col >= fields || !bind[col].buffer) return nullptr;
        return (const char*)bind[col].buffer;
    }
};




class IDatabase {
protected:
    // SQL执行器基类
    class SQLExecutor {
    protected:
        MYSQL* m_mysql;

    public:
        SQLExecutor(MYSQL* mysql) : m_mysql(mysql) {}
        virtual ~SQLExecutor() {}
        virtual void FreeResult() = 0;
    };

    // 常规SQL执行器
    class NormalExecutor : public SQLExecutor {
    public:
        NormalExecutor(MYSQL* mysql) : SQLExecutor(mysql) {}

        void FreeResult() override {
            if (!m_mysql) return;
            MYSQL_RES* result = mysql_store_result(m_mysql);
            if (result) {
                mysql_free_result(result);
            }
        }

        // 执行无返回值的SQL
        bool Execute(const char* sql) {
            if (!m_mysql || !sql) return false;

            TRACE("[SQL]执行sql：%s\n", sql);
            FreeResult();

            if (mysql_query(m_mysql, sql)) {
                TRACE("[SQL]执行失败：%s，错误代码：%d\n",
                    mysql_error(m_mysql), mysql_errno(m_mysql));
                return false;
            }
            return true;
        }

        // 执行有返回值的SQL
        MYSQL_RES* ExecuteQuery(const char* sql) {
            if (!Execute(sql)) return nullptr;
            return mysql_store_result(m_mysql);
        }
    };

    // 预编译SQL执行器
    class PrecompileExecutor : public SQLExecutor {
    private:
        MYSQL_STMT* m_stmt;

    public:
        PrecompileExecutor(MYSQL* mysql) : SQLExecutor(mysql), m_stmt(nullptr) {}
        ~PrecompileExecutor() {
            if (m_stmt) {
                mysql_stmt_close(m_stmt);
            }
        }

        void FreeResult() override {
            if (!m_stmt) return;
            MYSQL_RES* meta = mysql_stmt_result_metadata(m_stmt);
            if (meta) {
                mysql_free_result(meta);
            }
            mysql_stmt_free_result(m_stmt);
        }

        // 准备语句
        bool Prepare(const char* sql) {
            FreeResult();
            if (m_stmt) {
                mysql_stmt_close(m_stmt);
                m_stmt = nullptr;
            }

            m_stmt = mysql_stmt_init(m_mysql);
            if (!m_stmt) return false;

            if (mysql_stmt_prepare(m_stmt, sql, strlen(sql))) {
                TRACE("[SQL]准备语句失败：%s\n", mysql_stmt_error(m_stmt));
                mysql_stmt_close(m_stmt);
                m_stmt = nullptr;
                return false;
            }
            return true;
        }

        // 执行预编译语句
        bool Execute(const char* sql, MYSQL_BIND* bind, unsigned int argsNum) {
            if (!m_stmt) return false;

            TraceSql(sql, bind, argsNum);
            if (bind && argsNum > 0) {
                if (mysql_stmt_bind_param(m_stmt, bind)) {
                    TRACE("[SQL]绑定参数失败：%s\n", mysql_stmt_error(m_stmt));
                    return false;
                }
            }

            if (mysql_stmt_execute(m_stmt)) {
                TRACE("[SQL]执行失败：%s\n", mysql_stmt_error(m_stmt));
                return false;
            }
            return true;
        }

        // 获取结果集
        StmtResult* GetResult() {
            if (!m_stmt) return nullptr;

            std::unique_ptr<StmtResult> result(new StmtResult(m_stmt));
            if (!result->BindResult()) return nullptr;

            if (mysql_stmt_store_result(m_stmt)) {
                TRACE("[SQL]存储结果失败：%s\n", mysql_stmt_error(m_stmt));
                return nullptr;
            }

            return result.release();
        }
    private:
        // 预编译SQL日志打印
        void TraceSql(const char* sql, MYSQL_BIND* bind, unsigned int argsNum) {
            if (!sql) return;

            CString strSql = sql;
            int pos = 0;

            // 遍历所有参数
            for (unsigned int i = 0; i < argsNum; i++) {
                pos = strSql.Find('?');
                if (pos == -1) break;

                CString strValue;
                if (!bind[i].buffer) {
                    strValue = _T("NULL");
                }
                else {
                    switch (bind[i].buffer_type) {
                    case MYSQL_TYPE_STRING:
                        strValue.Format(_T("'%s'"),
                            CString((char*)bind[i].buffer));
                        break;

                    case MYSQL_TYPE_LONG:
                        strValue.Format(_T("%d"),
                            *(int*)bind[i].buffer);
                        break;

                    case MYSQL_TYPE_FLOAT:
                        strValue.Format(_T("%.2f"),
                            *(float*)bind[i].buffer);
                        break;

                    case MYSQL_TYPE_DOUBLE:
                        strValue.Format(_T("%.2f"),
                            *(double*)bind[i].buffer);
                        break;

                    case MYSQL_TYPE_TIMESTAMP:
                    case MYSQL_TYPE_DATETIME: {
                        MYSQL_TIME* time = (MYSQL_TIME*)bind[i].buffer;
                        strValue.Format(_T("'%04d-%02d-%02d %02d:%02d:%02d'"),
                            time->year, time->month, time->day,
                            time->hour, time->minute, time->second);
                        break;
                    }

                    default:
                        strValue = _T("?");
                        break;
                    }
                }

                strSql = strSql.Left(pos) + strValue + strSql.Mid(pos + 1);
            }

            TRACE("[SQL]预编译sql：%s\n", (LPCSTR)strSql);
        }
    };

protected:
    MYSQL* m_mysql;
    std::unique_ptr<NormalExecutor> m_normalExec;
    std::unique_ptr<PrecompileExecutor> m_preparedExec;

protected:
    IDatabase() : m_mysql(nullptr) {
        m_mysql = mysql_init(nullptr);
        if (m_mysql) {
            m_normalExec.reset(new NormalExecutor(m_mysql));
            m_preparedExec.reset(new PrecompileExecutor(m_mysql));
        }
    }

    // 常规SQL方法
    bool query_void(const char* sql) {
        return m_normalExec->Execute(sql);
    }

    MYSQL_RES* query_return(const char* sql) {
        return m_normalExec->ExecuteQuery(sql);
    }

    // 预编译SQL方法
    bool pre_query_void(const char* sql, MYSQL_BIND* bind, unsigned int argsNum) {
        if (!m_preparedExec->Prepare(sql)) return false;
        return m_preparedExec->Execute(sql, bind, argsNum);
    }

    StmtResult* pre_query_return(const char* sql, MYSQL_BIND* bind, unsigned int argsNum) {
        if (!pre_query_void(sql, bind, argsNum)) return nullptr;
        return m_preparedExec->GetResult();
    }

    virtual bool Init() = 0;
};